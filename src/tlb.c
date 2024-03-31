
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tlb.h"

#include "conf.h"
#include "time.h"

struct tlb_entry
{
  unsigned int page_number;
  int frame_number;             /* Invalide si négatif.  */
  bool readonly : 1;
};

static FILE *tlb_log = NULL;
static struct tlb_entry tlb_entries[TLB_NUM_ENTRIES]; 

static unsigned int tlb_hit_count = 0;
static unsigned int tlb_miss_count = 0;
static unsigned int tlb_mod_count = 0;

/* Initialise le TLB, et indique où envoyer le log des accès.  */
void tlb_init (FILE *log)
{
  for (int i = 0; i < TLB_NUM_ENTRIES; i++)
    tlb_entries[i].frame_number = -1;
  tlb_log = log;
}

/******************** ¡ NE RIEN CHANGER CI-DESSUS !  ******************/

/* Recherche dans le TLB.
 * Renvoie le `frame_number`, si trouvé, ou un nombre négatif sinon.  */
static int tlb__lookup (unsigned int page_number, bool write)
{
  for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
      if (tlb_entries[i].page_number == page_number) { // page found in TLB
          return tlb_entries[i].frame_number;
      }
  }
  return -1; // page is not in TLB // TODO maybe add the page number and frame number to TLB, p.366 of the book
}

/* Ajoute dans le TLB une entrée qui associe `frame_number` à
 * `page_number`.  */
// TODO : can we keep an attribute for TLB num of elements that tells us how many pages are in the TLB?
static void tlb__add_entry (unsigned int page_number, unsigned int frame_number, bool readonly)
{
  for (int i = 0; i < TLB_NUM_ENTRIES; i++) {
      if (tlb_entries[i].frame_number < 0) { // TLB empty slot found
          tlb_entries[i].page_number = page_number;
          tlb_entries[i].frame_number = frame_number;
          tlb_entries[i].readonly = readonly;
          return;
      }
  }

  // no empty slot found. Replacement algorithm must be used.
  int victim = rand() % TLB_NUM_ENTRIES;
  tlb_entries[victim].page_number = page_number;
  tlb_entries[victim].frame_number = frame_number;
  tlb_entries[victim].readonly = readonly; // TODO seed the random number generator at the start of the program
}


/******************** ¡ NE RIEN CHANGER CI-DESSOUS !  ******************/

void tlb_add_entry (unsigned int page_number,
                    unsigned int frame_number, bool readonly)
{
  tlb_mod_count++;
  tlb__add_entry (page_number, frame_number, readonly);
}

int tlb_lookup (unsigned int page_number, bool write)
{
  int fn = tlb__lookup (page_number, write);
  (*(fn < 0 ? &tlb_miss_count : &tlb_hit_count))++;
  return fn;
}

/* Imprime un sommaires des accès.  */
void tlb_clean (void)
{
  fprintf (stdout, "TLB misses   : %3u\n", tlb_miss_count);
  fprintf (stdout, "TLB hits     : %3u\n", tlb_hit_count);
  fprintf (stdout, "TLB changes  : %3u\n", tlb_mod_count);
  fprintf (stdout, "TLB miss rate: %.1f%%\n",
           100 * tlb_miss_count
           /* Ajoute 0.01 pour éviter la division par 0.  */
           / (0.01 + tlb_hit_count + tlb_miss_count));
}
