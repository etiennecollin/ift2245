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

#define fiffnnull(e) do {if(NULL != (e)) free(e);} while(0)

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
    uint32_t root_dir_sectors = ((as_uint16(block->BPB_RootEntCnt) * 32) + (as_uint16(block->BPB_BytsPerSec) - 1)) /
                                as_uint16(block->BPB_BytsPerSec);
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

static void free_stack(char **stack, unsigned long allocated_size) {
    for (int i = 0; i < allocated_size; i++) {
        fiffnnull(stack[i]);
    }
    fiffnnull(stack);
}

int get_simplified_path(char *path, char ***output_stack) {
    unsigned long path_len = strlen(path);
    unsigned long allocated_size = path_len + 1;
    int stackIndex = 0;

    // Stack of strings to store directories
    *output_stack = (char **) malloc(allocated_size * sizeof(char *));
    if (*output_stack == NULL) {
        return -1;
    }

    // Allocate memory for each string in the stack
    for (int i = 0; i < allocated_size; i++) {
        (*output_stack)[i] = (char *) malloc(allocated_size * sizeof(char));
        if ((*output_stack)[i] == NULL) {
            free_stack(*output_stack, allocated_size);
            return -1;
        }
    }

    // Create temporary string to create the current string
    char *temp = (char *) malloc(allocated_size * sizeof(char));
    if (temp == NULL) {
        free_stack(*output_stack, allocated_size);
        return -1;
    }

    // Traverse the path string
    int i = 0;
    while (i < path_len) {
        // Handle multiple slashes by skipping additional slashes
        if (path[i] == '/') {
            while (i < path_len && path[i] == '/') {
                i++;
            }
        }
            // Handle "." (current directory) by ignoring it
        else if (path[i] == '.' && (i + 1 == path_len || path[i + 1] == '/')) {
            i += 2;
        }
            // Handle ".." (parent directory) by popping from the stack
        else if (path[i] == '.' && i + 1 < path_len && path[i + 1] == '.' &&
                 (i + 2 == path_len || path[i + 2] == '/')) {
            i += 3;
            if (stackIndex > 0) {
                stackIndex--;
            } else {
                free_stack(*output_stack, allocated_size);
                free(temp);
                return -2;
            }
        }
            // A character is found
        else {
            // Create string by iterating over the characters until the next '/'
            int j = 0;
            while (i < path_len && path[i] != '/') {
                temp[j] = path[i];
                i++;
                j++;
            }

            // Null-terminate the string
            temp[j] = '\0';

            //Copy the directory to the stack
            strcpy((*output_stack)[stackIndex], temp);

            // Update the stack index
            stackIndex++;
        }
    }

    // If the stack is not full, add a null terminator
    fiffnnull((*output_stack)[stackIndex]);
    (*output_stack)[stackIndex] = NULL;
    free(temp);
    return 0;
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

    // simplify the path
    char **simplified_stack = NULL;
    int status = get_simplified_path(path, &simplified_stack);
    unsigned long allocated_size = strlen(path) + 1;
    if (status == -1) {
        return -3;
    } else if (status == -2) {
        return -1;
    }

    // Check length of the simplified stack
    int stack_length = 0;
    while (simplified_stack[stack_length] != NULL) {
        stack_length++;
    }

    // Check if the level is valid
    if (level >= stack_length) {
        free_stack(simplified_stack, allocated_size);
        return -2;
    }


    // Copy the string to the output
    char *output_string = simplified_stack[level];
    *output = (char *) malloc((strlen(output_string) + 1) * sizeof(char));
    if (*output == NULL) {
        free_stack(simplified_stack, allocated_size);
        return -3;
    }
    strcpy(*output, output_string);

    // convert the output string to uppercase
    // https://www.tutorialspoint.com/convert-a-string-to-uppercase-in-c
    for (int i = 0; (*output)[i] != '\0'; i++) {
        if ((*output)[i] >= 'a' && (*output)[i] <= 'z') {
            (*output)[i] = (*output)[i] - 32;
        }
    }

    free_stack(simplified_stack, allocated_size);
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
    long root_lba =
            cluster_to_lba(block, root_cluster, get_first_data_sector(block)) * as_uint16(block->BPB_BytsPerSec);

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
        if (fread(*entry, sizeof(FAT_entry), 1, archive) != 1) {
            return GENERAL_ERR; // error reading from the FAT
        }

        if (file_has_name(*entry, name)) {
            depth++;
            free(name);
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
                root_cluster = (as_uint16((*entry)->DIR_FstClusHI) << 16) + as_uint16((*entry)->DIR_FstClusLO);
                root_lba = cluster_to_lba(block, root_cluster, get_first_data_sector(block)) *
                           as_uint16(block->BPB_BytsPerSec);
            } else {
                if (name != NULL) {
                    free(name);
                }
                // the entry is a file but the path has not been fully traversed
                return GENERAL_ERR;
            }
        } else {
            i++;
            // check if the entry is the last entry in the root directory
            if ((*entry)->DIR_Name[0] == 0x00) {
                free(name);
                return GENERAL_ERR;
            }
        }
    }

    return 0;
}

static size_t min(size_t a, size_t b) {
    return a < b ? a : b;
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
    int cluster_size = block->BPB_SecPerClus * as_uint16(block->BPB_BytsPerSec);
    uint32_t cluster_current = (as_uint16(entry->DIR_FstClusHI) << 16) + as_uint16(entry->DIR_FstClusLO);
    uint32_t first_data_sector = get_first_data_sector(block);


    uint32_t cluster_prev;
    size_t bytes_read = 0;
    bool done = false;

    while (!done) {
        if (bytes_read >= max_len) {
            return bytes_read;
        }

        // calculate the LBA of the cluster_prev
        long lba = cluster_to_lba(block, cluster_current, first_data_sector) * as_uint16(block->BPB_BytsPerSec);

        // read cluster_prev into buffer
        if (fseek(archive, lba, SEEK_SET) != 0) { // fseek returns 0 iff the seek was successful
            return GENERAL_ERR; // error positioning within the FAT file
        }

        // read the cluster_prev at the specified offset
        size_t to_read = min(cluster_size, max_len - bytes_read);
        if (fread(buff + bytes_read, to_read, 1, archive) != 1) {
            return GENERAL_ERR; // error reading from the FAT
        }

        // update the number of bytes read
        bytes_read += to_read;

        // find next cluster_prev to read
        cluster_prev = cluster_current;
        get_cluster_chain_value(block, cluster_prev, &cluster_current, archive);

        if (cluster_current >= 0xFFFFFF8) { // end of file has been reached
            done = true;
        }
    }

    return bytes_read;
}

// ༽つ۞﹏۞༼つ

int main() {
    // ====================================================================================================

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

    // ====================================================================================================

    char *path = "first/second/third/fourth";
    char *read = NULL;
    break_up_path(path, 0, &read);
    printf("Path 1: %d, %s\n", strcasecmp(read, "FIRST") == 0, read);
    fiffnnull(read);

    read = NULL;
    break_up_path(path, 1, &read);
    printf("Path 3: %d, %s\n", strcasecmp(read, "SECOND") == 0, read);
    fiffnnull(read);

    read = NULL;
    break_up_path(path, 2, &read);
    printf("Path 4: %d, %s\n", strcasecmp(read, "THIRD") == 0, read);
    fiffnnull(read);

    read = NULL;
    break_up_path(path, 3, &read);
    printf("Path 5: %d, %s\n", strcasecmp(read, "FOURTH") == 0, read);
    fiffnnull(read);

    read = NULL;
    path = "first/../second/third/fourth/";
    break_up_path(path, 0, &read);
    printf("Path 6: %d, %s\n", strcasecmp(read, "SECOND") == 0, read);
    fiffnnull(read);

    // ====================================================================================================

    FILE *archive = fopen("../floppy.img", "rb");
    BPB *bpb = NULL;
    read_boot_block(archive, &bpb);

    FAT_entry *e = NULL;
    printf("File Descriptor 1: %d\n", find_file_descriptor(archive, bpb, "notexist", &e) < 0);
    fiffnnull(e);

    e = NULL;
    printf("File Descriptor 2: %d\n", find_file_descriptor(archive, bpb, "zola.txt", &e) >= 0);
    fiffnnull(e);

    e = NULL;
    printf("File Descriptor 3: %d\n", find_file_descriptor(archive, bpb, "afolder/another/candide.txt", &e) >= 0);
    fiffnnull(e);

    e = NULL;
    printf("File Descriptor 4: %d\n", find_file_descriptor(archive, bpb, "afolder/los.txt", &e) < 0);
    fiffnnull(e);

    e = NULL;
    printf("File Descriptor 5: %d\n", find_file_descriptor(archive, bpb, "afolder/spansih/titan.txt", &e) < 0);
    fiffnnull(e);
    fiffnnull(bpb);

    // ====================================================================================================

    bpb = NULL;
    read_boot_block(archive, &bpb);

    e = NULL;
    char *hello = "Bonne chance pour le TP4!\n";
    char *content_read = (char *) malloc(sizeof(char) * 1001);
    memset(content_read, '\0', 1001);
    printf("Read 1a: %d\n", find_file_descriptor(archive, bpb, "hello.txt", &e) >= 0);
    int bytes_read = read_file(archive, bpb, e, content_read, 1000);
    printf("Read 1b: %d\n", 0 == strcasecmp(hello, content_read));
    fiffnnull(e);

    e = NULL;
    char *zola = "The Project Gutenberg eBook, Zola, by Émile Faguet\n\n\nThis eBook is for the use of anyone anywhere at no cost and with\nalmost no restrictions whatsoever.  You may copy it, give it away or\nre-use it under the terms of the Project Gutenberg License included\nwith this eBook or online at www.gutenberg.org\n\n\n\n\n\nTitle: Zola\n\n\nAuthor: Émile Faguet\n\n\n\nRelease Date: June 5, 2008  [eBook #25704]\n\nLanguage: French\n\n\n***START OF THE PROJECT GUTENBERG EBOOK ZOLA***\n\n\nE-text prepared by Gerard Arthus, Rénald Lévesque, and the Project\nGutenberg Online Distributed Proofreading Team (http://www.pgdp.net)\n\n\n\nZOLA\n\nPar\n\nEMILE FAGUET\nde l'Académie Française\nProfesseur à la Sorbonne\n\n\n\n\n\n\nPrix: 10¢\n\n\n\n\nÉmile Zola\n\n\nJe ne m'occuperai ici, strictement, que de l'oeuvre littéraire de\nl'écrivain célèbre qui vient de mourir.\n\nÉmile Zola a eu une carrière littéraire de quarante années environ, ses\ndébuts remontant à 1863 et sa fin tragique et prématurée étant\nsurvenue,--alors qu'il écrivait ";
    memset(content_read, '\0', 1001);
    printf("Read 2a: %d\n", find_file_descriptor(archive, bpb, "zola.txt", &e) >= 0);
    bytes_read = read_file(archive, bpb, e, content_read, 1000);
    printf("Read 2b: %d\n", 0 == strcasecmp(zola, content_read));
    fiffnnull(e);

    e = NULL;
    char *los = "The Project Gutenberg EBook of Los exploradores españoles del siglo XVI, by \nCharles F. Lummis\n\nThis eBook is for the use of anyone anywhere at no cost and with\nalmost no restrictions whatsoever.  You may copy it, give it away or\nre-use it under the terms of the Project Gutenberg License included\nwith this eBook or online at www.gutenberg.org/license\n\n\nTitle: Los exploradores españoles del siglo XVI\n\nAuthor: Charles F. Lummis\n\nTranslator: Arturo Cuyás\n\nRelease Date: April 2, 2020 [EBook #61739]\n\nLanguage: Spanish\n\n\n*** START OF THIS PROJECT GUTENBERG EBOOK LOS EXPLORADORES ESPAÑOLES ***\n\n\n\n\nProduced by Adrian Mastronardi and the Online Distributed\nProofreading Team at https://www.pgdp.net (This file was\nproduced from images generously made available by The\nInternet Archive/American Libraries.)\n\n\n\n\n\n\n[Illustration: CHARLES F. LUMMIS]\n\n  Los\n  Exploradores españoles\n  del Siglo XVI\n\n  VINDICACIÓN DE LA ACCIÓN COLONIZADORA\n  ESPAÑOLA EN AMÉRICA\n\n  OBRA ESCRITA EN INGLÉS POR\n\n  C";
    memset(content_read, '\0', 1001);
    printf("Read 3a: %d\n", find_file_descriptor(archive, bpb, "spanish/los.txt", &e) >= 0);
    bytes_read = read_file(archive, bpb, e, content_read, 1000);
    printf("Read 3b: %d\n", 0 == strcasecmp(los, content_read));
    fiffnnull(e);

    fiffnnull(content_read);
    fiffnnull(bpb);
    fclose(archive);

}

// ༽つ۞﹏۞༼つ