#include <stdlib.h>
#include <check.h>
#include "../src/main.h"
#include "./check_utils.h"
#include "./call_by_string.h"

DEFINE_TEST(test_strlen_1) {
    char *temp = "4444";
    ck_assert_int_eq(strlen2(temp), 4);
} END_TEST

DEFINE_TEST(test_strlen_2) {
    char *temp = "4444444444444444444444444444444444444444444444444444444444444444";
    ck_assert_int_eq(strlen2(temp), 64);
} END_TEST

DEFINE_TEST(test_strlen_3) {
    char *temp = "";
    ck_assert_int_eq(strlen2(temp), 0);
} END_TEST

DEFINE_TEST(test_no_of_lines_1) {
    FILE *test_file = fopen("../src/five_lines", "r");
    ck_assert_int_eq(no_of_lines(test_file), 5);
    fclose(test_file);
} END_TEST

DEFINE_TEST(test_no_of_lines_2) {
    FILE *test_file = fopen("../src/six_lines", "r");
    ck_assert_int_eq(no_of_lines(test_file), 6);
    fclose(test_file);
} END_TEST

DEFINE_TEST(test_no_of_lines_3) {
    FILE *test_file = fopen("../src/five_lines", "r");

    fgetc(test_file);
    no_of_lines(test_file);
    ck_assert_int_eq(fgetc(test_file), 'i');
    fclose(test_file);
} END_TEST

DEFINE_TEST(test_no_of_lines_4) {
    FILE *test_file = fopen("../src/empty", "r");
    ck_assert_int_eq(no_of_lines(test_file), 0);
    fclose(test_file);
} END_TEST

DEFINE_TEST(test_readline_1) {
    char **read = malloc(sizeof(char *));

    FILE *test_file = fopen("../src/five_lines", "r");
    readline(test_file, read, 1024);

    char *line = *read;

    ck_assert_str_eq(line, "line one");
    free(*read);
    free(read);
    fclose(test_file);
} END_TEST

DEFINE_TEST(test_readline_2) {  // tests if cursor moves
    char **read = malloc(sizeof(char*));

    FILE *test_file = fopen("../src/five_lines", "r");
    readline(test_file, read, 1024);
    ck_assert_str_eq(*read, "line one");

    free(*read);
    readline(test_file, read, 1024);
    ck_assert_str_eq(*read, "line two");

    free(*read);
    free(read);
    fclose(test_file);
} END_TEST

DEFINE_TEST(test_memcpy_1) {
    byte *a = malloc(sizeof(byte) * 100);
    byte *b = malloc(sizeof(byte) * 100);

    for (int i = 0; i < 100; ++i) {
        b[i] = 0;
        a[i] = 0;
    }

    for (int i = 0; i < 10; i++) b[i] = i + 1;

    int nb_bytes = memcpy2(a, b, 10);
    ck_assert_int_eq(nb_bytes, 10);

    for(int i=0; i<10; i++) ck_assert_int_eq(a[i], b[i]);

    free(a);
    free(b);

} END_TEST

DEFINE_TEST(test_execute_1) {
    #define HAS_ERROR(code) ((code) < 0)
    ck_assert_int_eq(HAS_ERROR(execute("../this_file_dne", "101010")), 1);
} END_TEST

DEFINE_TEST(test_execute_2) {
#define HAS_ERROR(code) ((code) < 0)
    ck_assert_int_eq(execute("../src/youre_gonna_go_far_kid", ""), 1);
} END_TEST

DEFINE_TEST(test_execute_3) {
#define HAS_ERROR(code) ((code) < 0)
    ck_assert_int_eq(execute("../src/has_five_ones", "0000"), 0);
} END_TEST

DEFINE_TEST(test_execute_4) {
#define HAS_ERROR(code) ((code) < 0)
    ck_assert_int_eq(execute("../src/has_five_ones", "101010101"), 1);
} END_TEST

DEFINE_TEST(test_execute_5) {
#define HAS_ERROR(code) ((code) < 0)
    ck_assert_int_eq(execute("../src/has_five_ones", "111111111"), 1);
} END_TEST

DEFINE_TEST(test_execute_6) {
#define HAS_ERROR(code) ((code) < 0)
    ck_assert_int_eq(execute("../src/youre_gonna_go_far_kid", "STARING AT THE SUN"), -1);
} END_TEST


/**
 * Get rid of a transition struct
 * @param t the struct
 */
void destroy_transition(transition *t) {
    if(!t)
        return;
    free(t->next_state);
    free(t->current_state);
    free(t);
}

DEFINE_TEST(test_parse_line_1) {
    char *line = "(q0,0)->(qR,0,D)\n";
    transition *t = parse_line(line, strlen(line));
    ck_assert_msg(t, "The transition cannot be null");
    ck_assert_str_eq(t->current_state, "q0");
    ck_assert_str_eq(t->next_state, "qR");
    ck_assert_int_eq(t->read, '0');
    ck_assert_int_eq(t->write,'0');
    ck_assert_int_eq(t->movement, 1);
    destroy_transition(t);
} END_TEST

DEFINE_TEST(test_parse_line_2) {
    char *line = "(q0,1)->(qA,1,R)";
    transition *t = parse_line(line, strlen(line));
    ck_assert_msg(t, "The transition cannot be null");
    ck_assert_str_eq(t->current_state, "q0");
    ck_assert_str_eq(t->next_state, "qA");
    ck_assert_int_eq(t->read, '1');
    ck_assert_int_eq(t->write,'1');
    ck_assert_int_eq(t->movement, 0);
    destroy_transition(t);
} END_TEST

int main(int argc, char *argv[]) {
    if(strcmp(argv[1], "valgrind")==0) {
        execute("../src/has_five_ones", "111111111");
        execute("../src/has_five_ones", "0000");
        execute("../this_file_dne", "101010");
        exit(0);
    }

    call_by_string(argv[1]);

    exit(-1);
}
