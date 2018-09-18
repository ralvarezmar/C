#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>

void fichero(char *ruta){
 	FILE* fp;
	char buf[128];
	fp = fopen(ruta, "r");
	while(fgets(buf, sizeof(buf),fp)>0){
		printf("%s", buf);
	}
	fclose(fp);
}

int text(char *name){
	char *p;
	p = strstr(name, ".txt");
	if ((p != NULL) && (strlen(p) == strlen (".txt")))
	{
		return 1;
	}else{
		return 0;
	}
}

void directorio(char *path){
	DIR *d = opendir(path);
	char newpath[512];
	struct dirent *entry;
	while((entry= readdir(d)) != NULL){
		if((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)){
			sprintf(newpath,"%s/%s",path,entry->d_name);
			printf("%s\n", newpath);
			if(entry->d_type == 4){
				directorio(newpath);
			}else if(text(newpath)){
				fichero(newpath);
			}
		}
	}
	closedir(d);
}

int main(int argc, char* argv[]){
	char cwd[1024];
	if(argc == 2){
		printf("%s\n", argv[1]);
		directorio(argv[1]);
	}else {
		getcwd(cwd, sizeof(cwd));
		printf("%s\n", cwd);
		directorio(cwd);
	}
	exit(0);
}
