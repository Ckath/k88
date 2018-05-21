#ifndef LLIST_H
#define LLIST_H

#include "extern.h"

typedef struct llnode llnode;

struct llnode {
    char name[100];
    llnode *next;
};

typedef struct {
    llnode *head;
} llist;

void pop(llist *ll, char *name);
void push(llist *ll, char *name);

#endif
