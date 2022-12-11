#include "kvs.h"

static const size_t kvs_pair_size = sizeof(KVSpair);

static const size_t kvs_store_size = sizeof(KVSstore);

static int kvs_sort_compare(const void *a, const void *b) {
    const KVSpair *pairA = a;
    const KVSpair *pairB = b;
    if (pairA->key > pairB->key) {
        return -1;
    }
    if (pairA->key < pairB->key) {
        return 1;
    }
    return 0;
}

static int kvs_search_compare(const void *key, const void *element) {
    const KVSpair *pair = element;
    if (key > pair->key) {
        return -1;
    }
    if (key < pair->key) {
        return 1;
    }
    return 0;
}

static KVSpair *kvs_get_pair(KVSstore *store, const void *key) {
    if ((!store) || (!store->pairs)) {
        return NULL;
    }
    return bsearch(key, store->pairs, store->length, kvs_pair_size,
                    kvs_search_compare);
}

static void kvs_sort_pairs(KVSstore *store) {
    if ((!store) || (!store->pairs)) {
        return;
    }
    qsort(store->pairs, store->length, kvs_pair_size, kvs_sort_compare);
}

static void kvs_resize_pairs(KVSstore *store) {
    if (!store) {
        return;
    }
    store->pairs = realloc(store->pairs, kvs_pair_size * store->length);
}

static void kvs_create_pair(KVSstore *store, const void *key, void *value) {
    KVSpair *pair;
    if (!store) {
        return;
    }
    ++store->length;
    kvs_resize_pairs(store);
    pair = &store->pairs[store->length - 1];
    pair->key = key;
    pair->value = value;
    kvs_sort_pairs(store);
}

static void kvs_remove_pair(KVSstore *store, KVSpair *pair) {
    if ((!store) || (!pair)) {
        return;
    }
    pair->key = NULL;
    kvs_sort_pairs(store);
    --store->length;
    kvs_resize_pairs(store);
}

KVSstore *kvs_create(void) {
    KVSstore *store = malloc(kvs_store_size);
    store->pairs = NULL;
    store->length = 0;
    return store;
}

void kvs_destroy(KVSstore *store) {
    if (!store) {
        return;
    }
    if (store->pairs) {
        free(store->pairs);
    }
    free(store);
}

void kvs_put(KVSstore *store, const void *key, void *value) {
    KVSpair *pair = kvs_get_pair(store, key);
    if (pair) {
        if (value) {
            pair->value = value;
        } else {
            kvs_remove_pair(store, pair);
        }
    } else if (value) {
        kvs_create_pair(store, key, value);
    }
}

void *kvs_get(KVSstore *store, const void *key) {
    KVSpair *pair = kvs_get_pair(store, key);
    return pair ? pair->value : NULL;
}
