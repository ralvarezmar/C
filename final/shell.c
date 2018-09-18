//gcc -c -Wall -Wshadow -g shell.c
//gcc -o shell shell.o

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <ctype.h>
#include <features.h>
#include <signal.h>

typedef struct TCommand TCommand;

enum{
    Maxcmd=50,
    Nargs=50,
    Stack=1024
};

struct TCommand{
  char *cmd;
  char *argumentos[Nargs];
};

TCommand commands [Maxcmd];

int procesos;
int entrada;
int salida;
int background;
int document;

//Separo palabras por "separador"  que están en "buffer" y las guardo en "palabras"
int tokenizar(char *buffer, char **palabras,char *separador){
  char *token;
  int i = 0;

  token=strtok(buffer,separador);
  while(token){
    palabras[i]=token;
    i++;
    token = strtok(NULL,separador);
  }
  return i;
}

//Compruebo si hay alguna variable de entorno en la linea introducida
void varentorno(int n, char **argumentos){
  char *variable;

  for(int i=0;i<n;i++){
    if(strchr(argumentos[i],'$')!=NULL){
      variable = &argumentos[i][1];
			argumentos[i] = getenv(variable);
    }
  }
}

//Built-in cd
void chdfunction(char *chd){
  char *home;
  int c;

  if(chd == NULL){
    home = getenv("HOME");
    c=chdir(home);
  }else{
    c = chdir(chd);
  }
  if(c != 0){
    err(0,"error in chd");
  }
}

//redireccion de salida
void checkFileOut(char **token, char *fout){
  int redir;
  char *aux[Stack];
  char *aux2[Stack];

  for(int i=procesos;i>=0;i--){
    redir = tokenizar(token[i],aux,">"); //SALIDA
    if(redir>1){
      salida=1;
      tokenizar(aux[1],aux2," ");
      strncpy(fout,aux2[0],strlen(aux2[0]));
      token[i]= aux[0];
      break;
    }
  }
}

//Redireccion de entrada
void checkFileIn(char **token,char *fin){
  int redir;
  char *aux[Stack];
  char *aux2[Stack];

  for(int i=0;i<procesos;i++){
    redir = tokenizar(token[i],aux,"<"); //ENTRADA
    if (redir>1){
      entrada=1;
      tokenizar(aux[1],aux2," ");
      //printf("Entrada: %s-%s-\n",aux[1],aux2[0]);
      strncpy(fin,aux2[0],strlen(aux2[0]));
      token[i] = aux[0];
      break;
    }
  }
}

void structure(char *buffer,char *in,char *out){
  char *token[Stack];
  int arg;

  procesos = tokenizar(buffer,token,"|\r\n&");
  checkFileOut(token,out);
  checkFileIn(token,in);
  for(int i=0;i<procesos;i++){
		arg = tokenizar(token[i],commands[i].argumentos," ");
    commands[i].cmd=commands[i].argumentos[0];
    varentorno(arg,commands[i].argumentos);
    commands[i].argumentos[arg]=NULL;
  }
}

//Busca el binario del comando a ejecutar
int searchBin(int n, char **token,char *comando,char path[128]){
  if (access(comando,X_OK) == 0){
    sprintf(path,"%s/%s",getcwd(NULL,0),comando);
  }else{
    for(int i=0;i<n;i++){
      sprintf(path,"%s/%s",token[i],comando);
      if(access(path,X_OK) == 0){
        return 1;
      }
    }
    return 0;
  }
  return 1;
}

//redirecciona la salida a dev/null cuando se le pone &
void redirToBack(){
  int fp;
  fp = open("/dev/null",O_WRONLY);
  if(fp<0){
    err(1,"err background");
  }
  dup2(fp,STDOUT_FILENO);
  close(fp);
}

//Redirecciona la salida a un fichero
void redirOut(char *path){
  int fp;
  fp=open(path,O_WRONLY|O_CREAT|O_TRUNC,0664);
  if(fp<0){
    err(1,"error in open out");
  }
  dup2(fp,STDOUT_FILENO);
  close(fp);
}

//Toma como entrada un fichero
void redirIn(char *path){
  int fp;

  fp=open(path,O_RDONLY);
  if(fp<0){
    err(1,"error in open in");
  }
  dup2(fp,STDIN_FILENO);
  close(fp);
}

//OPCIONAL1-> Leer entrada estandar y tomar como documento
void hereDocument(int p){
  char text[Stack];
  int n;

  while((n=read(STDIN_FILENO,text,Stack))> 2){
    n=write(p,text,n);
  }
}

//OPCIONAL 2-> Crear variable de entorno
void createVar(char * buffer){
  char *token[512];

  tokenizar(buffer,token,"=");
  token[1][strlen(token[1])-1]= '\0';
  setenv(buffer,token[1],1);
}

void dopipes(char *in,char *out){
  pid_t pid[Maxcmd];
  int fd[Maxcmd][2];
  char *token[Stack];
  char *path;
  char binpath[Stack];
  int n,sts;

  for(int i=0;i<procesos-1;i++){
    pipe(fd[i]);
  }

  for(int i=0;i<procesos;i++){
    pid[i] = fork();

    switch(pid[i]){
      case -1:
        err(1,"fork");
        break;
      case 0:
        path = getenv("PATH");
        n = tokenizar(path,token,":");
        if(searchBin(n,token,commands[i].argumentos[0],binpath)==0){
          err(1,"Binary fail");
        }
        //printf("Binario: %s \n",binpath);
        if (background == 1 && i == procesos-1 && salida != 1){
          redirToBack();
        }
        if(entrada == 1 && i==0){
          redirIn(in);
        }
        if(salida == 1 && i==procesos-1){
          redirOut(out);
        }
        if(document == 1 && i==0){
          hereDocument(fd[0][1]); //redireccionar a la entrada del pipe
          exit(0);
        }
        if(procesos > 1){
          if(i==0 && document != 1){ //Primer comando-> redirige salida
            dup2(fd[0][1],STDOUT_FILENO);
          }else if(i>0 && i<procesos-1){ //Comandos intermedios
            dup2(fd[i][1],STDOUT_FILENO);
            dup2(fd[i-1][0],STDIN_FILENO);
          }else if(i==procesos-1){ //Ultimo comando ->toma la entrada
            dup2(fd[i-1][0],STDIN_FILENO);
          }
        }
        for(int j=0;j<procesos-1;j++){ //Cierro pipes en el hijo
        //  printf("Entra en for de cierre %i\n",i);
          close(fd[j][0]);
          close(fd[j][1]);
        }
        //printf("Binario:%s\n",binpath);
        //printf("Argumento:%s\n",*commands[i].argumentos);
        execvp(binpath,commands[i].argumentos);
        err(1,"excl error");
        exit(1);
      }
    }
    //printf("Entra en segundo for de cierre\n");
    for(int j=0;j<procesos-1;j++){ //Cierro pipes
      close(fd[j][0]);
      close(fd[j][1]);
    }
    if(background!=1){
      for(int i=0;i<procesos;i++){ //Espero a que terminen los hijos
        if(waitpid(pid[i],&sts,0) < 0){
          err(1,"Wait pid");
        }
      }
    }

}

void docmd(){
    char buffer[Stack];
    char fileout[Stack];
    char filein[Stack];
    char *n;
    //char str[3];
  //  int lineas=0;

    while(1){
      //lineas++;
    //  sprintf(str,"%d",lineas);
    //  setenv("nl",str,1);
      background = 0;
      entrada = 0;
      salida = 0;
      document = 0;

      printf("; ");

      n=fgets(buffer,Stack,stdin);

      if (n==NULL){
        break;
      }

      if(strcmp(buffer,"exit\n") == 0){ //EXIT
        break;
      }

      if((strchr(buffer,'=')!=NULL)){ //CREAR VARIABLE DE ENTORNO- OPCIONAL2
        createVar(buffer);
        continue;
      }

      if(strchr(buffer,'&')!=NULL){ //BACKGROUND
        background=1;
      }

      if(strchr(buffer,'[')!=NULL){ //OPCIONAL1
        document=1;
        buffer[strlen(buffer)-2]= '\0';
      }

      structure(buffer,filein,fileout); //ARMAR ESTRUCTURA

      if(strcmp(commands[0].cmd,"chd")==0){  //BUILT-IN CHD
        chdfunction(commands[0].argumentos[1]);
        continue;
      }
      dopipes(filein,fileout);
    }
}

int main(int argc, char* argv[]){
  docmd();
  exit(0);
}
