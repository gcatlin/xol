#include "common.h"

typedef struct {
    size_t len;
    size_t cap;
    char buf[];
} buf_hdr_t;

// clang-format off
#define buf__raw(b) ((size_t *)(b)-2)
#define buf__len(b) buf__raw(b)[0]
#define buf__cap(b) buf__raw(b)[1]

#define buf__fit(b, n)  (buf__fits(b, n) ? 0 : buf__grow(b, buf_len(b)+(n)))
#define buf__fits(b, n) ((b) && buf__len(b)+(n) <= buf__cap(b))
#define buf__grow(b, n) (*((void **)&(b)) = buf___grow((b), (n), sizeof(*(b))))

#define buf_cap(b)        ((b) ? buf__cap(b) : 0)
#define buf_len(b)        ((b) ? buf__len(b) : 0)
#define buf_hdr(b)        ((buf_hdr_t *)buf__raw(b))
#define buf_last(b)       ((b)[buf__len(b)-1])
#define buf_free(b)       ((b) ? (free(buf__raw(b)), (b) = NULL) : 0)
#define buf_push(b, x  )  (buf__fit(b, 1), (b)[buf__len(b)++] = (x))
#define buf_reserve(b, n) (buf__fit(b, n), (b)[buf__len(b)])

size_t bufcap(const void *b) { return buf_cap(b); }
size_t buflen(const void *b) { return buf_len(b); }
buf_hdr_t *bufhdr(const void *b) { return buf_hdr(b); }

void *buf___grow(const void *b, size_t len, size_t elem_size)
{
    size_t dbl = 2 * buf_cap(b);
    size_t cap = len < dbl ? dbl : len;
    assert(0 < cap && len <= cap);
    size_t size = offsetof(buf_hdr_t, buf) + elem_size * cap;
    buf_hdr_t *hdr = (buf_hdr_t *)realloc(b ? buf__raw(b) : NULL, size);
    hdr->cap = cap;
    if (!b) hdr->len = 0;
    return hdr->buf;
}
// clang-format on

void buf_test(void)
{
    size_t N = 1024;

    int *b = NULL;
    assert(buf_len(b) == 0);
    assert(buf_cap(b) == 0);

    // push increases len and cap
    for (size_t i = 0; i < N; i++) {
        buf_push(b, i);
    }
    assert(buf_len(b) == N);
    assert(buf_cap(b) >= N);
    assert(buf_last(b) == (int)N-1);

    // push sets values
    for (int i = 0, max = buf_len(b); i < max; i++) {
        assert(b[i] == i);
    }

    buf_free(b);
    assert(buf_free(b) == NULL);
}
