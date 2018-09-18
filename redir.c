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
#include <glob.h>

void compile(char *pathc){
	char *ejecutable;
	char *temp;

	temp = getenv("CFLAGS");
	ejecutable=strdup(pathc);
	strtok(ejecutable,".");
	if (temp == NULL){
		execl("/usr/bin/gcc","gcc","-o",ejecutable,pathc,NULL);
		fprintf(stderr, "%s: no compila\n", pathc);
	}else{
		execl("/usr/bin/gcc", "gcc", temp,"-o",ejecutable,pathc,NULL);
		fprintf(stderr, "%s: no compila\n", pathc);
	}
}

void dogrep(char* word){
	int status;
	pid_t pid;
	pid = fork();
	
	switch(pid){
		case -1:
			err(1,"fork");
			break;
		case 0:
			execl("/bin/grep","grep",word,NULL);
			err(1,"execl");
		default: 
			while(wait(&status)!=pid);
	}
}

void pipefrom(char *path,char *word){
	int fd[2];
	pid_t pid;
	int sts;

	pipe(fd);
	pid = fork();
	switch(pid){
		case -1:
			err(1,"fork");
			break;
		case 0:
			dup2(fd[1], STDERR_FILENO);
			close(fd[1]);
			compile(path);
		default: 
			dup2(fd[0],0);		
			close(fd[0]);

			wait(&sts);
			if(WEXITSTATUS(sts)==0){
				printf("%s: compila\n",path);
			}else{
				close(fd[0]);
				close(fd[1]);
				printf("%s: no compila\n",path);
				dogrep(word);
			}
		}
}

void directory(char *path,char *word){
	glob_t results;

	results.gl_pathc=0;
	results.gl_pathv=NULL;
	results.gl_offs=0;
	glob(path,GLOB_DOOFFS,NULL,&results);
	for(int i=0;i<results.gl_pathc;i++){
		pipefrom(results.gl_pathv[i],word);
	}
}

int main(int argc, char* argv[]){
	char cwd[512];
	char path[512];
	if (argc==3){
		sprintf(path,"%s/*.c",argv[1]);
		directory(path,argv[2]);
	}else{
		getcwd(cwd, sizeof(cwd));
		sprintf(path,"%s/*.c",cwd);
		directory(path,argv[1]);
	}
	exit(0);
}
