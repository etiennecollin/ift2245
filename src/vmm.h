#ifndef VMM_H
#define VMM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "conf.h"

void vmm_init(FILE *log);

char vmm_read(unsigned int logical_address);

void vmm_write(unsigned int logical_address, char);

void vmm_clean(void);

unsigned int get_physical_address(unsigned int frame, unsigned int page_offset);

int get_frame(unsigned int page_number);

#endif
