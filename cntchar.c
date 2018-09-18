#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <string.h>

void contar_letras(char* buffer, int c, int argc, char* argv[]){
	int contador; 
	for(int i=c;i<argc;i++){
			contador = 0;
			for(int x = 0; x<strlen(buffer); x++){	
					
				if(strncmp(&buffer[x],argv[i],1)==0){ 
					contador++;
				}				
			}
			printf("%s: %i\n",argv[i],contador);
		}
}

int main(int argc, char* argv[]){
	char buffer[100];
	FILE *archivo; 
	int c=1;
 	if(strcmp(argv[1],"-f")==0){ 
		archivo=fopen(argv[2],"r");	
		fgets(buffer,100,archivo);
		c=3;
	}else{
		fgets(buffer,100,stdin);
	}
	contar_letras(buffer,c,argc,argv);
	
	exit(0);

}
