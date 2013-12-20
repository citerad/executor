//Funzioni di utilità.
#include <stdlib.h>
#include <string.h>
#include "util.h"

/*Ritorna il vettore contenenti gli argomenti presenti
   nella linea separati dal separatore sep e
   scrive il numero di argomenti in narg */
char ** split_arg(char *linea, char *sep, int *narg) {
	int i = 0;   //Contiene narg trovati
	char **argv; //Contiene gli argomenti trovati
	char *copialinea;  //Copia della linea
	char **indirizzocopialinea = &copialinea; //Punta a copialinea

	//Copia la linea
	copialinea = (char *) malloc( strlen(linea) + 1 );
	strcpy(copialinea, linea);
	//Alloco prima posizione vettore degli argomenti
	argv = (char **) malloc(sizeof(char *));
	//Fino alla fine
	while( (argv[i] = strsep(indirizzocopialinea, sep)) != NULL  ) {
		i++;
		//Ridimensiona vettore degli argomenti
		argv = (char **) realloc(argv, (i + 1) * sizeof(char *));
	}
	//Scrive il numero di argomenti  trovati
	*narg = i;
	return(argv);
}

/* Libera il vettore ritornato da
   una split_arg */
void free_arg(char **argv)	{
  free(argv[0]);
  /* libera il vettore di puntatori */
  free(argv);
}
