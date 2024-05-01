
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tlb.h"

#include "conf.h"
#include "time.h"

struct tlb_entry {
    unsigned int page_number;
    int frame_number;             /* Invalide si négatif.  */
    bool readonly: 1;
    unsigned int counter;
};

static FILE *tlb_log = NULL;
static struct tlb_entry tlb_entries[TLB_NUM_ENTRIES];

static unsigned int tlb_hit_count = 0;
static unsigned int tlb_miss_count = 0;
static unsigned int tlb_mod_count = 0;

/* Initialise le TLB, et indique où envoyer le log des accès.  */
void tlb_init(FILE *log) {
    for (int i = 0; i < TLB_NUM_ENTRIES; i++)
        tlb_entries[i].frame_number = -1;
    tlb_log = log;
}

/******************** ¡ NE RIEN CHANGER CI-DESSUS !  ******************/

/* Recherche dans le TLB.
 * Renvoie le 'frame_number', si trouvé, ou un nombre négatif sinon.  */
static int tlb__lookup(unsigned int page_number, bool write) {
    increment_counters();
    for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
        if (tlb_entries[i].frame_number >= 0 && tlb_entries[i].page_number == page_number) {
            // page found in TLB
            tlb_entries[i].counter = 0;
            if (tlb_entries[i].readonly && write) {
                tlb_entries[i].readonly = false;
            }
            return tlb_entries[i].frame_number;
        }
    }
    return -1; // page is not in TLB
}

void increment_counters() {
    for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
        if (tlb_entries[i].frame_number >= 0) {
            tlb_entries[i].counter++;
        }
    }
}

static unsigned int get_least_recently_used() {
    int lru = 0;
    bool lru_readonly = tlb_entries[lru].readonly;
    for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
        // if the entry is valid and has a higher counter than the current LRU
        if (tlb_entries[i].frame_number >= 0 && tlb_entries[i].counter >= tlb_entries[lru].counter) {
            // if the entry has the same counter as the current LRU
            if (tlb_entries[lru].counter == tlb_entries[i].counter) {
                // if the current LRU is dirty and the new entry is not
                if (!tlb_entries[lru].readonly && tlb_entries[i].readonly) {
                    lru = i;
                } else {
                    continue;
                }
            } else {
                lru = i;
            }
        }
    }

    return lru;
}

/* Ajoute dans le TLB une entrée qui associe 'frame_number' à 'page_number'.  */
static void tlb__add_entry(unsigned int page_number, unsigned int frame_number, bool readonly) {
    int victim = -1;
    // check if there is an available entry
    for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
        if (tlb_entries[i].frame_number == -1) {
            victim = i;
        }
    }

    // else find the least recently used entry
    if (victim == -1) {
        victim = get_least_recently_used();
    }

    // add entry
    tlb_entries[victim].page_number = page_number;
    tlb_entries[victim].frame_number = frame_number;
    tlb_entries[victim].counter = 0;
    tlb_entries[victim].readonly = readonly;
}

int remove_page_from_tlb(unsigned int frame) {
    for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
        if (tlb_entries[i].frame_number == frame) {
            tlb_entries[i].frame_number = -1;
            return tlb_entries[i].page_number;
        }
    }
    return -1;
}


/******************** ¡ NE RIEN CHANGER CI-DESSOUS !  ******************/

void tlb_add_entry(unsigned int page_number,
                   unsigned int frame_number, bool readonly) {
    tlb_mod_count++;
    tlb__add_entry(page_number, frame_number, readonly);
}

int tlb_lookup(unsigned int page_number, bool write) {
    int fn = tlb__lookup(page_number, write);
    (*(fn < 0 ? &tlb_miss_count : &tlb_hit_count))++;
    return fn;
}

/* Imprime un sommaires des accès.  */
void tlb_clean(void) {
    fprintf(stdout, "TLB misses   : %3u\n", tlb_miss_count);
    fprintf(stdout, "TLB hits     : %3u\n", tlb_hit_count);
    fprintf(stdout, "TLB changes  : %3u\n", tlb_mod_count);
    fprintf(stdout, "TLB miss rate: %.1f%%\n",
            100 * tlb_miss_count
            /* Ajoute 0.01 pour éviter la division par 0.  */
            / (0.01 + tlb_hit_count + tlb_miss_count));
}
