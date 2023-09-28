#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void print_memory(void *mem, size_t size, int is_free) {
    printf("Memory at address %p of size %ld is %s\n", mem, size, (is_free ? "free" : "allocated"));
} 

void test0(){
    printf("\n\033[01;31mTEST0\033[00m\n");
    mem_show(print_memory);
}

void test1(){
    printf("\n\033[01;31mTEST1\033[00m\n");
    printf("\033[01;32mAllocation de 8160 octets\033[00m\n");
    void *ptr1 = mem_alloc(8160);
    mem_show(print_memory);
    printf("\033[01;32mLibération de 8160 octets\033[00m\n");
    mem_free(ptr1);
    mem_show(print_memory);
}

void test2(){
    printf("\n\033[01;31mTEST2\033[00m\n");
    
    printf("\033[01;32mAllocation de 8000 octets\033[00m\n");
    void *ptr1 = mem_alloc(8000);

    printf("\033[01;32mAllocation de 104 octets\033[00m\n");
    void *ptr2 = mem_alloc(104);

    printf("\033[01;32mAllocation de 40 octets\033[00m\n");
    void *ptr3 = mem_alloc(40);

    mem_show(print_memory);

    printf("\033[01;32mLibération des blocks en ordre croissant (du premier alloué au dernier)\033[00m\n");
    mem_free(ptr1);
    mem_free(ptr2);
    mem_free(ptr3);

    mem_show(print_memory);
}

void test3(){
    printf("\n\033[01;31mTEST3\033[00m\n");

    printf("\033[01;32mAllocation de 8000 octets\033[00m\n");
    void *ptr1 = mem_alloc(8000);

    printf("\033[01;32mAllocation de 104 octets\033[00m\n");
    void *ptr2 = mem_alloc(104);

    printf("\033[01;32mAllocation de 40 octets\033[00m\n");
    void *ptr3 = mem_alloc(40);

    mem_show(print_memory);

    printf("\033[01;32mLibération des blocks en ordre décroissant (du dernier alloué au premier)\033[00m\n");
    mem_free(ptr3);
    mem_free(ptr2);
    mem_free(ptr1);
    
    mem_show(print_memory);
}

void test4(){
    printf("\n\033[01;31mTEST4\033[00m\n");

    printf("\033[01;32mAllocation de 8000 octets\033[00m\n");
    void *ptr1 = mem_alloc(8000);

    printf("\033[01;32mAllocation de 104 octets\033[00m\n");
    void *ptr2 = mem_alloc(104);

    printf("\033[01;32mAllocation de 40 octets\033[00m\n");
    void *ptr3 = mem_alloc(40);

    // Affichage de la mémoire
    mem_show(print_memory);

    printf("\033[01;32mLibération du bloc du milieu\033[00m\n");
    mem_free(ptr2);

    // Affichage de la mémoire
    mem_show(print_memory);

    printf("\033[01;32mLibération des deux autres blocs\033[00m\n");
    mem_free(ptr1);
    mem_free(ptr3);

    // Affichage de la mémoire
    mem_show(print_memory);
}

void test5(){
    printf("\n\033[01;31mTEST5\033[00m\n");
    
    printf("\033[01;32mAllocation de 8000 octets\033[00m\n");
    void *ptr1 = mem_alloc(8000);

    printf("\033[01;32mAllocation de 104 octets\033[00m\n");
    void *ptr2 = mem_alloc(104);

    printf("\033[01;32mAllocation de 40 octets\033[00m\n");
    void *ptr3 = mem_alloc(40);

    // Affichage de la mémoire
    mem_show(print_memory);

    printf("\033[01;32mLibération du bloc des blocs aux extrémités\033[00m\n");
    mem_free(ptr1);
    mem_free(ptr3);

    // Affichage de la mémoire
    mem_show(print_memory);

    printf("\033[01;32mLibération du bloc du milieu\033[00m\n");
    mem_free(ptr2);

    // Affichage de la mémoire
    mem_show(print_memory);
}

int main(){
    // Initialisation de l'allocateur mémoire
    mem_init(get_memory_adr(), get_memory_size());

    // Affichage des zones libres et occupées d'un allocateur initialisé
    test0();

    // Cas où on alloue toute la zone mémoire d'un seul coup
    test1();

    // Cas où on alloue toute la zone mémoire en trois blocs, et qu'on libère toute la mémoire en ordre croissant
    test2();

    // Cas où on alloue toute la zone mémoire en trois blocs, et qu'on libère toute la mémoire en ordre décroissant
    test3();

    // Cas où on alloue toute la zone mémoire en trois blocs, et qu'on libère le bloc du milieu puis les deux autres
    test4();

    // Cas où on alloue toute la zone mémoire en trois blocs, et qu'on libère les deux blocs aux extrémités et qu'on libère celui du milieu en dernier
    test5();
    
}