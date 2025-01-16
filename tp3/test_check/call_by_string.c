#include <fcntl.h>
#include <stdio.h>
#include <elf.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

// Taken from
// https://stackoverflow.com/a/1118808/11296012
// but the post was wrong: it really only gives the *offset*, not the address
// requires libelf https://github.com/WolfgangSt/libelf
// this technique is absolutely cursed, don't use it in production
// Reading the proc's own ELF file while it is running can only be good, right?
int get_offset_by_string(char *string) {
    Elf64_Shdr *shdr;
    Elf64_Ehdr *ehdr;
    Elf *elf;
    Elf_Scn *scn;
    Elf_Data *data;
    int cnt;

    int fd = 0;

    /* This is probably Linux specific - Read in our own executable*/
    if ((fd = open("/proc/self/exe", O_RDONLY)) == -1)
        exit(1);

    elf_version(EV_CURRENT);

    if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
        fprintf(stderr, "file is not an ELF binary\n");
        exit(1);
    }
    /* Let's get the elf sections */
    if (((ehdr = elf64_getehdr(elf)) == NULL) ||
        ((scn = elf_getscn(elf, ehdr->e_shstrndx)) == NULL) ||
        ((data = elf_getdata(scn, NULL)) == NULL)) {
        fprintf(stderr, "Failed to get SOMETHING\n");
        exit(1);
    }

    /* Let's go through each elf section looking for the symbol table */
    for (cnt = 1, scn = NULL; scn = elf_nextscn(elf, scn); cnt++) {
        if ((shdr = elf64_getshdr(scn)) == NULL)
            exit(1);

        if (shdr->sh_type == SHT_SYMTAB) {
            char *name;
            char *strName;
            data = 0;
            if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0) {
                fprintf(stderr, "No data in symbol table\n");
                exit(1);
            }

            Elf64_Sym *esym = (Elf64_Sym *) data->d_buf;
            Elf64_Sym *lastsym = (Elf64_Sym *) ((char *) data->d_buf + data->d_size);

            /* Look through all symbols */
            for (; esym < lastsym; esym++) {
                if ((esym->st_value == 0) ||
                    (ELF64_ST_BIND(esym->st_info) == STB_WEAK) ||
                    (ELF64_ST_BIND(esym->st_info) == STB_NUM) ||
                    (ELF64_ST_TYPE(esym->st_info) != STT_FUNC))
                    continue;

                name = elf_strptr(elf, shdr->sh_link, (size_t) esym->st_name);

                if (!name) {
                    fprintf(stderr, "%sn", elf_errmsg(elf_errno()));
                    exit(-1);
                }
                /* This could obviously be a generic string */
                if (strcmp(string, name) == 0) {
                    //printf("Found func @ %x\n", esym->st_value);
                    return esym->st_value;
                }
            }

        }
    }
    fprintf(stderr, "Could not reflect. A shame. I will kill your program now :)");
    exit(-1);
}

// From the ELF we can the offset. But we can't call an offset like a function.
// So, we get the offset of a known function (a function that we can call without a string).
// Then, we get the actual address of that same function, and we subtract the offset from it.
// We now know the base address.
// Then, we get the offset of the function we actually want to call by string, and add said
// offset to the base address. We thus get the function pointer of the string's function.
void call_by_string(char *string) {
    void *base_addr_plus_offset = get_offset_by_string;
    int initial_offset = get_offset_by_string("get_offset_by_string");
    void *base_addr = base_addr_plus_offset - initial_offset;

    void *func_addr = base_addr + get_offset_by_string(string);
    ((int (*)(void))func_addr)();
}