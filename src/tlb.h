#ifndef TLB_H
#define TLB_H

#include <stdio.h>
#include <stdbool.h>

/* Initialise le TLB, et indique où envoyer le log des accès.  */
void tlb_init (FILE *log);

/* Recherche dans le TLB.
 * Renvoie le `frame_number`, si trouvé et l'accès est valide,
 * ou un nombre négatif sinon.  */
int tlb_lookup (unsigned int page_number, bool write);

/* Ajoute dans le TLB une entrée qui associe `frame_number` à
 * `page_number` avec droits d'accès `readonly`.  */
void tlb_add_entry (unsigned int page_number,
                    unsigned int frame_number, bool readonly);

void tlb_clean (void);

#endif
