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
    Free, Running, Waiting,Deleted
};

enum{
	maxThread= 32
};

struct Thread{
	int status; 
	int threadid;
	char *stack;
	long time; //tiempo para el cuanto
	ucontext_t uctx_thread;//contexto	
};

int globalid = 1;
int currentid = 0; //guardo el indice del array

static struct Thread numThread[maxThread];


int currenttime(void){
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	return ((start.tv_sec * 1000000) + (start.tv_nsec / 1000));
}

int planificador(){
	int i=currentid+1;
	if (currenttime() - numThread[currentid].time  > 200){		
		while(i%maxThread != currentid){  
			if (numThread[i%maxThread].status == Waiting){
				return i % maxThread;		
			}else if (numThread[i % maxThread].status == Deleted){
				numThread[i % maxThread].status = Free;
				free(numThread[i % maxThread].stack);
			}
			i++;
		}
	}else{
		return currentid;
	}	
	return -1;
}


int nextThread(void){
	int i=currentid+1;
	while(i%maxThread != currentid){
			if (numThread[i%maxThread].status == Waiting){
				return i % maxThread;
			}
			i++;
	}
	return -1;
}

void initthreads(void){
	numThread[0].threadid=globalid;
	numThread[0].status = Running;	
	numThread[0].stack= (char *)malloc (32*1024);
	numThread[0].time = currenttime();
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

//Funcion auxiliar para recorrer todos los threads y mostrar sus campos 
void printarray(void){
	for(int i=0; i<maxThread;i++){
		printf("Id array: %i, Id global: %i, Estado: %i\n", i, numThread[i].threadid, numThread[i].status);
	}
}

void yieldthread(void){
	int nextid=0;
	int actual=currentid;
//	if (numThread[currentid].status == Running){ 
		nextid=planificador();
		if (nextid != -1){
			numThread[currentid].status = Waiting; 
			currentid=nextid; //cambio la global y meto el identificador del array	
			numThread[currentid].status = Running;
 			numThread[currentid].time = currenttime();	
			swapcontext(&numThread[actual].uctx_thread,&numThread[nextid].uctx_thread);	
		}
//		printarray();
//	}else{
//		err(1,"CurrentID not Running");
//	}
	
}

void exitsthread(void){
	int nextid=nextThread();
	int actual=currentid;
	if (nextid== -1){
		exit(0);	
	}
	numThread[currentid].status = Deleted;
	numThread[nextid].status = Running; 
	currentid=nextid;
	if (swapcontext(&numThread[actual].uctx_thread,&numThread[nextid].uctx_thread) == -1){
		err(1,"swapcontext");
	}
}

int curidthread(void){
	return currentid;
}
