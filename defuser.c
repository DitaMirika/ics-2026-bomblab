#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}


int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge;
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t t1 = 3u * (high - middle) + 0x898au;
    uint32_t t0 = (low + 4u * high) ^ 0x8ddu;

    uint32_t values[MAX_FIELDS] = {0};
    values[0] = 8u * t0 + (t1 ^ 0x9975u);
    size_t count = 1;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 14) & 0xffu) ^ 5u;

    for (uint32_t i = 1; i < 6; i++) {
        uint32_t part = (uint32_t)((challenge >> (15u + 2u * i)) & 0x1fu);
        values[i] = (3u * values[i - 1]) ^ (part + i);
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = index;
    switch (index) {
        case 0: values[1] = 3u * x + 0xfdb6u; break;
        case 1: values[1] = (2u * x) ^ 0xaf5bu; break;
        case 2: values[1] = (4u * x) ^ 0x3d2du; break;
        case 3: values[1] = 5u * x + 0x1ceadu; break;
        case 4: values[1] = (x ^ 0x94b9u) + (x >> 1); break;
        case 5: values[1] = 5u * x + 0xf9fbu; break;
        case 6: values[1] = (x ^ 0xd0c2u) + (x >> 1); break;
        case 7: values[1] = 5u * x + 0xbaa9u; break;
    }
    return write_answer(out, cap, values, 2);
}

static uint32_t phase4_walk(uint32_t target, uint32_t lower, uint32_t upper,
                            uint32_t salt) {
    uint32_t middle = lower + ((upper - lower) >> 1);
    if (target == middle) return (middle + 0x83u) ^ salt;
    if (target < middle) {
        uint32_t child = phase4_walk(target, lower, middle - 1u, salt);
        return (3u * child) ^ (middle + salt) ^ 0xau;
    }
    uint32_t child = phase4_walk(target, middle + 1u, upper, salt);
    return (3u * child) ^ (middle + salt) ^ 0x15u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)challenge & 0x7fu;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase4_walk(target, 0, 127, salt);
    return write_answer(out, cap, values, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    struct node {
        uint32_t value;
        uint32_t id;
        const struct node *next;
    };
    static const struct node nodes[12] = {
        {0xe10au, 1655u, &nodes[3]},
        {0x1c19u, 2368u, &nodes[11]},
        {0xb6f6u, 2912u, &nodes[0]},
        {0x498du, 2176u, &nodes[6]},
        {0x94b2u, 801u, &nodes[8]},
        {0x7351u, 1104u, &nodes[4]},
        {0x0ae4u, 283u, NULL},
        {0x60bau, 520u, &nodes[9]},
        {0x7b05u, 2571u, &nodes[7]},
        {0xb924u, 1819u, &nodes[10]},
        {0x0e1cu, 1323u, &nodes[2]},
        {0x34eeu, 78u, &nodes[5]}
    };
    static const struct node *const heads[8] = {
        &nodes[1], &nodes[11], &nodes[5], &nodes[4],
        &nodes[8], &nodes[7], &nodes[9], &nodes[10]
    };
    uint32_t values[MAX_FIELDS] = {0};
    const struct node *current = heads[(uint32_t)challenge & 7u];
    uint32_t hops = (uint32_t)(challenge >> 3) & 3u;
    uint32_t state = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ 0x242bu;
    for (uint32_t i = 0; i <= hops; i++) {
        state = (state ^ current->value) + current->id;
        values[1] = current->id;
        if (i < hops) current = current->next;
    }
    values[0] = state;
    return write_answer(out, cap, values, 2);
}

static uint32_t phase6_apply(uint32_t index, uint32_t x) {
    switch (index) {
        case 0:
        case 4:
            return (x >> 3) | (x << 29);
        case 1:
            return (x + 0x1260u) ^ (x >> 5);
        case 2:
            return ((x << 7) | (x >> 25)) ^ 0x2e76u;
        case 3:
        case 7:
            return (2u * x + 0x1809u) ^ (7u * x);
        case 5:
            return (x ^ (x >> 7)) + 0x3cfau;
        case 6:
            return (x ^ 0x41b9u) + 2u * x;
        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t first = (uint32_t)challenge & 7u;
    uint32_t second = (uint32_t)(challenge >> 3) & 7u;
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t intermediate = phase6_apply(first, x) ^ 0xdba3u;
    values[0] = phase6_apply(second, intermediate) + 0x7533u;
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t mode = (uint32_t)context & 3u;
    uint32_t folded_challenge = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t folded_context = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t x = folded_challenge ^ folded_context;
    uint32_t mixed = (x ^ 0x2770u) + ((x >> 3) ^ 0x76f4u);
    uint32_t second = mixed ^ (mode * 0x2770u) ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ ((folded_challenge >> 3) + folded_context + mode + 0x76f4u);
    int size = snprintf(out, cap,
                        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
                        mode, second, third);
    return size < 0 || (size_t)size >= cap;
}
