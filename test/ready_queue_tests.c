
#include <check.h>
#include <pthread.h>
#include <unistd.h>

#include "ready_queue.h"

START_TEST(test_ready_queue_init_zero_size)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    size_t size = ready_queue_size(&queue);

    ck_assert_int_eq(size, 0);

    ready_queue_destroy(&queue);
}
END_TEST

START_TEST(test_ready_queue_push_size_one)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    struct process* process = create_process(0);
    ready_queue_push(&queue, process);

    size_t size = ready_queue_size(&queue);

    ck_assert_int_eq(size, 1);

    ready_queue_destroy(&queue);
}
END_TEST

START_TEST(test_ready_queue_push_size_two)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    struct process* process1 = create_process(0);
    struct process* process2 = create_process(1);
    ready_queue_push(&queue, process1);
    ready_queue_push(&queue, process2);

    size_t size = ready_queue_size(&queue);

    ck_assert_int_eq(size, 2);

    ready_queue_destroy(&queue);
}
END_TEST

START_TEST(test_ready_queue_push_pop_size_zero)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    struct process* process = create_process(0);

    ready_queue_push(&queue, process);
    ready_queue_pop(&queue);

    size_t size = ready_queue_size(&queue);

    ck_assert_int_eq(size, 0);

    ready_queue_destroy(&queue);
}
END_TEST

START_TEST(test_ready_queue_push_push_pop_size_one)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    struct process* process = create_process(0);

    ready_queue_push(&queue, process);
    ready_queue_push(&queue, process);
    ready_queue_pop(&queue);

    size_t size = ready_queue_size(&queue);

    ck_assert_int_eq(size, 1);

    ready_queue_destroy(&queue);
}
END_TEST


START_TEST(test_ready_queue_push_pop_multiple) {
    struct ready_queue queue;
    ready_queue_init(&queue);

    struct process* process[100];

    for (int i = 0; i < 100; i++) {
        process[i] = create_process(i);
        ready_queue_push(&queue, process[i]);
    }

    int found[100] = {0};

    for (int i = 0; i < 100; i++) {
        process_t *p = ready_queue_pop(&queue);
        ck_assert_ptr_ne(p, NULL);
        ck_assert_int_ge(p->pid, 0);
        ck_assert_int_lt(p->pid, 100);
        found[p->pid] = 1;
    }

    for (int i = 0; i < 100; i++) {
        ck_assert_int_eq(found[i], 1);
    }
}
END_TEST

START_TEST(test_ready_queue_push_push_pop_poison_pill)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    ready_queue_push(&queue, NULL);

    process_t *process = ready_queue_pop(&queue);

    ck_assert_ptr_eq(process, NULL);
}
END_TEST

static int pop_count = 0;

void *ready_queue_pop_blocks(void *queue)
{
    ready_queue_pop((ready_queue_t *) queue);

    pop_count++;

    return NULL;
}

START_TEST(test_ready_queue_pop_blocks)
{
    struct ready_queue queue;
    ready_queue_init(&queue);

    pop_count = 0;

    pthread_t thread;
    pthread_create(&thread, NULL, ready_queue_pop_blocks, &queue);

    sleep(1);

    ck_assert_int_eq(pop_count, 0);

    ready_queue_push(&queue, NULL);

    pthread_join(thread, NULL);

    ck_assert_int_eq(pop_count, 1);

    ready_queue_destroy(&queue);
}
END_TEST

int main(void)
{
    // Print amount of tests passed and failed
    int number_failed;

    Suite *s = suite_create("ready_queue");
    TCase *tc = tcase_create("ready_queue");
    tcase_add_test(tc, test_ready_queue_init_zero_size);
    tcase_add_test(tc, test_ready_queue_push_size_one);
    tcase_add_test(tc, test_ready_queue_push_size_two);
    tcase_add_test(tc, test_ready_queue_push_pop_size_zero);
    tcase_add_test(tc, test_ready_queue_push_push_pop_size_one);
    tcase_add_test(tc, test_ready_queue_push_pop_multiple);
    tcase_add_test(tc, test_ready_queue_push_push_pop_poison_pill);
    tcase_add_test(tc, test_ready_queue_pop_blocks);
    suite_add_tcase(s, tc);

    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? 0 : 1;
}
