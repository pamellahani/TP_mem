/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* structure placée au début de la zone de l'allocateur

   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur

   Elle peut bien évidemment être complétée
*/
struct allocator_header {
        size_t memory_size; // taille de toute la zone mémoire initialisée
	mem_fit_function_t *fit;
	struct fb *first_free;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void* memory_addr;

static inline void *get_system_memory_addr() {
	return memory_addr;
}

static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}


struct fb {
	size_t size; // taille de la zone occupée (métadonnées + zone utilisateur)
	struct fb* next; // pointeur sur la prochaine zone libre
};


void mem_init(void* mem, size_t taille)
{
        memory_addr = mem;
        *(size_t*)memory_addr = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());
	
	/* Création de la première zone libre */
	get_header()->memory_size = taille;
	get_header()->first_free = (struct fb*)(get_header()+1);

	struct fb *b = get_header()->first_free; // L'adresse de la première zone libre correspond au début de l'allocateur, c'est-à-dire à la fin du header
	b->next = NULL; // Une unique zone libre est présente dans l'allocateur lorsqu'il est initialisé
	b->size = get_system_memory_size() - sizeof(struct allocator_header) - sizeof(size_t);


	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	struct fb *ad_zl = get_header()->first_free; // Adresse de la première zone libre
	void *current = (get_header()+1); // Adresse de la première zone de la mémoire (occupée ou non)
	size_t size;

	while (current < get_system_memory_addr() + get_system_memory_size()) { // Tant qu'on n'atteint pas la fin de l'allocateur, on continue de parcourir

		size = *((size_t *)current); // Taille du bloc actuel

		if (ad_zl == current){ // Cas où la zone actuelle est une zone libre
			print(current + sizeof(struct fb*) + sizeof(size_t), size, 1);
			ad_zl = ad_zl->next; // On passe à la prochaine zone libre
		}
		// On affiche l'adresse de la zone actuelle, sa taille, et on indique si elle est libre (1) ou non (0).
		else{ // Cas où la zone actuelle est une zone occupée
			print(current + sizeof(size_t), size, 0);
		}
		current = (current + size + sizeof(size_t)); // On passe au bloc suivant

	}
}

void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

// Renvoie la zone libre qui précède la zone mémoire zone donnée en argument, ou NULL si elle n'en possède pas
struct fb* get_prec_void(void *zone){
	struct fb *ad_zl = get_header()->first_free; // Première zone libre

	if (ad_zl == NULL || (void *)ad_zl > zone){ // S'il n'y a aucune zone libre du tout, ou que la première zone libre se situe après zone
		return NULL;  
	}
	
	while (ad_zl->next != NULL && (void *)ad_zl->next < zone){ // Tant que la prochaine zone libre n'est pas nulle et se situe avant zone
		ad_zl = ad_zl->next; // On passe à la prochaine zone libre
	}
	return ad_zl;
} 

// Renvoie la zone libre qui précède la zone libre fb donnée en argument
struct fb* get_prec(struct fb* fb){
	struct fb *ad_zl = get_header()->first_free; // Première zone libre

	if (ad_zl == NULL || ad_zl == fb) return NULL; // Si il n'existe aucune zone libre, ou que la première zone libre est égale à celle donnée en argument, elle n'a donc pas de zone libre précédente

	while (ad_zl->next != NULL && ad_zl-> next != fb){ // Tant que la prochaine zone libre n'est pas nulle ou n'est pas égale à fb, alors on continue de parcourir
		ad_zl = ad_zl->next;
	}
	return ad_zl;
}

void *mem_alloc(size_t taille) {
	size_t taille_alignee = (taille +7)& ~ 7; //padding = 7 ALIGNEMENT
	size_t taille_alloc = taille_alignee + sizeof(size_t); // Taille totale qu'il va falloir allouer
	
	struct fb *fb = get_header()->fit(get_header()->first_free, taille); // On récupère la première zone libre où il sera possible d'allouer.

	if (fb == NULL || taille == 0){ // Si celle-ci est nulle, ou que la taille donnée en paramètre est nulle (donc rien à allouer), alors on n'alloue rien
		return NULL;
	}


	struct fb* fb_prec = get_prec(fb); // On obtient la zone libre (fb_prec) qui précède la zone libre où l'on va allouer de la mémoire (fb)
	struct fb* new_fb = ((void *)fb) + taille_alloc; // On recalcule l'adresse de la nouvelle zone libre si elle existe

	if (taille_alloc == fb->size + sizeof(size_t)){ // Cas où la taille de la zone qu'on alloue "fit" parfaitement (on utilise exactement la taille de la zone)
		new_fb = NULL; // Donc pas besoin de créer un nouveau free block
	}

	if (fb_prec != NULL){ // Cas où la zone où on alloue n'est pas la première zone libre 
		fb_prec->next = new_fb;
		if (new_fb != NULL){
			new_fb->next = fb->next;
			new_fb->size = fb->size - taille_alloc;
		}
	}
	else{ // Cas où la zone où on alloue est la première zone libre

		if (new_fb != NULL){ // Si on nécessite de créer un nouveau free block, alors on initialise ses champs
			new_fb->next = fb->next; 
			new_fb->size = fb->size - taille_alloc;
			get_header()->first_free = (struct fb*)new_fb; // Et il sera le nouveau premier free block 
		}
		else{
			get_header()->first_free = fb->next; // Sinon, on pointe sur le free block suivant de fb
		}
	}

	void *ptr = ((void *)fb); // Adresse de la nouvelle zone allouée (MD + Zone Utilisateur)
	*((size_t *)fb) = taille_alignee; // On stocke la taille de la nouvelle allouée
	
	return ptr;
}

void merge_fb(){
	struct fb* current = get_header()->first_free; 
	
	while (current != NULL){ // Tant qu'on n'a pas parcouru toute la liste des zones libres 

		void *next = (void *)current + current->size + sizeof(size_t); // Calcul de la prochaine zone libre à partir de l'actuelle (current)
		
		if (current->next != NULL && next == (void *)current->next){
			current->size = current->size + current->next->size + sizeof(size_t); // Somme des tailles des blocs sans oublier la taille de l'emplacement où on stocke la taille
			current->next = current->next->next;
		}
		else {
			current = current->next;
		}
		
	}
}


void mem_free(void* mem){

	void *block_address = ((void *)mem);

	size_t taille = *((size_t *)block_address); 

	struct fb *last = get_prec_void(mem); // On récupère la zone libre précédente

	// Création du nouveau free block
	struct fb* block_to_free = ((void *)mem);
	block_to_free->size = taille;
	
	if (last == NULL){ // Cas où il n'existe pas de zone libre qui précède mem : on ajoute block_to_free à la tête de la liste chaînée des free block
		block_to_free->next = get_header()->first_free;
		get_header()->first_free = block_to_free; // Donc le premier free block est celui qu'on vient de libérer
	}
	else{ // Cas où il existe une zone libre qui précède mem : on ajoute block_to_free à la liste chaînée des free blocks, entre last et last->next
		block_to_free->next = last->next;
		last->next = block_to_free; 
	}

	// Fusion des free block 
	merge_fb();

}

struct fb* mem_fit_first(struct fb *list, size_t size) {
	struct fb *zone_libre = list;
	while (zone_libre != NULL){
		if (zone_libre-> size >= size){
			return zone_libre;
		}
		zone_libre = zone_libre->next;
	}
	return NULL; // Aucune zone mémoire libre de taille supérieure ou égale à size n'a été trouvée
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	return *(size_t *)((void *)zone); // À l'adresse zone (début d'une zone libre ou occupée), se trouve déjà la taille maximale allouable de la zone en question
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}