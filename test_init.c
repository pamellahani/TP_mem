#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NB_TESTS 10

void print_zone(void *ad, size_t taille, int boolean){
	if (boolean){ // La zone est libre
		printf("La zone à l'adresse %p , de taille %ld est libre\n", ad, taille);
	}else{ // La zone est occupée
		printf("La zone à l'adresse %p , de taille %ld est occupée\n", ad, taille);
	}
}

int main(int argc, char *argv[]) {
	// Pointeur sur la fonction print
	void (*print)(void *, size_t, int) = &print_zone;


	fprintf(stderr, "Test réalisant de multiples fois une initialisation suivie d'une alloc max.\n"
			"Définir DEBUG à la compilation pour avoir une sortie un peu plus verbeuse."
 		"\n");
	for (int i=0; i<NB_TESTS; i++) {
		debug("Initializing memory\n");
		printf("%p\n",get_memory_adr()); 
		mem_init(get_memory_adr(), get_memory_size());
		alloc_max(get_memory_size());
	}
	mem_show((*print));

	
	

	// TEST OK
	return 0;
}
