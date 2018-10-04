# Generic Hash based LRU Cache

The cache functionality is implemented through two unordered sets (primary and secondary).
Every valid insert() populates the primary set. Every item is search first in primary set.

If found, the cache will return true, indicating that the item is present in the cache.
However, if primary set does not have the item, the secondary set is searched. If the secondary one
also does not have the item, the cache returns false, indicating not found.

If in case, the primary set gets full with items (reached configured cache size), a new inserted item
is then added into the secondary set. Now, the secondary set becomes the primary set and primary
set assumes the role of secondary set. Unlike the initial case, here both primary and secondary sets have
entries.

If an item is not found inside primary set and is found inside secondary set, the item is also added
into the primary set. If the current primary set also gets full, the current secondary
set is emptied and a new subsequent item is added to the secondary set. The secondary set then
again becomes the primary set and the primary becomes secondary. This process then continues. Here,
we are safe to empty the secondary set since it only contains either the entries that are also present in the
primary set or the entries that have not been accessed since last cache size times.

The pseudo code for the algorithm is below:

    count = 0
    insert(elem):
        if (count < maxsize)
            insert the entry into primary set
        else
            if (secondary is not empty)
                empty secondary set
            insert the entry into secondary set
            make secondary set as primary
        count = count + 1

    find(elem):
        if (elem is in primary set)
            return true
        if (elem is NOT in secondary set)
            return false
        // elem was found inside secondary set
        insert elem into primary set from secondary set
        return true

