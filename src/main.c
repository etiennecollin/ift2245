#include <stdio.h>
#include <stdlib.h>

#include "os.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    return os_start(argv[1]);
}