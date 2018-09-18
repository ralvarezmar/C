#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <glob.h>


void compile(char *pathc){	
	char *ejecutable;
	char *temp;
	pid_t pid;
	int sts;

	temp = getenv("CFLAGS");
	ejecutable=strdup(pathc);
	strtok(ejecutable,".");
	printf("Ejecutable: %s Path: %s\n",ejecutable,pathc);
	if (temp == NULL){
		execl("/usr/bin/gcc","gcc","-o",ejecutable,pathc,NULL);
		fprintf(stderr, "%s: no compila\n", pathc);
	}else{
		execl("/usr/bin/gcc", "gcc", temp,"-o",ejecutable,pathc,NULL);
		fprintf(stderr, "%s: no compila\n", pathc);
	}
}


void dofork(char *directorio){
	int sts;
	pid_t pid;
	pid = fork();
	switch(pid) {
		case -1:
			err(1, "fork");
			break;
		case 0:
			compile(directorio);
			break;
		default:
			wait(&sts);
			if(WEXITSTATUS(sts)==0){
				printf("%s: compila\n", directorio);	
			}else{
				printf("%s: No compila\n", directorio);	
			}
	}
}

void directory(char *path){
	int sts;
	glob_t results;
	results.gl_pathc=0;
	results.gl_pathv=NULL;
	results.gl_offs=0;
	pid_t pid,wpid;
	glob(path,GLOB_DOOFFS,NULL,&results);
	pid=fork();
	switch(pid){
		case -1:
			err(1,"fork");
			break;
		case 0:	
			for(int i=0;i<results.gl_pathc;i++){
				dofork(results.gl_pathv[i]);
			}
		default:
			while ((wpid = wait(&sts)) > 0){
				if(WIFEXITED(&sts)){
					if(WEXITSTATUS(&sts)!=0){
						WEXITSTATUS(&sts);						
					}
				}
			}
		}
}

int main(int argc, char* argv[]){
	char ruta[512];
	sprintf(ruta,"%s/*.c",argv[1]);
	directory(ruta);
	exit(0);
}

