#include "kv/kvstore.h"
#include <cassert>
#include <iostream>

int main() {
    kv::KVStore store;

    store.set("foo", "bar");
    assert(store.exists("foo"));
    assert(store.get("foo").value() == "bar");

    store.set("foo", "baz");
    assert(store.get("foo").value() == "baz");

    assert(store.del("foo"));
    assert(!store.exists("foo"));
    assert(!store.get("foo").has_value());

    std::cout << "All KVStore tests passed!\n";
    return 0;
}