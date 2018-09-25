#include "kvs.h"

struct KVSstore {
    KVSpair *pairs;
    KVScompare *compare;
    size_t length;
    size_t space;
};

static const size_t kvs_pair_size = sizeof(KVSpair);

static const size_t kvs_store_size = sizeof(KVSstore);

static KVSpair *kvs_search(KVSstore *store, KVSkey *key, int exact) {
    KVSpair *pairs = store->pairs;
    size_t lbound = 0;
    size_t rbound = store->length;
    while (lbound < rbound) {
        size_t index = lbound + ((rbound - lbound) >> 1);
        KVSpair *element = pairs + index;
        int result = (int)store->compare((const char *)key, (const char *)element->key);
        if (result < 0) {
            rbound = index;
        } else if (result > 0) {
            lbound = index + 1;
        } else {
            return element;
        }
    }
    return exact ? NULL : pairs + lbound;
}

static KVSpair *kvs_search_2(KVSstore *store, KVSkey *key) {
    KVSpair *pair;
    int i = 0;
    
    while ((pair = kvs_pair(store, i++))) {
        if(strcmp((const char *)key, (const char *)pair->key) == 0){
            return pair;
        }        
    }

    return NULL;
}

void kvs_print(KVSstore *store){
    KVSpair *pair;
    int i = 0;

    printf("Priting kvs keys\n");
    
    while ((pair = kvs_pair(store, i++))) {
        printf("%s:\n", (char *)pair->key);
    }
}

static KVSpair *kvs_get_pair(KVSstore *store, KVSkey *key) {
    if ((!store) || (!store->pairs)) {
        return NULL;
    }
    //return kvs_search(store, key, 1);
    return kvs_search_2(store, key);
}

static void kvs_abort_if_null(void *pointer, const char *message) {
    if (pointer == NULL) {
        fprintf(stderr, "%s\n", message);
        exit(-1);
    }
}

static void kvs_resize_pairs(KVSstore *store, size_t amount) {
    if (!store) {
        return;
    }
    store->length += amount;
    if (store->space > store->length) {
        return;
    }
    store->space += KVS_SPACE_INCREMENT;
    store->pairs = (KVSpair*)realloc(store->pairs, kvs_pair_size * store->space);
    kvs_abort_if_null(store->pairs, "out of memory");
}

static size_t kvs_get_pair_index(KVSstore *store, KVSpair *pair) {
    if ((!store) || (!pair)) {
        return -1;
    }
    return pair - store->pairs;
}

static size_t kvs_get_bytes_from_pair(KVSstore *store, KVSpair *pair) {
    size_t pair_index;
    if ((!store) || (!pair)) {
        return 0;
    }
    pair_index = kvs_get_pair_index(store, pair);
    return (store->length - pair_index) * kvs_pair_size;
}

static void kvs_create_pair(KVSstore *store, KVSkey *key, KVSvalue *value) {
    KVSpair *pair;
    if (!store) {
        return;
    }
    pair = kvs_search(store, key, 0);
    if (pair < store->pairs + store->length) {
        size_t bytes = kvs_get_bytes_from_pair(store, pair);
        memmove(pair + 1, pair, bytes);
    }
    pair->key = key;
    pair->value = value;
    kvs_resize_pairs(store, +1);
}

static void kvs_remove_pair(KVSstore *store, KVSpair *pair) {
    if ((!store) || (!pair)) {
        return;
    }
    memmove(pair, pair + 1, kvs_get_bytes_from_pair(store, pair + 1));
    kvs_resize_pairs(store, -1);
}

static int kvs_compare_pointers(const char *a, const char *b) {
    return a - b;
}

KVSstore *kvs_create(KVScompare compare) {
    KVSstore *store = (KVSstore *)malloc(kvs_store_size);
    kvs_abort_if_null(store, "out of memory");
    store->pairs = NULL;
    store->length = 0;
    store->space = 0;
    if (compare) {
        store->compare = compare;
    } else {
        store->compare = kvs_compare_pointers;
    }
    kvs_resize_pairs(store, 0);
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

void kvs_put(KVSstore *store, KVSkey *key, void *value) {
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

KVSvalue *kvs_get(KVSstore *store, KVSkey *key) {
    KVSpair *pair = kvs_get_pair(store, key);
    return pair ? pair->value : NULL;
}

void kvs_remove(KVSstore *store, KVSkey *key) {
    kvs_put(store, key, NULL);
}

size_t kvs_length(KVSstore *store) {
    if (!store) {
        return 0;
    }
    return store->length;
}

KVSpair *kvs_pair(KVSstore *store, size_t index) {
    if ((!store) || (index < 0) || (index >= store->length)) {
        return NULL;
    }
    return store->pairs + index;
}