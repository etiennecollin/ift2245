#include <stdlib.h>
#include <check.h>
#include "../src/vmm.h"
#include "../src/pt.h"
#include "../src/tlb.h"
#include "../src/pm.h"
#include "../src/common.h"
#include "./check_utils.h"
#include "./call_by_string.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//assumes bs is 00011112223344556677889900112233...

DEFINE_TEST(test_pm_1) {
        //printf("Wat");
        //printf("Wat %c", pm_read(10));
    ck_assert_int_eq(pm_read(0), NULL);

    pm_download_page(0, 2);

    ck_assert_int_eq(pm_read(2*256-1), NULL);
    ck_assert_int_eq(pm_read(2*256), '0');

    ck_assert_int_eq(pm_read(3*256-1), '0');
    ck_assert_int_eq(pm_read(3*256), NULL);
} END_TEST

DEFINE_TEST(test_pm_2) {
    pm_download_page(0, 3);

    ck_assert_int_eq(pm_read(3*256-1), NULL);
    ck_assert_int_eq(pm_read(3*256), '0');

    ck_assert_int_eq(pm_read(4*256-1), '0');
    ck_assert_int_eq(pm_read(4*256), NULL);
} END_TEST

DEFINE_TEST(test_pm_3) {
    pm_download_page(0, 3);

    ck_assert_int_eq(pm_read(3*256+2), '0');
    pm_write(3*256+2, 'n');
    ck_assert_int_eq(pm_read(3*256+2), 'n');
} END_TEST

DEFINE_TEST(test_pm_4) {
    ck_assert_int_eq(pm_read(3*256+2), '\0');
    pm_write(3*256+2, 'n');
    ck_assert_int_eq(pm_read(3*256+2), 'n');
} END_TEST

DEFINE_TEST(test_pm_5) {
        pm_download_page(0, 3);

        ck_assert_int_eq(pm_read(3*256+2), '0');
        pm_write(3*256+2, 'n');
        ck_assert_int_eq(pm_read(3*256+2), 'n');

        pm_download_page(1, 3);

        ck_assert_int_eq(pm_read(3*256+2), '1');
        pm_write(3*256+2, 'n');
        ck_assert_int_eq(pm_read(3*256+2), 'n');
} END_TEST

DEFINE_TEST(test_pm_6) {
        pm_download_page(0, 3);

        ck_assert_int_eq(pm_read(3*256+2), '0');
        pm_write(3*256+2, 'n');
        ck_assert_int_eq(pm_read(3*256+2), 'n');

        pm_download_page(0, 3);
        ck_assert_int_eq(pm_read(3*256+2), '0');
} END_TEST

DEFINE_TEST(test_pm_7) {
        pm_download_page(0, 3);

        ck_assert_int_eq(pm_read(3*256+2), '0');
        pm_write(3*256+2, 'n');
        ck_assert_int_eq(pm_read(3*256+2), 'n');

        pm_backup_page(3, 0);
        pm_download_page(0, 3);
        ck_assert_int_eq(pm_read(3*256+2), 'n');
} END_TEST


DEFINE_TEST(test_pt_1) {
    ck_assert_int_eq(pt_lookup(0) < 0, 1);
} END_TEST

DEFINE_TEST(test_pt_2) {
    pt_set_entry(0, 20);
    ck_assert_int_eq(pt_lookup(0), 20);
} END_TEST

DEFINE_TEST(test_pt_3) {
    pt_set_entry(0, 20);
    ck_assert_int_eq(pt_lookup(0), 20);
    pt_unset_entry(0);
    ck_assert_int_eq(pt_lookup(0) < 0, 1);
} END_TEST

DEFINE_TEST(test_pt_4) {
        pt_set_entry(0, 20);
        ck_assert_int_eq(pt_readonly_p(0), 0);
} END_TEST

DEFINE_TEST(test_pt_5) {
        pt_set_entry(0, 20);
        ck_assert_int_eq(pt_readonly_p(0), 0);
        pt_set_readonly(0, 0);
        ck_assert_int_eq(pt_readonly_p(0), 0);
        pt_set_readonly(0, 1);
        ck_assert_int_eq(pt_readonly_p(0), 1);
} END_TEST

DEFINE_TEST(test_tlb_1) {
    tlb_add_entry(2, 3, 1);
    ck_assert_int_eq(tlb_lookup(2, 1), 3);
} END_TEST

DEFINE_TEST(test_tlb_2) {
    ck_assert_int_eq(tlb_lookup(2, 1) < 0, 1);
} END_TEST

DEFINE_TEST(test_tlb_3) {
    pt_set_entry(0, 20);
    ck_assert_int_eq(pt_lookup(0), 20);
    pt_unset_entry(0);
    ck_assert_int_eq(pt_lookup(0) < 0, 1);
} END_TEST

int main(int argc, char *argv[]) {
    FILE *pm_log = NULL;
    FILE *tlb_log = NULL;
    FILE *pt_log = NULL;
    FILE *vmm_log = NULL;
    FILE *backstore = NULL;

    backstore = fopen ("bs.txt", "r+");

  /* Initialisation.  */
  tlb_init (tlb_log);
  pt_init (pt_log);
  pm_init (backstore, pm_log);
  vmm_init (vmm_log);

    //printf("Wat");
//printf("Wat %c", pm_read(10));
    //ck_assert_int_eq(pm_read(10), NULL);

    call_by_string(argv[1]);
    exit(-1);
}