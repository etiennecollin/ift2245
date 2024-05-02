#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma ide diagnostic ignored "readability-non-const-parameter"

#include "main.h"

uint8_t ilog2(uint32_t n) {
    uint8_t i = 0;
    while ((n >>= 1U) != 0)
        i++;
    return i;
}

//--------------------------------------------------------------------------------------------------------
//                                           DEBUT DU CODE
//--------------------------------------------------------------------------------------------------------

/**
 * Exercice 1
 *
 * Prend cluster et retourne son adresse en secteur dans l'archive
 * @param block le block de paramètre du BIOS
 * @param cluster le cluster à convertir en LBA
 * @param first_data_sector le premier secteur de données, donnée par la formule dans le document
 * @return le LBA
 */
uint32_t cluster_to_lba(BPB *block, uint32_t cluster, uint32_t first_data_sector) {
    uint32_t LBA = first_data_sector + (cluster - 2) * block->BPB_SecPerClus;
    return LBA;
}

uint32_t get_first_data_sector(BPB *block) {
    uint32_t root_dir_sectors = ((as_uint16(block->BPB_RootEntCnt) * 32) +
                                 (as_uint16(block->BPB_BytsPerSec) - 1)) / as_uint16(block->BPB_BytsPerSec);
    uint32_t fat_table_sectors = block->BPB_NumFATs * as_uint32(block->BPB_FATSz32);
    uint32_t first_data_sector = as_uint16(block->BPB_RsvdSecCnt) + fat_table_sectors + root_dir_sectors;

    return first_data_sector;
}

/**
 * Exercice 2
 *
 * Va chercher une valeur dans la cluster chain
 * @param block le block de paramètre du système de fichier
 * @param cluster le cluster qu'on veut aller lire
 * @param value un pointeur ou on retourne la valeur
 * @param archive le fichier de l'archive
 * @return un src d'erreur
 */
error_code get_cluster_chain_value(BPB *block, uint32_t cluster, uint32_t *value, FILE *archive) {
    uint32_t fat_table = as_uint16(block->BPB_RsvdSecCnt) * block->BPB_SecPerClus * as_uint16(block->BPB_BytsPerSec);
    uint32_t offset = fat_table + cluster * 4;

    // position cursor in the FAT table at the offset
    if (fseek(archive, offset, SEEK_SET) != 0) { // fseek returns 0 iff the seek was successful
        return GENERAL_ERR; // error positioning within the FAT file
    }

    // read the entry in the FAT table at the specified offset. This entry contains the next cluster of the file
    if (fread(value, sizeof(uint32_t), 1, archive) != 1) {
        return GENERAL_ERR; // error reading from the FAT
    }

    *value &= 0x0FFFFFFF; // only the first 28 bits in a FAT entry are relevant. Thus, the other 4 bits must be masked

    return NO_ERR;
}


/**
 * Exercice 3
 *
 * Vérifie si un descripteur de fichier FAT identifie bien fichier avec le nom name
 * @param entry le descripteur de fichier
 * @param name le nom de fichier
 * @return 0 ou 1 (faux ou vrai)
 */
bool file_has_name(FAT_entry *entry, char *name) {
    char filename[9]; // holds the filename (8 char) and the null terminator (1 char)
    strncpy(filename, entry->DIR_Name, 8);
    filename[8] = '\0'; // filename is null terminated

    // remove trailing spaces
    for (int i = 7; i >= 0; i--) {
        if (filename[i] == ' ') {
            filename[i] = '\0';
        } else {
            break;
        }
    }

    char extension[4]; // holds the extension (8 char) and the null terminator (1 char)
    strncpy(extension, entry->DIR_Name + 8, 3);
    extension[3] = '\0';

    // remove trailing spaces (because there might not be an extension)
    for (int i = 2; i >= 0; i--) {
        if (extension[i] == ' ') {
            extension[i] = '\0';
        } else {
            break;
        }
    }

    char clean_entry_name[12]; // filename => 8 char; extension => 3 char; null terminator => 1 char; 8 + 3 + 1 = 12
    snprintf(clean_entry_name, sizeof(clean_entry_name), "%s%s", filename, extension);

    // remove '.' in name if it exists
    char *dot = strchr(name, '.');
    if (dot != NULL) {
        // merge the filename and the extension into a single string
        char clean_name[12];

        strncpy(extension, dot + 1, 3);
        strncpy(filename, name, dot - name);
        snprintf(clean_name, sizeof(clean_name), "%s%s", filename, extension);

        return strcasecmp(clean_entry_name, clean_name) == 0;
    }

    // strcasecmp returns 0 iff both strings are the same. strcasecmp is not case-sensitive
    return strcasecmp(clean_entry_name, name) == 0;
}

/**
 * Exercice 4
 *
 * Prend un path de la forme "/dossier/dossier2/fichier.ext et retourne la partie
 * correspondante à l'index passé. Le premier '/' est facultatif.
 * @param path l'index passé
 * @param level la partie à retourner (ici, 0 retournerait dossier)
 * @param output la sortie (la string)
 * @return -1 si les arguments sont incorrects, -2 si le path ne contient pas autant de niveaux
 * -3 si out of memory
 */
error_code break_up_path(char *path, uint8_t level, char **output) {
    if (path == NULL || level < 0) {
        return -1; // invalid arguments
    }

    int level_counter = 0;

    // find the position of the beginning of the part of the path
    char *ptr = path;

    if (*ptr == '.' && *(ptr + 1) == '.' && *(ptr + 2) == '/') {
        return -1; // invalid arguments
    }

    // case where the path starts with '/'. Must not be counted.
    if (*ptr == '/' || *ptr == '.') {
        ptr++;
    }
    while (*ptr == '/') { // skip consecutive '/'
        ptr++;
    }

    // initialize the start pointer to the beginning of the path
    char *start_ptr = ptr;

    // iterate through the path to find the part of the path
    while (*ptr != '\0' && level_counter <= level) {
        if (*ptr == '/') {
            level_counter++;
            if (level_counter == level + 1) {
                break;
            }

            while (*ptr == '/') { // skip consecutive '/'
                ptr++;
            }
            start_ptr = ptr;
        } else if (*(ptr - 1) == '/' && *ptr == '.') {
            if (*(ptr + 1) == '/') {
                ptr += 2;
                start_ptr = ptr;
            } else if (*(ptr + 1) == '.' && *(ptr + 2) == '/') {
                level_counter--;
                ptr += 3;
            }
        } else {
            ptr++;
        }
    }

    // check whether the level is within bounds
    if (level > level_counter) {
        return -2; // not enough levels in the path
    }

    // find the end of the part of the path
    long length = ptr - start_ptr;

    // allocate memory
    *output = (char *) malloc(length + 1);
    if (*output == NULL) {
        return -3; // memory allocation error
    }

    // copy the substring into the output string
    memcpy(*output, start_ptr, length);
    (*output)[length] = '\0'; // add null terminator

    // convert the output string to uppercase
    // https://www.tutorialspoint.com/convert-a-string-to-uppercase-in-c
    for (int i = 0; (*output)[i] != '\0'; i++) {
        if ((*output)[i] >= 'a' && (*output)[i] <= 'z') {
            (*output)[i] = (*output)[i] - 32;
        }
    }

    return NO_ERR; // include the null character
}


/**
 * Exercice 5
 *
 * Lit le BIOS parameter block
 * @param archive fichier qui correspond à l'archive
 * @param block le block alloué
 * @return un src d'erreur
 */
error_code read_boot_block(FILE *archive, BPB **block) {
    if (archive == NULL || block == NULL) {
        return -1;
    }

    // allocate memory for the BPB structure
    *block = (BPB *) malloc(sizeof(BPB));
    if (*block == NULL) {
        return -3; // memory allocation error
    }

    fseek(archive, 0, SEEK_SET); // the BPB is always the first sector of the volume

    // read BPB from file into allocated memory
    size_t bytes_read = fread(*block, sizeof(BPB), 1, archive);
    if (bytes_read != 1) {
        free(*block); //  free allocated memory
        return -4; // read error
    }

    return NO_ERR;
}

/**
 * Exercice 6
 *
 * Trouve un descripteur de fichier dans l'archive
 * @param archive le descripteur de fichier qui correspond à l'archive
 * @param path le chemin du fichier
 * @param entry l'entrée trouvée
 * @return un src d'erreur
 */
error_code find_file_descriptor(FILE *archive, BPB *block, char *path, FAT_entry **entry) {
    uint32_t root_cluster = as_uint32(block->BPB_RootClus);
    long root_lba = cluster_to_lba(block, root_cluster, get_first_data_sector(block));

    *entry = (FAT_entry *) malloc(sizeof(FAT_entry));
    if (*entry == NULL) {
        return GENERAL_ERR; // memory allocation error
    }

    // Initialize some variables
    int i = 0;
    int depth = 0;
    char *name = NULL;
    int status = break_up_path(path, depth, &name);
    if (status == -1 || status == -3) {
        return GENERAL_ERR;
    }

    while (true) {
        if (fseek(archive, root_lba + i * 32, SEEK_SET) != 0) { // fseek returns 0 iff the seek was successful
            return GENERAL_ERR; // error positioning within the FAT file
        }

        // read the entry at the specified offset
        if (fread(entry, sizeof(FAT_entry), 1, archive) != 1) {
            return GENERAL_ERR; // error reading from the FAT
        }

        if (file_has_name(*entry, name)) {
            depth++;
            status = break_up_path(path, depth, &name);

            // check if the path has been fully traversed
            if (status == -2) {
                break;
            } else if (status < 0) {
                return GENERAL_ERR;
            }

            // check if the entry is a directory
            if (((*entry)->DIR_Attr & 0x10) != 0) {
                i = 0;
                root_cluster = (as_uint32((*entry)->DIR_FstClusHI) << 16) + as_uint32((*entry)->DIR_FstClusLO);
                root_lba = cluster_to_lba(block, root_cluster, get_first_data_sector(block));
            } else {
                // the entry is a file but the path has not been fully traversed
                return GENERAL_ERR;
            }
        } else {
            // check if the entry is the last entry in the root directory
            if ((*entry)->DIR_Name[0] == 0x00) {
                return GENERAL_ERR;
            }
        }
    }

    return 0;
}

/**
 * Exercice 7
 *
 * Lit un fichier dans une archive FAT
 * @param entry l'entrée du fichier
 * @param buff le buffer ou écrire les données
 * @param max_len la longueur du buffer
 * @return un src d'erreur qui va contenir la longueur des donnés lues
 */
error_code read_file(FILE *archive, BPB *block, FAT_entry *entry, void *buff, size_t max_len) {

    uint32_t *value;
    bool not_end_of_file = true;
    error_code bytes_read = 0;
    uint32_t cluster =
            (as_uint32((*entry).DIR_FstClusHI) << 16) + as_uint32((*entry).DIR_FstClusLO); // first cluster to be read
    *value = cluster;

    while (not_end_of_file) {
        if (bytes_read > max_len) {
            return -3; // buffer overflow
        }

        uint32_t lba = cluster_to_lba(block, *value, get_first_data_sector(block));

        // read cluster into buffer
        fseek(archive, lba, SEEK_SET);
        fread(buff, block->BPB_SecPerClus * as_uint16(block->BPB_BytsPerSec), 1, archive);
        bytes_read += block->BPB_SecPerClus * as_uint16(block->BPB_BytsPerSec);

        // find next cluster to read
        get_cluster_chain_value(block, cluster, value, archive);

        if (*value == 0xFFFFFF8) { // end of file has been reached
            not_end_of_file = false;
        }
    }

    return bytes_read;
}

// ༽つ۞﹏۞༼つ

int main() {
    FAT_entry entry;
    entry.DIR_Name[0] = 65;
    entry.DIR_Name[1] = 78;
    entry.DIR_Name[2] = 65;
    entry.DIR_Name[3] = 77;
    entry.DIR_Name[4] = 69;
    entry.DIR_Name[5] = 32;
    entry.DIR_Name[6] = 32;
    entry.DIR_Name[7] = 32;
    entry.DIR_Name[8] = 32;
    entry.DIR_Name[9] = 32;
    entry.DIR_Name[10] = 32;

    printf("Name 1a: %d\n", file_has_name(&entry, "ANAME"));
    printf("Name 1b: %d\n", !file_has_name(&entry, "NAME"));
    printf("Name 1c: %d\n", !file_has_name(&entry, "ANAM"));

    entry.DIR_Name[0] = 65;
    entry.DIR_Name[1] = 78;
    entry.DIR_Name[2] = 65;
    entry.DIR_Name[3] = 77;
    entry.DIR_Name[4] = 69;
    entry.DIR_Name[5] = 32;
    entry.DIR_Name[6] = 32;
    entry.DIR_Name[7] = 32;
    entry.DIR_Name[8] = 65;
    entry.DIR_Name[9] = 65;
    entry.DIR_Name[10] = 65;

    printf("Name 2a: %d\n", !file_has_name(&entry, "ANAME"));
    printf("Name 2b: %d\n", file_has_name(&entry, "ANAME.AAA"));


    char *path = "first/second/third/fourth";
    char *read = NULL;
    break_up_path(path, 0, &read);
    printf("Path 1: %d, %s\n", strcasecmp(read, "FIRST"), read);

    read = NULL;
    break_up_path(path, 1, &read);
    printf("Path 3: %d, %s\n", strcasecmp(read, "SECOND"), read);

    read = NULL;
    break_up_path(path, 2, &read);
    printf("Path 4: %d, %s\n", strcasecmp(read, "THIRD"), read);

    read = NULL;
    break_up_path(path, 3, &read);
    printf("Path 5: %d, %s\n", strcasecmp(read, "FOURTH"), read);

    free(read);

    FILE *archive = fopen("../floppy.img", "rb");
    BPB *bpb = NULL;
    read_boot_block(archive, &bpb);

    FAT_entry *e = NULL;
    printf("File Descriptor 1: %d", find_file_descriptor(archive, bpb, "notexist", &e) < 0);

    e = NULL;
    printf("File Descriptor 2: %d", find_file_descriptor(archive, bpb, "zola.txt", &e) >= 0);

    e = NULL;
    printf("File Descriptor 3: %d", find_file_descriptor(archive, bpb,  "afolder/another/candide.txt", &e) >= 0);

    e = NULL;
    printf("File Descriptor 4: %d", find_file_descriptor(archive, bpb, "afolder/los.txt", &e) < 0, 1);

    e = NULL;
    printf("File Descriptor 5: %d", find_file_descriptor(archive, bpb, "afolder/spansih/titan.txt", &e) < 0 );

    free(e);

}

// ༽つ۞﹏۞༼つ