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
static FILE *vmm_log;


void vmm_init(FILE *log) {
    // Initialise le fichier de journal.
    vmm_log = log;
}


// NE PAS MODIFIER CETTE FONCTION
static void vmm_log_command(FILE *out, const char *command,
                            unsigned int laddress, /* Logical address. */
                            unsigned int page,
                            unsigned int frame,
                            unsigned int offset,
                            unsigned int paddress, /* Physical address.  */
                            char c) /* Caractère lu ou écrit.  */
{
    if (out)
        fprintf(out, "%s[%c]@%05d: p=%d, o=%d, f=%d pa=%d\n", command, c, laddress,
                page, offset, frame, paddress);
}

/* Effectue une lecture à l'adresse logique 'laddress'.  */
char vmm_read(unsigned int laddress) {
    char c = '!';
    read_count++;

    unsigned int temp = laddress;
    unsigned int page_offset = temp & 0xFF; // get first 8 bits of the address
    unsigned int page_number = temp / PAGE_FRAME_SIZE; // get the last 24 bits of the address

    // get frame number
    int frame = get_frame(page_number, false);

    // build physical address and read from it
    unsigned int physical_address = get_physical_address(frame, page_offset);
    c = pm_read(page_number);

    vmm_log_command(stdout, "READING", laddress, page_number, frame, page_offset, physical_address, c);
    return c;
}

/* Effectue une écriture à l'adresse logique 'laddress'.  */
void vmm_write(unsigned int laddress, char c) {
    write_count++;
    unsigned int temp = laddress;
    unsigned int page_offset = temp & 0xFF; // get first 8 bits of the address
    unsigned int page_number = temp / PAGE_FRAME_SIZE; // get the last 24 bits of the address

    // get frame number
    int frame = get_frame(page_number, true);

    // build physical address
    unsigned int physical_address = get_physical_address(frame, page_offset);
    pm_write(page_number, c);

    // update dirty bit
    pt_set_readonly(page_number, false);

    vmm_log_command(stdout, "WRITING", laddress, page_number, frame, page_offset, physical_address, c);
}

unsigned int get_physical_address(unsigned int frame, unsigned int page_offset) {
    // build physical address
    unsigned int physical_address = frame;
    physical_address = physical_address * PAGE_FRAME_SIZE;
    physical_address = physical_address | page_offset;
    return physical_address;
}

int get_frame(unsigned int page_number, bool write) {
    int frame;
    LOOKUP:
    // check if the page is in the TLB
    frame = tlb_lookup(page_number, write);

    // page is not in the TLB
    if (frame == -1) {
        // check if the page is in the page table
        frame = pt_lookup(page_number);
        // page fault
        if (frame == -1) {
            frame = find_free_frame_number();
            if (frame == -1) {
                frame = find_victim_frame_number();

                // find the page number with the TLB
                int page = remove_page_from_tlb(frame);

                // check if the page was in the TLB
                if (page != -1) {
                    // if not, find it in the page table
                    page = pt_find_page(frame);
                }

                // check if the page was modified
                if (pt_readonly_p(page) == false) {
                    // save the page to the backing store
                    pm_backup_page(frame, page);
                }

                // update the page table
                pt_unset_entry(page);
            }


            // download the page from the backing store
            pm_download_page(page_number, frame);
            // update the page table
            pt_set_entry(page_number, frame);
            goto LOOKUP;
        }
        // add the page to the TLB
        tlb_add_entry(page_number, frame, pt_readonly_p(page_number));
    }
    return frame;
}

// NE PAS MODIFIER CETTE FONCTION
void vmm_clean(void) {
    fprintf(stdout, "VM reads : %4u\n", read_count);
    fprintf(stdout, "VM writes: %4u\n", write_count);
}
