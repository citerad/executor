#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <signal.h>
#include "executor.h"
#include <sys/stat.h>
#include <sys/types.h>

extern char *nomecomandi[];		//Contiene comandi disponibili.
extern int  (*nomefunzioni[])(int, char **);	//Funzioni.
const char* jail_dir;
const char* fifo_path;
int fds;	//File descriptor FIFO

/*Funzione main*/
int main(int argc, char *argv[]) {

	//Due segnali di terminazione.
	signal(SIGINT, Signal_Hand);
	signal(SIGILL, Signal_Hand);

	//Controllo gli argomenti
	if (argc!=3) {
		fprintf(stderr, "Usage: %s <jail_dir> <fifo_path>\n", argv[0]);
		exit(10);	//Errore negli argomenti.
	}

	//Assegno alle variabili gli argomenti.
	jail_dir=argv[1];
	fifo_path=argv[2];

	//Creo la gabbia
	if(mkJail(jail_dir)<0){
		fprintf(stderr,"Errore creazione jail. \n");
		MyAbort(13);	//Esco con errore 13
	}
	//Crea la  FIFO
	int mode=umask(0);
	if(mkfifo( fifo_path, 0666 )<0 ) {
		perror("mkfifo() error.");
	    MyAbort(16); //Esco con errore 16
	  }
	umask(mode);

	//Ciclo infinito
	char msg[MSG_SIZE]; //Conterrà la linea letta dalla FIFO
	while(1) {
		if((fds=open(fifo_path, O_RDONLY))<0){   //Apre la FIFO
			perror("open() error.");
			MyAbort(16);	//Esco con errore 16
		}
		int stato; //Conterrà lo stato di uscita di ogni comando
		int narg; //Conterrà il numero di argomenti nella linea
		int len; //Lunghezza della linea
		char **comando;  //Conterrà i comandi da eseguire
		if((len=read(fds,msg,sizeof(msg)))<0 || len==1){  //Legge dalla FIFO
				    printf("Error: il comando non è valido\n");
			  }
		else {
			msg[len-1]='\0';
			printf("\nIl server ha letto dalla FIFO '%s'\n",msg);
			if( strcmp(msg, "abort")==0) MyAbort(6);
			if(strcmp(msg, "shutdown")==0) Shutdown();
			comando = split_arg(msg, " ", &narg);	//Divido gli argomenti
			stato=RunComm(narg, comando); //Eseguo il comando e salvo il suo valore di  uscita in stato.
			free_arg(comando); //Libero memoria
			Result(stato); //Controllo gli errori
		}
		close(fds);
	}
	unlink(fifo_path);
	rmJail(jail_dir);
	exit(0);
}

/*Cerca tra i comandi interni, altrimenti esegue
 * come esterno, ritorna lo stato di terminazione*/
int RunComm(int n, char **comm) {
	//Controllo se il comando appartiene a quelli interni
	int i=0;
	while( nomecomandi[i] != NULL ) {
	    if( strcmp(nomecomandi[i], comm[0]) == 0 ) {
	    	printf("Comando interno '%s'\n\n",nomecomandi[i]);
	    	//Comando riconosciuto come interno
	    	return nomefunzioni[i](n, comm);	//Eseguo
	    }
	    else
	      i++;
	}
	//Comando non presente, eseguilo come esterno.
	printf("Comando esterno: '%s'\n\n",comm[0]);
	return RunExt(n, comm);	//Tento esecuzione.
}


/*Esegue un comando esterno,
 * ritorna il valore di terminazione*/
int RunExt(int argc, char **argv) {
	int pid, stato;
	pid=fork();
	if (pid==0) { //processo figlio
		execvp (argv[0], argv);
		perror("execvp() error.");
		exit(20);	//Se l'execvp fallisce.
	}
	else if (pid>0) { //processo padre
				pid=wait(&stato); //Attendo la terminazione del figlio
				return stato;
			}
			else { //se la fork fallisce
				perror("fork() error.");
				exit(22);
			}
}

/*Controlla e notifica l'esito di ogni comando al parent.*/
void Result(int status) {
	int ppid=getppid(); //pid del parent.
	if(WIFEXITED(status) && WEXITSTATUS(status)==0) {
		kill(ppid, SIGUSR2); //Notifico al parent la corretta esecuzione
	}
	else {
		kill(ppid, SIGUSR1); //Notifico al parent un errore
	}
}

/*Crea la gabbia
 * restituisce 0 se ha successo.*/
int mkJail(const char* dir){
	//Divento root
	if (setuid(0)) {
		perror("setuid() error.");
		return -1;
	}

	//Preparo ambiente di lavoro
	if(chdir(dir)<0) {
		perror("chdir() error.");
		return -2; //Chdir fallito
	}
	//Se non è la radice monto le cartelle
	char d[200];
	getcwd(d, 200);
	if(strcmp(d,"/")!=0) {
		mkdir("bin", S_IRWXU | S_IRWXG);
		mkdir("lib", S_IRWXU | S_IRWXG);
		mkdir("usr", S_IRWXU | S_IRWXG);
		mkdir("lib64", S_IRWXU | S_IRWXG);
		chdir("usr");
		mkdir("bin",S_IRWXU | S_IRWXG);
		mkdir("lib",S_IRWXU | S_IRWXG);
		chdir("..");

		//Monto file necessari per ambiente di lavoro
		mount("/bin", "bin","", MS_BIND | MS_RDONLY,"");
		mount("/lib", "lib","", MS_BIND | MS_RDONLY,"");
		mount("/usr/bin", "usr/bin","", MS_BIND | MS_RDONLY,"");
		mount("/usr/lib", "usr/lib","", MS_BIND | MS_RDONLY,"");
		mount("/lib64", "lib64","", MS_BIND | MS_RDONLY,"");
	}
	//Tento chroot su dir
	if(chroot(dir)<0) {
		perror("chroot() error.");
		return -3; //Chroot fallito
	}
	chdir("/");
	return 0;
}

/*Termina correttamente*/
void Shutdown(){
	close(fds);
	unlink(fifo_path);
	rmJail(jail_dir);
	exit(0);
}

/*Termina con errore err*/
void MyAbort(err){
	unlink(fifo_path);
	rmJail(jail_dir);
	exit(err);
}

/*Smonta ambiente di lavoro*/
void rmJail(char *d){
	if(strcmp(d,"/")!=0) {
		umount ("bin");
		umount("lib");
		umount("usr/bin");
		umount("usr/lib");
		umount("lib64");
		rmdir("bin");
		rmdir("lib");
		rmdir("lib64");
		rmdir("usr/bin");
		rmdir("usr/lib");
		rmdir("usr");
	}
}

/*Gestisce segnali*/
void Signal_Hand(int signo) {
    switch( signo ) {
        //Se si ha ricevuto SIGINT
        case SIGINT: {
            printf( "\nExecutor PID: %d ha ricevuto SIGINT \n", getpid() );
            Shutdown();
            break;
	    }
        //Se si ha ricevuto SIGILL
        case SIGILL: {
        	printf( "\nExecutor PID: %d ha ricevuto SIGILL \n", getpid() );
            MyAbort(SIGILL);
            break;
        }
        default:
                break;
    }
    return;
}
