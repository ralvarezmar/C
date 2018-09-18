#include <stdio.h>
#include "threads.h"
#include <time.h>
#include <unistd.h>
#include <stdlib.h>


//gcc -c -g -Wall -Wshadow main.c && gcc -c -g -Wall -Wshadow threads.c && gcc -o main threads.o main.o

int glob = 0;

void f1(){
	for (int i=0; i<10; i++){
		if ( i == 1){
			suspendthread();
		}
		usleep(200000);
		glob++;
		printf("im am f1 %d \n",  glob);
		yieldthread();
	}
	exitsthread();
}


void f2(){
	for (int i=0; i<10; i++){
		//resumethread(3);
		if ( i == 7){
			sleepthread(10000);
		}
		usleep(200000);
		glob++;
		printf("im am f2 %d \n",  glob);
		yieldthread();
	}
	exitsthread();
}

void f3(){
	int i;
	//int arg = *((int)*arg);
	for (i=0; i<10; i++){	
		usleep(200000);
		glob++;
		printf("im am f3 %d \n",  glob);
		yieldthread();
	}
	exitsthread();
}

int main(int argc,char **argv){
	int *list;
	int n;
	//int arg=13;
	initthreads();

	createthread(f1, NULL, 1024*4);
	createthread(f2, NULL, 1024*4);
	createthread(f3, NULL, 1024*4);
	//createthread(f3, (void*)arg, 1024*4);
	//createthread(f4, NULL, 1024*4);
	
	for(int i =0; i < 20; i++){
		if(i==5){
			n=suspendedthreads(&list);
			printf("Hilos suspendidos: %i \n ",  n);
			printf("Identificadores: ");
			for(int j=0;j<n;j++){
				printf("%d ", list[j]);
			}
			printf("\n");
			usleep(1000000);
			free(list);
		}
		if(i==7){
			resumethread(1);
		}
		usleep(200000);
		glob++;
		printf("im am main %d \n",  glob);
		yieldthread();
	}
	exitsthread();
	return 0;
}
