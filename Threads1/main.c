#include <stdio.h>
#include "threads.h"
#include <time.h>
#include <unistd.h>

int glob = 0;


//gcc -c -g -Wall -Wshadow main.c && gcc -c -g -Wall -Wshadow threads.c && gcc -o main threads.o main.o


void f1(){
	for (int i=0; i<10; i++){
		usleep(100000);
		glob++;
		printf("im am f1 %d \n",  glob);
		yieldthread();
	}
	exitsthread();
}


void f2(){
	for (int i=0; i<10; i++){
		usleep(100000);
		glob++;
		printf("im am f2 %d \n",  glob);
		yieldthread();
	}
	exitsthread();
}


int main(int argc,char **argv){
	initthreads();

	createthread(f1, NULL, 1024*4);
	createthread(f2, NULL, 1024*4);
	
	for(int i =0; i < 20; i++){
		usleep(100000);
		glob++;
		printf("im am main %d \n",  glob);
		yieldthread();
	}
	exitsthread();
	return 0;
}
