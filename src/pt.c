
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pt.h"

#include "conf.h"


static FILE *pt_log = NULL;

static unsigned int pt_lookup_count = 0;
static unsigned int pt_page_fault_count = 0;
static unsigned int pt_set_count = 0;

/* Initialise la table des pages, et indique où envoyer le log des accès.  */
void pt_init(FILE *log) {
    for (unsigned int i = 0; i < NUM_PAGES; i++)
        page_table[i].valid = false;
    pt_log = log;
}

/******************** ¡ NE RIEN CHANGER CI-DESSUS !  ******************/

/* Recherche dans la table des pages.
 * Renvoie le 'frame_number', si valide, ou un nombre négatif sinon.  */
static int pt__lookup(unsigned int page_number) {
    if (page_table[page_number].valid) {
        return page_table[page_number].frame_number;
    }
    return -1;
}

/* Modifie l'entrée de 'page_number' dans la page table pour qu'elle pointe vers 'frame_number'.  */
static void pt__set_entry(unsigned int page_number, unsigned int frame_number) {
    page_table[page_number].valid = true;
    page_table[page_number].readonly = true;
    page_table[page_number].frame_number = frame_number;
}

/* Marque l'entrée de 'page_number' dans la page table comme invalide.  */
void pt_unset_entry(unsigned int page_number) {
    page_table[page_number].valid = false;
}

/* Renvoie si 'page_number' est 'readonly'.  */
bool pt_readonly_p(unsigned int page_number) {
    return page_table[page_number].readonly;
}

bool pt_valid_p(unsigned int page_number) {
    return page_table[page_number].valid;
}

int pt_find_page(unsigned int frame_number) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].frame_number == frame_number) {
            return i;
        }
    }
    return -1;
}

/* Change l'accès en écriture de 'page_number' selon 'readonly'.  */
void pt_set_readonly(unsigned int page_number, bool readonly) {
    page_table[page_number].readonly = readonly;
}


int find_victim_frame_number() {
    // the victim is the first frame of a page which is not readonly
    // in the worst case, we will have to loop through all the pages
    // we start at a random index to avoid always selecting the same page
    for (int i = 0; i < NUM_PAGES; i++) {
        int index = ((rand() % NUM_PAGES) + i) % NUM_PAGES;
        if (page_table[index].valid && !page_table[index].readonly) {
            return page_table[index].frame_number;
        }
    }
    // all pages are readonly
    return rand() % NUM_FRAMES;
}

/******************** ¡ NE RIEN CHANGER CI-DESSOUS !  ******************/

void pt_set_entry(unsigned int page_number, unsigned int frame_number) {
    pt_set_count++;
    pt__set_entry(page_number, frame_number);
}

int pt_lookup(unsigned int page_number) {
    pt_lookup_count++;
    int fn = pt__lookup(page_number);
    if (fn < 0) pt_page_fault_count++;
    return fn;
}

/* Imprime un sommaires des accès.  */
void pt_clean(void) {
    fprintf(stdout, "PT lookups   : %3u\n", pt_lookup_count);
    fprintf(stdout, "PT changes   : %3u\n", pt_set_count);
    fprintf(stdout, "Page Faults  : %3u\n", pt_page_fault_count);

    if (pt_log) {
        for (unsigned int i = 0; i < NUM_PAGES; i++) {
            fprintf(pt_log,
                    "%d -> {%d,%s%s}\n",
                    i,
                    page_table[i].frame_number,
                    page_table[i].valid ? "" : " invalid",
                    page_table[i].readonly ? " readonly" : "");
        }
    }
}
