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
 * Prend cluster et retourne son addresse en secteur dans l'archive
 * @param block le block de paramètre du BIOS
 * @param cluster le cluster à convertir en LBA
 * @param first_data_sector le premier secteur de données, donnée par la formule dans le document
 * @return le LBA
 */
uint32_t cluster_to_lba(BPB *block, uint32_t cluster, uint32_t first_data_sector) {
    uint32_t LBA = first_data_sector + (cluster - 2) * block->BPB_SecPerClus;
    return LBA;
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
    uint32_t fat_offset = cluster * 4; // each entry in the FAT table is 32 bits (4 bytes)

    // position cursor in the FAT table at the offset
    if (fseek(archive, fat_offset, SEEK_SET) != 0) { // fseek returns 0 iff the seek was successful
        return GENERAL_ERR; // error positioning within the FAT file
    }

    // read the entry in the FAT table at the specified offset
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
    char filename[9] ; // holds the filename (8 char) and the null terminator (1 char)
    strncpy(filename, entry->DIR_Name, 8);
    filename[8] = '\0'; // filename is null terminated

    char extension[4]; // holds the extension (8 char) and the null terminator (1 char)
    strncpy(extension, entry->DIR_Name + 8, 3);
    extension[3] = '\0';

    char full_name[13]; // filename => 8 char; "." => 1 char; extension => 3 char; null terminator => 1 char; 8 + 1 + 3 + 1 = 13
    snprintf(full_name, sizeof(full_name), "%s.%s", filename, extension);

    // strcasecmp returns 0 iff both strings are the same. strcasecmp is not case-sensitive
    return strcasecmp(full_name, name) == 0;
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
    if (path == NULL || output == NULL || level < 0) {
        return -1; // invalid arguments
    }

    int part_count;
    if (strlen(path) == 0) {
        part_count = 0; // empty string
    } else part_count = 1; // string isn't empty => part_count is at least 1

    // find the position of the beginning of the part of the path
    char *ptr = path;
    if (*ptr == '/') {
        ptr++; // case where the path starts with '/'. Must not be counted.
    }
    while (*ptr != '\0' && part_count < level) {
        if (*ptr == '/') {
            while (*ptr == '/') { // skip consecutive '/'
                ptr++;
            }
            if (*ptr != '\0') {
                part_count++;
            }
        }
        ptr++;
    }

    // check whether the level is within bounds
    if (level >= part_count) {
        return -2; // not enough levels in the path
    }

    // ptr points to the part of the string we are looking
    // let's determine the size of the string
    char *start = ptr; // pointer to the start the string
    int length = 1;
    while (*ptr != '\0' && *ptr != '/') {
        length++;
        ptr++;
    }

    // allocate memory
    *output = (char *)malloc(length + 1);
    if (*output == NULL) {
        return -3; // memory allocation error
    }
    // copy the substring into the output string
    memcpy(*output, start, length);
    (*output)[length] = '\0'; // add null terminator

    return length + 1; // include the null character
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
    return 0;
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
error_code
read_file(FILE *archive, BPB *block, FAT_entry *entry, void *buff, size_t max_len) {
    return 0;
}

// ༽つ۞﹏۞༼つ

int main() {
// ous pouvez ajouter des tests pour les fonctions ici

    return 0;
}

// ༽つ۞﹏۞༼つ