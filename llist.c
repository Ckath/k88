#include "llist.h"

void 
pop(llist *ll, char *name)
{
    llnode *n = ll->head;
    if (!strcmp(name, n->name)) {
        free(ll->head);
        ll->head = n->next;
        return;
    }
    while (strcmp(name, n->next->name)) {
        if (n->next->next == NULL) {
            return;
        }
        n = n->next;
    }

    llnode *nn = n->next->next;
    free(n->next);
    n->next = nn;
}

void
push(llist *ll, char *name)
{
    llnode *n= malloc(sizeof(llnode));
    strcpy(n->name, name);
    n->next = ll->head;
    ll->head = n;
}
