#include "libvec.h"
#include <assert.h>

const size_t DEFAULT_CAP = 8;
const size_t PTR_SIZE = sizeof(char*);

Vec new_vec(void) {
        const char **data = malloc(DEFAULT_CAP * PTR_SIZE);
        if (!data) epanic("failed to malloc");

        return (Vec){.data = data, .cap = DEFAULT_CAP, .len = 0};
}

void push(Vec *const vec, const_str val) {
        if (vec->len == vec->cap) {
                vec->cap <<= 1;
                vec->data = realloc(vec->data, vec->cap * PTR_SIZE);
                if (!vec->data) epanic("failed to realloc");
        }
        assert(vec->len < vec->cap);

        vec->data[vec->len++] = val;
}

void reserve(Vec *const vec, const size_t additional) {
        const size_t min_cap = vec->len + additional;

        if (min_cap <= vec->cap) return;

        const size_t new_cap = max(min_cap, vec->cap << 1);
        vec->cap = new_cap;
        vec->data = realloc(vec->data, vec->cap * PTR_SIZE);
        if (!vec->data) epanic("failed to realloc");
}
