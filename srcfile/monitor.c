#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "monitor.h"
#include "errori.h"

int corretti=0, errori=0;	//Totale comandi eseguiti

int main(int argc, char *argv[]) {
	int pid;
	pid=fork();
	if (pid==0) { //Processo figlio
		execl (PATHEX, NAMEEX, JAILDIR, FIFOPATH, NULL);
		perror("execl() error");
		exit(20); //Fallita
		}
		else if (pid>0) { //Processo padre
					int stato; //Conterrà lo stato di uscita del figlio
					signal(SIGUSR1, Signal_Handler);
					signal(SIGUSR2, Signal_Handler);
					pid = wait(&stato); //Aspetto lo stato di terminazione dell'executor.
					WriteLog(stato); //Scrivo il file di Log
					exit(0);
				}
				else {
					perror("fork() error.");
					exit(22);	//Errore nel fork
			}
		exit(0);
}

/*Scrive in un file di log le attività dell'executor in base alla sua terminazione.*/
void WriteLog(int status) {
	FILE *logfile;
	//Preparo il logfile
	if((logfile = fopen ("log.txt", "w+"))<0) {
		perror("Errore creazione logfile");
		exit(1);	//errore creazione logfile
	}
	fprintf(logfile, "Avvio l'executor . . . \n");

	//Controllo lo stato di terminazione dell'executor
	if(WIFEXITED(status)){	//se è terminato normalmente
		switch (WEXITSTATUS(status)) {	//controllo lo stato di uscita
			case 0: {	//correttamente
						fprintf(logfile, "Executo terminato correttamente.\n");
						break;
			}
			case 10: {	//Argomenti non corretti
				fprintf(logfile, "%s", ARGER);
				break;
			}
			case 20: {	//Executor non trovato
				fprintf(logfile, "%s", EXEFLL);
			        	break;
			        	}
			case 22: {	//Errore fork
				fprintf(logfile, "%s", FRKFLL);
						break;
			}
			case 13: {	//Errori jail
				fprintf(logfile, "%s", JLLERR);
			        	break;
			        	}
			case 16: {	//Errori fifo
			        	fprintf(logfile, "%s", FIFOER);
			        	break;
			        	}
			case 6: {	//Abort
				fprintf(logfile, "%s", ABTCMD);
				break;
			}
		default: {	//Altri errori
			        fprintf(logfile, "Errore sconosciuto");
			        break;
			        }
		}
	}	//Se executor non è terminato normalmente
	else if( WIFSIGNALED( status ) ) //estraggo n segnale
			    fprintf(logfile, "Executor terminato con segnale: %d\n", WTERMSIG(status));

	fprintf(logfile, "\nComandi eseguiti correttamente: %d\n", corretti);
	fprintf(logfile, "Comandi con errore: %d\n", errori);
	fclose(logfile);
	return;
}

/*Gestisce i segnali*/
void Signal_Handler(int signo) {
    switch( signo ) {
        case SIGUSR1: { //Se è SIGUSR1 un comando non è andato a buon fine
            errori++;
            break;
	    }
        case SIGUSR2: { //Se è SIGUSR2 un comando è andato a buon fine
            corretti++;
            break;
        }
        default:
                break;
    }
    return;
}

