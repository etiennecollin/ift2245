#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "conf.h"
#include "common.h"
#include "vmm.h"
#include "tlb.h"
#include "pt.h"
#include "pm.h"


static unsigned int read_count = 0;
static unsigned int write_count = 0;
static FILE* vmm_log;


void vmm_init (FILE *log)
{
  // Initialise le fichier de journal.
  vmm_log = log;
}


// NE PAS MODIFIER CETTE FONCTION
static void vmm_log_command (FILE *out, const char *command,
                             unsigned int laddress, /* Logical address. */
		             unsigned int page,
                             unsigned int frame,
                             unsigned int offset,
                             unsigned int paddress, /* Physical address.  */
		             char c) /* Caractère lu ou écrit.  */
{
  if (out)
    fprintf (out, "%s[%c]@%05d: p=%d, o=%d, f=%d pa=%d\n", command, c, laddress,
	     page, offset, frame, paddress);
}

/* Effectue une lecture à l'adresse logique `laddress`.  */
char vmm_read (unsigned int laddress)
{
  char c = '!';
  read_count++;

  unsigned int temp = laddress;
  unsigned int page_offset = temp & 0xFF; // get first 8 bits of the address
  unsigned int page_number = (temp >> 8) & 0xFF; // get the last 8 bits of the address

  // check if the page is in the TLB
  int frame = tlb_lookup(page_number, false);
  if (frame == -1) { // page is not in the TLB. Check in page table.
      frame = pt_lookup(page_number);
      if (frame == -1) { // frame number is not valid TODO: handle this case

      }
  }

  // build physical address
  unsigned int physical_address = frame;
  physical_address = physical_address << 8;
  physical_address = physical_address | page_offset;

  vmm_log_command (stdout, "READING", laddress, page_number, frame, page_offset, physical_address, c);
  return c;
}

/* Effectue une écriture à l'adresse logique `laddress`.  */
void vmm_write (unsigned int laddress, char c)
{
  write_count++;
  unsigned int temp = laddress;
  unsigned int page_offset = temp & 0xFF; // get first 8 bits of the address
  unsigned int page_number = (temp >> 8) & 0xFF; // get the last 8 bits of the address

  // check if the page is in the TLB
  int frame = tlb_lookup(page_number, false);
  if (frame == -1) { // page is not in the TLB. Check in page table.
      // frame = pt_lookup(page_number);
      frame = pt_lookup(page_number);
      if (frame == -1) { // frame number is not valid TODO: handle this case

        }
    }

    // build physical address
    unsigned int physical_address = frame;
    physical_address = physical_address << 8;
    physical_address = physical_address | page_offset;

  vmm_log_command (stdout, "WRITING", laddress, page_number, frame, page_offset, physical_address, c);
}


// NE PAS MODIFIER CETTE FONCTION
void vmm_clean (void)
{
  fprintf (stdout, "VM reads : %4u\n", read_count);
  fprintf (stdout, "VM writes: %4u\n", write_count);
}
