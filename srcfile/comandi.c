/*Funzioni di comandi interni*/
#include <stdio.h>
#include "comandi.h"

char *nomecomandi[] = {"pwd", "help", "echo", NULL};
int  (*nomefunzioni[])(int, char **) = {pwd, help, echo};

//Comando echo
int echo(int argc, char **argv) {
	int i;
	for(i = 1; i < argc; i++) {
		printf("%s", argv[i]); printf(" ");
	}
	printf("\n");
	return 0;
}

//Comando help
int help(int argc, char **argv) {
	printf("Progetto di Sistemi Operativi Modulo II 2011/2012\n");
	return 0;
}

//Comando directory corrente
int pwd(int argc, char **argv) {
	char d[200];
	getcwd(d, 200);
	printf("%s",d);
	return 0;
}
