#ifndef PT_H
#define PT_H

#include <stdio.h>
#include <stdbool.h>
#include "conf.h"

struct page
{
  bool readonly : 1;
  bool valid : 1;
  unsigned int frame_number : 16;
};
static struct page page_table[NUM_PAGES];


/* Initialise la page table, et indique où envoyer le log des accès.  */
void pt_init (FILE *log);

/* Recherche dans la page table.
 * Renvoie le `frame_number`, si trouvé, ou un nombre négatif sinon.  */
int pt_lookup (unsigned int page_number);

/* Modifie l'entrée de `page_number` dans la page table pour qu'elle
 * pointe vers `frame_number`.  */
void pt_set_entry (unsigned int page_number, unsigned int frame_number);

/* Marque l'entrée de `page_number` dans la page table comme invalide.  */
void pt_unset_entry (unsigned int page_number);

/* Renvoie si `page_number` est `readonly`.  */
bool pt_readonly_p (unsigned int page_number);

/* Change l'accès en écriture de `page_number` selon `readonly`.  */
void pt_set_readonly (unsigned int page_number, bool readonly);

void pt_clean (void);

#endif
