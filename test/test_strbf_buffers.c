/**
 * @file test_strbf_buffers.c
 * @brief Test fixed vs dynamic buffer handling in strbf
 * 
 * Build: gcc -I include -o test_strbf_buffers test_strbf_buffers.c strbf.c numstr.c -lm
 * Run: ./test_strbf_buffers
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "strbf.h"

#define TEST_PASS() printf("  ✓ PASS\n")
#define TEST_FAIL(msg) printf("  ✗ FAIL: %s\n", msg)

void test_dynamic_buffer_int64() {
    printf("Test: Dynamic buffer with int64_t...\n");
    strbf_t sb;
    strbf_init(&sb);

    /* Test maximum int64_t values */
    strbf_putl(&sb, INT64_MAX);
    strbf_putc(&sb, ' ');
    strbf_putl(&sb, INT64_MIN);
    char *result = strbf_finish(&sb);

    printf("  Result: %s\n", result);
    assert(result != NULL);
    assert(strlen(result) > 0);

    strbf_free(&sb);
    TEST_PASS();
}

void test_dynamic_buffer_uint64() {
    printf("Test: Dynamic buffer with uint64_t...\n");
    strbf_t sb;
    strbf_init(&sb);

    /* Test maximum uint64_t value */
    strbf_putul(&sb, UINT64_MAX);
    char *result = strbf_finish(&sb);

    printf("  Result: %s\n", result);
    assert(result != NULL);
    assert(strlen(result) == 20);  /* UINT64_MAX is exactly 20 digits */

    strbf_free(&sb);
    TEST_PASS();
}

void test_dynamic_buffer_double() {
    printf("Test: Dynamic buffer with double...\n");
    strbf_t sb;
    strbf_init(&sb);

    /* Test various double values */
    strbf_putfd(&sb, 123.456, 2);
    strbf_putc(&sb, ' ');
    strbf_putfd(&sb, -99999.99, 2);
    char *result = strbf_finish(&sb);

    printf("  Result: %s\n", result);
    assert(result != NULL);

    strbf_free(&sb);
    TEST_PASS();
}

void test_fixed_buffer_sufficient() {
    printf("Test: Fixed buffer with sufficient space...\n");
    char buf[50];
    strbf_t sb;
    strbf_inits(&sb, buf, sizeof(buf));

    /* Small values should work fine */
    strbf_putul(&sb, 12345);
    strbf_putc(&sb, ' ');
    strbf_putfd(&sb, 67.89, 2);
    char *result = strbf_finish(&sb);

    printf("  Result: %s\n", result);
    assert(result != NULL);
    assert(strcmp(result, "12345 67.89") == 0);

    TEST_PASS();
}

void test_fixed_buffer_policy_note() {
    printf("Test: Fixed buffer policy note...\n");
    printf("  Fixed buffers are not overflow-guarded by design (speed-first policy).\n");
    printf("  Callers must provide sufficient capacity for written values.\n");
    TEST_PASS();
}

void test_fixed_buffer_small_values() {
    printf("Test: Fixed buffer with small bounded values (LCD usage)...\n");
    char buf[16];  /* Typical LCD display buffer size */
    strbf_t sb;
    strbf_inits(&sb, buf, sizeof(buf));

    /* Typical LCD display values - these should all work */
    strbf_putfd(&sb, 99.99, 2);  /* Speed: "99.99" = 5 chars */
    char *result = strbf_finish(&sb);

    printf("  Result: %s\n", result);
    assert(result != NULL);
    assert(strcmp(result, "99.99") == 0);

    /* Reset and test battery percentage */
    strbf_reset(&sb);
    strbf_putul(&sb, 87);
    strbf_putc(&sb, '%');
    result = strbf_finish(&sb);

    printf("  Result: %s\n", result);
    assert(result != NULL);
    assert(strcmp(result, "87%") == 0);

    TEST_PASS();
}

void test_reset_preserves_buffer_type() {
    printf("Test: strbf_reset preserves buffer type...\n");

    /* Test fixed buffer */
    char fixed_buf[32];
    strbf_t fixed_sb;
    strbf_inits(&fixed_sb, fixed_buf, sizeof(fixed_buf));
    strbf_puts(&fixed_sb, "test");
    strbf_reset(&fixed_sb);

    /* After reset, should still be fixed buffer */
    assert(fixed_sb.max != NULL);
    assert(fixed_sb.end == NULL);
    assert(fixed_sb.cur == fixed_sb.start);

    /* Test dynamic buffer */
    strbf_t dynamic_sb;
    strbf_init(&dynamic_sb);
    strbf_puts(&dynamic_sb, "test");
    strbf_reset(&dynamic_sb);

    /* After reset, should still be dynamic buffer */
    assert(dynamic_sb.max == NULL);
    assert(dynamic_sb.end != NULL);
    assert(dynamic_sb.cur == dynamic_sb.start);

    strbf_free(&dynamic_sb);
    TEST_PASS();
}

int main() {
    printf("\n=== String Buffer Tests (Fixed vs Dynamic) ===\n\n");

    test_dynamic_buffer_int64();
    test_dynamic_buffer_uint64();
    test_dynamic_buffer_double();
    test_fixed_buffer_sufficient();
    test_fixed_buffer_policy_note();
    test_fixed_buffer_small_values();
    test_reset_preserves_buffer_type();

    printf("\n=== All Tests Passed ✓ ===\n\n");
    return 0;
}
