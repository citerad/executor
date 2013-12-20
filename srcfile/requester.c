#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <requester.h>
#include <signal.h>

int fd;

int main(int argc, char *argv[]) {

	signal(SIGPIPE, Signal_Handler);
	signal(SIGINT, Signal_Handler);

	//Controllo gli argomenti
	if (argc!=2) {
		printf("Usage: %s <fifo_path>\n", argv[0]);
		exit(10);
	}
	const char* fifo_path=argv[1];
	char msg[MSG_SIZE];

	//Tento apertura FIFO
	if ((fd=open(fifo_path, O_WRONLY))<0) {
		perror("open() error");
		exit(16);
	}

	//Leggo da stdin e scrivo nella FIFO
	while (fgets(msg, MSG_SIZE, stdin)) {
			write(fd, msg, strlen(msg));
			sleep(1);
		}
	close(fd);
	exit(0);
}

//Gestisce SIGPIPE ricevuto quando l'executor esegue l'unlink della FIFO.
void Signal_Handler(int signo) {
	if(signo=SIGPIPE) {
		printf("Executor terminato.\n");
		printf("Arresto requester.\n");
	}
	if(signo=SIGINT) 
		printf("Arresto requester.\n");
	close(fd);
	exit(0);
}
