#include <iostream>
#include "lru_cache.h"

using namespace std;

int main() {
    /* Initialize the cache of strings. */
    lru::cache<std::string> cache;
    cache.insert("hello");
    cache.insert("world");

    if (cache.find("hello")) {
        cout << "Hello!" << endl;
    }
    else {
        cout << "Bye!" << endl;
    }

    return 0;
}
