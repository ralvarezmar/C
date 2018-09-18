//Rubén Álvarez Martín

#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <time.h>


//gcc -c -g -Wall -Wshadow main.c && gcc -c -g -Wall -Wshadow threads.c && gcc -o main threads.o main.o

enum{
    Free, Running, Waiting,Deleted,Suspended,Slept
};

enum{
	maxThread= 32
};

struct Thread{
	int status; 
	int threadid;
	char *stack;
	long time; //tiempo para el cuanto
	long timesleep;
	ucontext_t uctx_thread;//contexto	
};

int globalid = 0;
int currentid = 0; //guardo el indice del array
int minsleep=0; //id del array que menos tiene que dormir 


static struct Thread numThread[maxThread];


int currenttime(void){
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	return ((start.tv_sec * 1000) + (start.tv_nsec / 1000000));
}

int nextThread(int id){
	int i=id+1;

	while(i%maxThread != id){
			if (numThread[i%maxThread].status == Waiting){
				return i % maxThread;
			}else if(numThread[i%maxThread].status == Slept){ 
				//apuntar el hilo que menos tiempo tiene que dormir 
				if(minsleep==0 || (numThread[i % maxThread].timesleep < numThread[minsleep].timesleep)){
					minsleep=i%maxThread;
				}
				if((currenttime() - numThread[currentid].timesleep) < 0){
					if((i % maxThread) == minsleep){
						minsleep=0;
					}
					numThread[i % maxThread].timesleep = 0;
					numThread[i % maxThread].status = Waiting;
					return i % maxThread;
				}
			
			}else if (numThread[i % maxThread].status == Deleted){
				numThread[i % maxThread].status = Free;
				free(numThread[i % maxThread].stack);
			}
			i++;
	}
	return -1;
}

int planificador(void){
	if (currenttime() - numThread[currentid].time  > 200){		
		if (nextThread(currentid) == -1){
			return currentid;
		}else{
			return nextThread(currentid);
		}
	}else{
		return currentid;
	}	
}

int threadsSuspended(void){
	int n=0;
	for(int i=0;i<maxThread;i++){
		if(numThread[i].status == Suspended){
			n++;
		}
	}
	return n;
}

/////FUNCION DE PRUEBA
void printarray(void){
	for(int i=0; i<maxThread;i++){
		printf("Id array: %i, Id global: %i, Estado: %i\n", i, numThread[i].threadid, numThread[i].status);
	}
}


void initthreads(void){
	numThread[0].threadid=globalid;
	numThread[0].status = Running;	
	numThread[0].stack= (char *)malloc (32*1024);
	numThread[0].time = currenttime();
	numThread[0].timesleep = 0;
	currentid = 0;
}

int createthread(void (*mainf)(void*), void *arg, int stacksize){
	int id = 0;
	for (int i=1; i<maxThread;i++){
		if (numThread[i].status == Free){//creo thread
			getcontext(&numThread[i].uctx_thread); //inicializo contexto
			globalid++;
			id = globalid; 
			numThread[i].threadid=id;
			numThread[i].status = Waiting;
 			numThread[i].time = currenttime();	
			numThread[i].stack= malloc (stacksize);	
			numThread[i].timesleep = 0;

			numThread[i].uctx_thread.uc_stack.ss_sp= numThread[i].stack;
			numThread[i].uctx_thread.uc_stack.ss_size= stacksize;
			numThread[i].uctx_thread.uc_link= NULL; 

			makecontext(&numThread[i].uctx_thread, (void(*)(void))mainf,1,arg);
			break;	
		}
	}
	if (id == 0){ //Si id sigue a 0 es porque no podía crear thread 
		return -1;
	}else{
		return id;
	}
} 
void changethread(int sts,int nextid){
	int actualid=currentid;

	numThread[currentid].status = sts;
	numThread[nextid].status = Running; 
	numThread[nextid].time = currenttime();	
	currentid=nextid;
	if (swapcontext(&numThread[actualid].uctx_thread,&numThread[nextid].uctx_thread) == -1){
		err(1,"swapcontext");
	}
}

void yieldthread(void){
	int nextid=0;
	
	nextid=planificador();
	if (nextid != currentid){
		changethread(Waiting,nextid);
	 }	
}


void exitsthread(void){
	int wait;
	int id=nextThread(currentid);
	if(nextThread(currentid)==-1 && minsleep != 0 && numThread[minsleep].timesleep > 0){
		wait= numThread[minsleep].timesleep - currenttime();
		printf("Waiting for slept thread %i seconds...\n",wait/1000);
		numThread[minsleep].status = Waiting;
		numThread[minsleep].timesleep=0;
		usleep(wait*1000);
		changethread(Deleted,minsleep);
	}else if(nextThread(currentid)==-1 && threadsSuspended()>0){
		errx(1,"There aren't threads waiting but there are threads suspended");	
	}else if(nextThread(currentid) == -1){
		exit(0);
	}
	changethread(Deleted,id);//Comprobar si hay algun hilo en dormido cuando nextThread devuelva -1
}

int curidthread(void){
	return numThread[currentid].threadid;
}

void suspendthread(void){
	int id=nextThread(currentid);
	if (id >= 0){
		changethread(Suspended,id);			
	}else{
		errx(1,"There aren't threads waiting");	
	}	
}

int resumethread(int id){
	for(int i=0;i<maxThread;i++){
		if (numThread[i].threadid == id && numThread[i].status == Suspended){  
			numThread[i].status = Waiting;
			return 0;
		} 
	}
	return -1;
}

int suspendedthreads(int **list){
	int n=0;
	int threads= threadsSuspended();
	*list=malloc(sizeof(int)*threads);
	for(int i=0;i<maxThread;i++){
		if(numThread[i].status == Suspended){
			*list[n]= numThread[i].threadid;
			n++;
		}
	}
	return threads;
}

int killthread(int id){
	for(int i=0;i<maxThread;i++){
		if(numThread[i].threadid == id){
			if (numThread[i].status == Waiting || numThread[i].status == Suspended){ 
				numThread[i].status = Deleted;
				free(numThread[i].stack);
				return numThread[i].threadid;
			}else if(numThread[i].status == Running){ 
				exitsthread();
				return numThread[i].threadid;
			}else{
				return -1;
			}
		}
	}
	return -1;
}

void sleepthread(int msec){
	int id=nextThread(currentid);
	numThread[currentid].timesleep = currenttime() + msec; //Mientras este campo sea mayor que la hora actual el thread 
								//sigue dormido 
	changethread(Slept,id);
}

