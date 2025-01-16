#include <stdlib.h>
#include <limits.h>
#include <check.h>
#include "../src/main.h"
#include "./check_utils.h"
#include "./call_by_string.h"

#define fiffnnull(e) do {if(NULL != (e)) free(e);} while(0)
#define COERCE_STATIC_STR(str) ({char *line = malloc(sizeof(char)*(strlen(str)+1)); for(int i=0; i<=strlen(str); i++) line[i] = str[i]; line;})

static inline int strcmpi(const char *str1, const char *str2) {
    size_t num = 99999;
    int ret_code = INT_MIN;

    size_t chars_compared = 0;

    // Check for NULL pointers
    if (!str1 || !str2) {
        goto done;
    }

    // Continue doing case-insensitive comparisons, one-character-at-a-time, of str1 to str2,
    // as long as at least one of the strings still has more characters in it, and we have
    // not yet compared num chars.
    while ((*str1 || *str2) && (chars_compared < num)) {
        ret_code = tolower((int) (*str1)) - tolower((int) (*str2));
        if (ret_code != 0) {
            // The 2 chars just compared don't match
            break;
        }
        chars_compared++;
        str1++;
        str2++;
    }

    done:
    return ret_code;
}

uint32_t __ctlba(BPB *block, uint32_t cluster, uint32_t first_data_sector) {
  return cluster_to_lba(block, cluster, first_data_sector);
}

error_code __rbb(FILE *archive, BPB **block) {
  return read_boot_block(archive, block);
}

char *ARCHIVE_PATH = "floppy.img";

FILE *open_archive() {
    return fopen(ARCHIVE_PATH, "r");
}

#define STD_TEST() FILE*archive=open_archive();BPB*bpb=NULL;__rbb(archive, &bpb);fseek(archive,0,SEEK_SET);
#define STD_TEST_END() fclose(archive);free(bpb);

DEFINE_TEST(test_cluster_to_lba_1) {
        STD_TEST();

        for (uint8_t i = 2; i < 10; ++i) {
            ck_assert_uint_eq(__ctlba(bpb, i, 1600), cluster_to_lba(bpb, i, 1600));
        }

        fclose(archive);
}

END_TEST

DEFINE_TEST(test_next_cluster_1) {
        STD_TEST();
        uint32_t next;
        get_cluster_chain_value(bpb, 2, &next, archive);
        ck_assert_uint_eq(1, (FAT_EOC_TAG & next) > 0);
        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_check_name_1) {
        STD_TEST();

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

        ck_assert(file_has_name(&entry, COERCE_STATIC_STR("ANAME")));
        ck_assert(!file_has_name(&entry, COERCE_STATIC_STR("NAME")));
        ck_assert(!file_has_name(&entry, COERCE_STATIC_STR("ANAM")));

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_check_name_2) {
        STD_TEST();

        FAT_entry entry;

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

        ck_assert(!file_has_name(&entry, COERCE_STATIC_STR("ANAME")));
        ck_assert(file_has_name(&entry, COERCE_STATIC_STR("ANAME.AAA")));

        STD_TEST_END();
}

END_TEST


DEFINE_TEST(test_path_1) {
        STD_TEST()

        char *path = COERCE_STATIC_STR("first/second/third/fourth");
        char *read = NULL;

        if (break_up_path(path, 0, &read) < 0 || NULL == read) {
            ck_abort();
        } else {
            ck_assert_str_eq(read, COERCE_STATIC_STR("FIRST"));
        }

        free(read);

        STD_TEST_END()
}

END_TEST

DEFINE_TEST(test_path_2) {
        STD_TEST()

        char *path = COERCE_STATIC_STR("first/second/third/fourth");
        char *read = NULL;

        if (break_up_path(path, 1, &read) < 0 || NULL == read) {
            ck_abort();
        } else {
            ck_assert_str_eq(read, COERCE_STATIC_STR("SECOND"));
        }

        free(read);
        STD_TEST_END()
}

END_TEST

DEFINE_TEST(test_path_3) {
        STD_TEST()

        char *path = COERCE_STATIC_STR("first/second/third/fourth");
        char *read = NULL;

        if (break_up_path(path, 2, &read) < 0 || NULL == read) {
            ck_abort();
        } else {
            ck_assert_str_eq(read, COERCE_STATIC_STR("THIRD"));
        }

        free(read);
        STD_TEST_END()
}

END_TEST

DEFINE_TEST(test_path_4) {
        STD_TEST()

        char *path = COERCE_STATIC_STR("first/second/third/fourth");
        char *read = NULL;

        if (break_up_path(path, 3, &read) < 0 || NULL == read) {
            ck_abort();
        } else {
            ck_assert_str_eq(read, COERCE_STATIC_STR("FOURTH"));
        }

        free(read);

        STD_TEST_END()
}

END_TEST

DEFINE_TEST(test_read_boot_block_1) {
        STD_TEST();
        BPB *sbpb = NULL;

        if (read_boot_block(fopen(ARCHIVE_PATH, "r"), &sbpb) < 0 || NULL == sbpb) {
            ck_abort();
        } else {
            ck_assert_uint_eq(as_uint16(bpb->BPB_BytsPerSec), as_uint16(sbpb->BPB_BytsPerSec));
            ck_assert_uint_eq(as_uint16(bpb->BPB_RsvdSecCnt), as_uint16(sbpb->BPB_RsvdSecCnt));
            ck_assert_uint_eq(as_uint16(bpb->BPB_RootEntCnt), as_uint16(sbpb->BPB_RootEntCnt));
            ck_assert_uint_eq(as_uint16(bpb->BPB_TotSec16), as_uint16(sbpb->BPB_TotSec16));
            ck_assert_uint_eq(as_uint16(bpb->BPB_FATSz16), as_uint16(sbpb->BPB_FATSz16));
            ck_assert_uint_eq(as_uint16(bpb->BPB_FSVer), as_uint16(sbpb->BPB_FSVer));
            ck_assert_uint_eq(as_uint16(bpb->BS_VolLab), as_uint16(sbpb->BS_VolLab));
        }

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_find_file_descriptor_1) {
        STD_TEST();

        FAT_entry *e = NULL;
        ck_assert_int_eq(find_file_descriptor(archive, bpb, COERCE_STATIC_STR("notexist"), &e) < 0, 1);
        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_find_file_descriptor_2) {
        STD_TEST();

        FAT_entry *e = NULL;
        ck_assert_int_eq(find_file_descriptor(archive, bpb, COERCE_STATIC_STR("zola.txt"), &e) >= 0, 1);
        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_find_file_descriptor_3) {
        STD_TEST();

        FAT_entry *e = NULL;

        ck_assert_int_eq(find_file_descriptor(archive, bpb, COERCE_STATIC_STR(
                "afolder/another/candide.txt"), &e) >= 0, 1);

        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_find_file_descriptor_4) {
        STD_TEST();

        FAT_entry *e = NULL;

        ck_assert_int_eq(find_file_descriptor(archive, bpb, COERCE_STATIC_STR("afolder/los.txt"), &e) < 0, 1);

        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_find_file_descriptor_5) {
        STD_TEST();

        FAT_entry *e = NULL;

        ck_assert_int_eq(find_file_descriptor(archive, bpb, COERCE_STATIC_STR("afolder/spansih/titan.txt"), &e) < 0, 1);

        fiffnnull(e);

        STD_TEST_END();
}

END_TEST


DEFINE_TEST(test_read_content_1) {
        STD_TEST();

        FAT_entry *e = NULL;
        char *hello = COERCE_STATIC_STR("Bonne chance pour le TP4!\n");
        char *content_read = (char *) malloc(sizeof(char) * 1001);

        memset(content_read, '\0', 1001);

        if (find_file_descriptor(archive, bpb, COERCE_STATIC_STR(
                "hello.txt"), &e) < 0 || NULL == e || NULL == content_read) {
            ck_abort();
        } else {
            read_file(archive, bpb, e, content_read, 1000);
            ck_assert_int_eq(0, strcmpi(hello, content_read));
        }

        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_read_content_2) {
        STD_TEST();

        FAT_entry *e = NULL;

        char *zola = COERCE_STATIC_STR(
                "The Project Gutenberg eBook, Zola, by Émile Faguet\n\n\nThis eBook is for the use of anyone anywhere at no cost and with\nalmost no restrictions whatsoever.  You may copy it, give it away or\nre-use it under the terms of the Project Gutenberg License included\nwith this eBook or online at www.gutenberg.org\n\n\n\n\n\nTitle: Zola\n\n\nAuthor: Émile Faguet\n\n\n\nRelease Date: June 5, 2008  [eBook #25704]\n\nLanguage: French\n\n\n***START OF THE PROJECT GUTENBERG EBOOK ZOLA***\n\n\nE-text prepared by Gerard Arthus, Rénald Lévesque, and the Project\nGutenberg Online Distributed Proofreading Team (http://www.pgdp.net)\n\n\n\nZOLA\n\nPar\n\nEMILE FAGUET\nde l'Académie Française\nProfesseur à la Sorbonne\n\n\n\n\n\n\nPrix: 10¢\n\n\n\n\nÉmile Zola\n\n\nJe ne m'occuperai ici, strictement, que de l'oeuvre littéraire de\nl'écrivain célèbre qui vient de mourir.\n\nÉmile Zola a eu une carrière littéraire de quarante années environ, ses\ndébuts remontant à 1863 et sa fin tragique et prématurée étant\nsurvenue,--alors qu'il écrivait ");

        char *content_read = (char *) malloc(sizeof(char) * 1001);

        memset(content_read, '\0', 1001);

        if (find_file_descriptor(archive, bpb, COERCE_STATIC_STR("zola.txt"), &e) < 0 || NULL == e ||
        NULL == content_read) {
            ck_abort();
        } else {
            read_file(archive, bpb, e, content_read, 1000);
            ck_assert_int_eq(0, strcmpi(zola, content_read));
        }

        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

DEFINE_TEST(test_read_content_3) {
        STD_TEST();

        FAT_entry *e = NULL;

        char *los = COERCE_STATIC_STR(
                "The Project Gutenberg EBook of Los exploradores españoles del siglo XVI, by \nCharles F. Lummis\n\nThis eBook is for the use of anyone anywhere at no cost and with\nalmost no restrictions whatsoever.  You may copy it, give it away or\nre-use it under the terms of the Project Gutenberg License included\nwith this eBook or online at www.gutenberg.org/license\n\n\nTitle: Los exploradores españoles del siglo XVI\n\nAuthor: Charles F. Lummis\n\nTranslator: Arturo Cuyás\n\nRelease Date: April 2, 2020 [EBook #61739]\n\nLanguage: Spanish\n\n\n*** START OF THIS PROJECT GUTENBERG EBOOK LOS EXPLORADORES ESPAÑOLES ***\n\n\n\n\nProduced by Adrian Mastronardi and the Online Distributed\nProofreading Team at https://www.pgdp.net (This file was\nproduced from images generously made available by The\nInternet Archive/American Libraries.)\n\n\n\n\n\n\n[Illustration: CHARLES F. LUMMIS]\n\n  Los\n  Exploradores españoles\n  del Siglo XVI\n\n  VINDICACIÓN DE LA ACCIÓN COLONIZADORA\n  ESPAÑOLA EN AMÉRICA\n\n  OBRA ESCRITA EN INGLÉS POR\n\n  C");

        char *content_read = (char *) malloc(sizeof(char) * 1001);

        memset(content_read, '\0', 1001);

        if (find_file_descriptor(archive, bpb, COERCE_STATIC_STR("spanish/los.txt"), &e) < 0 || NULL == e ||
        NULL == content_read) {
            ck_abort();
        } else {
            read_file(archive, bpb, e, content_read, 1000);
            ck_assert_int_eq(0, strcmpi(los, content_read));
        }

        fiffnnull(e);

        STD_TEST_END();
}

END_TEST

int main(int argc, char *argv[]) {
    call_by_string(argv[1]);
    exit(-1);
}
