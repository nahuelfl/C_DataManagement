// Vous devez modifier ce fichier pour le TP 2
// Tout votre code doit être dans ce fichier

#include <stdlib.h>
#include <stdio.h> 
#include "mymalloc.h"

// Structure de noeuds (liste simplement chainée)
typedef struct node 
{
	size_t size;
	int references;
	void* address;
	struct node* next;
}node;

static int alreadyInitialized = 0;
static int upgradeSize = 4 * 1024;	// Taille d'un grand bloc de mémoire
static int linkedListSize = 1024 * 1024;
node* root;

// Pour débugger seulement : montre explicitement le contenu de chaque noeud
void show() 
{
	node *current = root;
	int index = 0;
	while (current != NULL)
	{
		printf("===============================\n");
		printf("node %i size : %u\n", index, current->size);
		printf("node %i ref : %u\n", index, current->references);
		printf("node %i Ownaddress : %p\n", index, current);
		printf("node %i address : %p\n", index, current->address);
		printf("node %i next : %p\n", index, current->next);
		index++;
		current = current->next;
	}
}

// Retourne l'adresse d'une plage mémoire convenable selon la "size" demandée par l'utilisateur
void *mymalloc(size_t size) 
{
    size = (size + 15) & ~15; // Alignment sur un multiple de 16
	
	if (alreadyInitialized == 0) // Premier appel au mymalloc
	{
		if (size > upgradeSize)	// Pour être certain d'avoir un grand bloc
			upgradeSize = size * 2;

		root = malloc(sizeof(node) * linkedListSize); 
		void* mem = malloc(upgradeSize);
		root->size = size;
		root->references = 1;
		root->address = mem;

		node* empty = root + sizeof(node);
		empty->references = 0;
		empty->size = upgradeSize - size;
		empty->address = root->address + size;

		empty->next = NULL;
		root->next = empty;

		alreadyInitialized = 1;
		return root->address;
	}

	node* tmp = root;

	// Traverse la liste jusqu'au dernier bloc
	while (tmp->next != NULL)
	{ 
		if (tmp->size == size && tmp->references == 0) // Un bloc de taille parfaite a été trouvé
		{
			tmp->references = 1;
			return tmp->address;
		}
		else if (tmp->size > size && tmp->references == 0) // Un bloc plus grand a été trouvé
		{ 
			node* empty = tmp + sizeof(node);
			empty->size = tmp->size - size;
			empty->references = 0;
			empty->address = tmp->address + size;
			empty->next = tmp->next;

			tmp->size = size;
			tmp->references = 1;
			tmp->next = empty;
			return tmp->address;
		}
		tmp = tmp->next;
	}

	// Conditions d'allocation du dernier bloc de mémoire dans la liste

	if (tmp->size == size && tmp->references == 0) // Le dernier bloc fit parfaitement
	{ 
		tmp->references = 1;
		return tmp->address;
	}
	else if (tmp->size > size && tmp->references == 0) // Le dernier bloc est plus grand de ce qu'on a besoin
	{ 
		node* empty = tmp + sizeof(node);
		empty->size = tmp->size - size;
		empty->references = 0;
		empty->address = tmp->address + size;
		empty->next = NULL;

		tmp->size = size;
		tmp->references = 1;
		tmp->next = empty;
		return tmp->address;
	}
	else // Aucun bloc est convenable, on a besoin d'un autre appel à malloc
	{ 
		if (size > upgradeSize)	//  Pour être certain d'avoir un grand bloc
			upgradeSize = size; 

		upgradeSize = upgradeSize * 2; // On double la valeur a chaque fois que ca arrive : questions d'efficacité

		void* mem = malloc(upgradeSize);
		node* newEmpty = tmp + sizeof(node) * 2;
		newEmpty->size = upgradeSize - size;
		newEmpty->address = mem + size;
		newEmpty->references = 0;
		newEmpty->next = NULL;

		node* newNode = tmp + sizeof(node);
		newNode->size = size;
		newNode->address = mem;
		newNode->references = 1;
		newNode->next = newEmpty;
		tmp->next = newNode;
		return newNode->address;
	}
	return tmp->address;
}

// Incremente le compteur de references internes de la plage mémoire vers laquelle ptr pointe
// Retourne la valeur du compteur 
int refinc(void *ptr) 
{ 
	// Si le ptr est NULL, on retourne 0
	if (ptr == NULL) 
	{
		return 0;
	}

	// Recherche du ptr, voir si il a déjà été retourné par malloc
	node *current = root;
	while (current != NULL && current->address != (void*) ptr) 
	{
		current = current->next;
	}

	// Si le ptr est trouvé, son current est != NULL et il a au moins une référence, on incrémente
	if (current != NULL && current->references > 0) 
	{
		current->references++;
	}
	return current->references;
}

// Décremente le compteur de references internes de la plage mémoire vers laquelle ptr pointe
// Ne retourne rien
void myfree(void *ptr)
{
	// Si le ptr est NULL, rien à faire, return.
	if (ptr == NULL) 
	{
		return;
	}

	// Recherche du ptr, voir si il a déjà été retourné par malloc
	node *current = root;
	while (current != NULL && current->address != (void*)ptr) 
	{
		current = current->next;
	}

	// Si le ptr est trouvé, son current est != NULL et son compteur de référence est plus grand que zero 
	if (current != NULL && current->references > 0) 
	{
		current->references--;
	}
}
