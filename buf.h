#pragma once

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#define BUF_COUNT(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
#define BUF_MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    int len;
    int cap;
    char buf[];
} BufHdr;

// clang-format off
#define buf__raw(b) ((int *)(b)-2)
#define buf__len(b) buf__raw(b)[0]
#define buf__cap(b) buf__raw(b)[1]

int buf_len(const void *b);
void *buf__grow(const void *b, int len, size_t elem_size);
#define buf__fit(b, n) ((n) <= buf_cap((b)) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))

// Adds n uninitialzed new elements at the end of the buffer and returns pointer to first new element.
#define buf_append(b, n) (buf__fit(b, buf_len((b))+(n)), (b) ? buf__len(b)+=(n), (b)+buf__len(b)-(n) : (b))

// Returns a pointer to element at index i if it's within bounds.
#define buf_at(b, i) ((b) && 0 <= (i) && (i) < buf__len(b) ? (b)+(i) : NULL)

// Return the number of elements allocated for the buffer
int buf_cap(const void *b) { return (b ? buf__cap(b) : 0); }

// Sets the length of the buffer to zero.
// FIXME: memset(0) from 0 to cap?
// buf_resize ???
void buf_clear(const void *b) { if (b) buf__len(b) = 0; }

// Returns whether the buffer is empty.
bool buf_empty(const void *b) { return (b == NULL || buf__len(b) == 0); }

// Returns a pointer to 1 past the last element.
#define buf_end(b) ((b) ? (b)+buf__len(b) : NULL)

// Deallocates the buffer.
#define buf_free(b) ((b) ? (free(buf__raw(b)), (b)=NULL) : NULL)

// Returns a BufHdr pointer for the buffer.
BufHdr *buf_hdr(const void *b) { return (b ? (BufHdr *)buf__raw(b) : NULL); }

// Return a pointer to the last element in the buffer.
#define buf_last(b) ((b) ? (buf__len(b) ? (b)+buf__len(b)-1 : (b)) : NULL)

// Returns the number of elements in the buffer
int buf_len(const void *b) { return (b ? buf__len(b) : 0); }

// Adds a new element at the end of the buffer.
#define buf_push(b, v) (buf__fit(b, buf_len((b))+1), (b)[buf__len(b)++]=(v), (b)+buf__len(b)-1)

// Removes the last element in the vector, returns pointer to the removed element.
#define buf_pop(b) ((b) && buf__len(b) > 0 ? (b) + (--buf__len(b)) : NULL)

// Request that the buffer capacity be enough to contain at least n elements.
#define buf_reserve(b, n) (buf__fit(b, n))

// Returns size of the buffer in bytes
#define buf_sizeof(b) ((b) ? buf__len(b)*sizeof(*b) : 0)

//
// char *str_new(const char *s) { str t = NULL; return str_append(t, s); }
// #define buf_new(T, b, n) ()

//
#define buf_take(b, n) ((b) && 0 <= (n) && (n) < buf__len(b) ? buf__len(b)=(n) : 0, (b))

void *buf__grow(const void *b, int len, size_t elem_size)
{
    assert(buf_cap(b) <= (INT_MAX - 1)/2);
    int cap = BUF_MAX(32, BUF_MAX(2 * buf_cap(b), len));
    assert(len <= cap && cap <= (INT_MAX - (int)offsetof(BufHdr, buf))/(int)elem_size);
    size_t size = offsetof(BufHdr, buf) + elem_size * cap;
    BufHdr *hdr = (BufHdr *)realloc(b ? buf__raw(b) : NULL, size);
    hdr->cap = cap;
    if (!b) hdr->len = 0;
    return hdr->buf;
}
// clang-format on

void buf_init_test(void)
{
    int *b = NULL;
    assert(buf_at(b, 0) == NULL);
    assert(buf_cap(b) == 0);
    assert(buf_empty(b) == true);
    assert(buf_end(b) == NULL);
    assert(buf_hdr(b) == NULL);
    assert(buf_last(b) == NULL);
    assert(buf_len(b) == 0);
    assert(buf_pop(b) == NULL);
    assert(buf_sizeof(b) == 0);
}

void buf_append_test(void)
{
    // // clang-format off
    // struct t {
    //     int *init; int reserve, append;
    // } tt[] = {
    //     {},
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    //     int *b = NULL;
    //     int n_init = BUF_COUNT(t->init);
    //     if (t->init) for (int i = 0; i < n_init; ++i) buf_push(b, t->init[i]);
    //     if (t->reserve) buf_reserve(b, t->reserve);
    //     int *a = buf_append(b, t->append);
    //     assert(a == b + n_init);
    //     assert(buf_len(b) == n_init + t->append);
    //     assert(buf_end(b) == a + t->append);
    //     buf_free(b);
    // }

    int n = 4;

    int *b1 = NULL;
    int *p1 = buf_append(b1, n);
    assert(b1 == p1);
    assert(buf_len(b1) == n);
    buf_free(b1);

    int *b2 = NULL;
    buf_reserve(b2, 2 * n);
    buf_push(b2, 1);
    int *p2 = buf_append(b2, n);
    assert(b2 + 1 == p2);
    buf_free(b2);

    int *b3 = NULL;
    int *p3 = buf_append(b3, 0);
    assert(b3 == p3 && p3 == NULL);
}

void buf_at_test(void)
{
    // // clang-format off
    // struct t {
    //     int *init, indexes;
    // } tt[] = {
    //     {},
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    //     int *b = NULL;
    //     int n_init = BUF_COUNT(t->init);
    //     if (t->init) for (int i = 0; i < n_init; ++i) buf_push(b, t->init[i]);
    //     buf_free(b);
    // }
}

void buf_clear_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    buf_clear(b1);
    assert(b1 == NULL && buf_len(b1) == 0 && buf_cap(b1) == 0);
    buf_free(b1);

    int *b2 = NULL;
    buf_reserve(b2, 1);
    buf_clear(b2);
    assert(b2 != NULL && buf_len(b2) == 0 && buf_cap(b2) >= 1);
    buf_free(b2);

    int *b3 = NULL;
    buf_append(b3, 1);
    buf_clear(b3);
    assert(b3 != NULL && buf_len(b3) == 0 && buf_cap(b3) >= 1);
    buf_free(b3);
}

void buf_empty_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    assert(buf_empty(b1));

    int *b2 = NULL;
    buf_reserve(b2, 1);
    assert(buf_empty(b2));
    buf_free(b2);

    int *b3 = NULL;
    buf_append(b3, 1);
    assert(!buf_empty(b3));
    buf_free(b3);
}

void buf_end_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    assert(buf_end(b1) == NULL);
    buf_free(b1);

    int *b2 = NULL;
    buf_reserve(b2, 1);
    assert(buf_end(b2) == b2 + 0);
    buf_free(b2);

    int *b3 = NULL;
    buf_append(b3, 1);
    assert(buf_end(b3) == b3 + 1);
    buf_free(b3);
}

void buf_free_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    buf_free(b1);
    assert(b1 == NULL);

    int *b2 = NULL;
    buf_reserve(b2, 1);
    buf_free(b2);
    assert(b2 == NULL);

    int *b3 = NULL;
    buf_append(b3, 1);
    buf_free(b3);
    assert(b3 == NULL);
}

void buf_last_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    assert(buf_last(b1) == NULL);
    buf_free(b1);

    int *b2 = NULL;
    buf_reserve(b2, 1);
    assert(buf_last(b2) == b2);
    buf_free(b2);

    int *b3 = NULL;
    buf_push(b3, 42);
    assert(*buf_last(b3) == 42);
    buf_free(b3);
}

void buf_push_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b = NULL;
    int n = 1024;
    for (int i = 0; i < n; i++) {
        buf_push(b, i);
    }
    for (int i = 0, max = buf_len(b); i < max; i++) {
        assert(b[i] == i);
    }

    int *p = buf_push(b, n + 1);
    assert(b[n] == *p && b[n] == n + 1);
    assert(buf_len(b) == n + 1 && buf_cap(b) >= n + 1);
    buf_free(b);
}

void buf_pop_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    buf_pop(b1);
    assert(buf_len(b1) == 0 && buf_cap(b1) == 0);
    buf_free(b1);

    int *b2 = NULL;
    buf_reserve(b2, 1);
    buf_pop(b2);
    assert(buf_len(b2) == 0 && buf_cap(b2) >= 1);
    buf_pop(b2);
    assert(buf_len(b2) == 0 && buf_cap(b2) >= 1);
    buf_free(b2);

    int *b3 = NULL;
    buf_push(b3, 1);
    assert(*buf_pop(b3) == 1);
    assert(buf_len(b3) == 0 && buf_cap(b3) >= 1);
    buf_pop(b3);
    assert(buf_len(b3) == 0 && buf_cap(b3) >= 1);
    buf_free(b3);
}

void buf_reserve_test(void)
{
    // // clang-format off
    // struct t {
    // } tt[] = {
    // };
    // // clang-format on
    // for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
    // }

    int *b1 = NULL;
    buf_reserve(b1, 10);
    assert(buf_len(b1) == 0 && buf_cap(b1) >= 10);
    buf_free(b1);

    int *b2 = NULL;
    buf_append(b2, 4);
    buf_reserve(b2, 256);
    assert(buf_len(b2) == 4 && buf_cap(b2) >= 256);
    buf_free(b2);

    int *b3 = NULL;
    buf_append(b3, 256);
    buf_reserve(b3, 4);
    assert(buf_len(b3) == 256 && buf_cap(b3) >= 256);
    buf_free(b3);
}

void buf_take_test(void)
{
    // clang-format off
    struct t {
        int buf_len, take, result_len;
    } tt[] = {
        {0,  0, 0},
        {0, -1, 0},
        {0,  1, 0},
        {3,  0, 0},
        {3,  1, 1},
    };
    // clang-format on
    for (struct t *t = tt, *end = tt + BUF_COUNT(tt); t != end; ++t) {
        int *b = NULL;
        buf_append(b, t->buf_len);
        buf_take(b, t->take);
        assert(buf_empty(b) == (t->result_len == 0));
        assert(buf_len(b) == t->result_len);
        assert(buf_cap(b) >= t->result_len);
        buf_free(b);
    }
}

void buf_test(void)
{
    buf_init_test();
    buf_append_test();
    buf_at_test();
    buf_clear_test();
    buf_empty_test();
    buf_end_test();
    buf_free_test();
    buf_last_test();
    buf_push_test();
    buf_pop_test();
    buf_reserve_test();
    buf_take_test();
}
