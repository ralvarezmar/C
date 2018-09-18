#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>

void copyfile(char *or,char *dest, long permisos){
	int fd,fo;
	char buf[64]
	int bytes;
	fo = open(or,O_RDONLY);
	fd = open(dest,O_WRONLY|O_TRUNC|O_CREAT);
	if (fd<0){
		err(1,"Fallo al crear el fichero");
	}
	while((bytes=read(fo,&buf,sizeof buf))>0){ //read del archivo origen y write en destino
		write(fd,&buf,bytes);
	}	
	close(fo);
	close(fd);
	chmod(dest,permisos);
}

void directorio(char *opath,char *dpath,long fpermisos){
	char newpath[512];
	char destino[512];
	DIR *d = opendir(opath);
	struct dirent *entry;
	mkdir(dpath,0777);
	while((entry= readdir(d)) != NULL){
		if((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)){
			sprintf(newpath,"%s/%s",opath,entry->d_name);
			sprintf(destino,"%s/%s",dpath,entry->d_name);
			if(entry->d_type == 4){
				//printf("Origen: %s -->", newpath);
				//printf("Destino: %s\n", destino);
				directorio(newpath,destino,fpermisos);
			}else if(entry->d_type == 8){
				//printf("Origen: %s -->", newpath);
				//printf("Destino: %s\n", destino);
				copyfile(newpath,destino,fpermisos);
			}
		}
	}
	closedir(d);
}

void cambiopermisos(char *dpath,long permisos){
	DIR *d = opendir(dpath);
	struct dirent *entry;
	char newpath[512];
	chmod(dpath,permisos);
	while((entry= readdir(d)) != NULL){
		if((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)){			
			if(entry->d_type == 4){
				sprintf(newpath,"%s/%s",dpath,entry->d_name);
				cambiopermisos(newpath,permisos);
			}
		}
	}
	closedir(d);
}

int main(int argc, char* argv[]){

	long permisosdir=strtol(argv[1],NULL,8);
	long permisosfile=strtol(argv[2],NULL,8);

	directorio(argv[3],argv[4],permisosfile);
	cambiopermisos(argv[4],permisosdir);
	exit(0);
}
