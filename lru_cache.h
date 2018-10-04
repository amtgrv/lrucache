/*
 * Copyright (c) 2017, Amit Gaurav
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the project nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY AMIT GAURAV ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AMIT GAURAV BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * Header file for storing LRU entries.
 */

#ifndef __LRU_CACHE_H__
#define __LRU_CACHE_H__

#include <mutex>
#include <unordered_set>

namespace lru
{

#define CACHE_DEFAULT_MAXSIZE 1000

/*
 * Class to define cache implementation.
 * The cache design uses two hash sets that
 * are used interchangeably.
 */
template<typename K,
         typename H = std::hash<K>,
         typename P = std::equal_to<K>,
         typename A = std::allocator<K>>
class cache {

public:

    cache() : size_(CACHE_DEFAULT_MAXSIZE)
    {}

    ~cache() noexcept {}

    /* Non copyable and movable. */
    cache(
        const cache&
        ) = delete;

    cache&
    operator=(
        const cache&
        ) = delete;

    cache(
        cache&&
        ) = delete;

    cache&
    operator=(
        cache&&
        ) = delete;

    /*
     * Insert an element into current primary set.
     * If primary set is full, insert into secondary
     * set and make that one primary.
     */
    auto
    insert(
        K const& elem
        ) -> void
    {
        bool status = false;

        std::lock_guard<std::mutex> guard(cachelock);
        if (count_ < size_) {
            status = insert_element(elem);
            if (status) goto cleanup;
        }
        else {
            /* move the set store */
            active_ = (active_+1)%2;
            count_ = 0;

            /* This set contains old + duplicate items.
             * clean it before storing new ones. */
            setstore_[active_].clear();

            status = insert_element(elem);
            if (status) goto cleanup;
        }

        count_++;
        status = true;

    cleanup:
        return;
    }

    /*
     * Remove all elements from the cache sets
     */
    auto
    clear() -> void
    {
        std::lock_guard<std::mutex> guard(cachelock);
        setstore_[0].clear();
        setstore_[1].clear();
        count_ = 0;
    }

    /*
     * Check whether an incoming element is present
     * inside the cache sets. First check primary set,
     * if not found, check secondary set.
     * If found inside secondary set, add to primary.
     */
    auto
    find(
        K const& elem
        ) -> bool
    {
        bool found = false;

        {
            std::lock_guard<std::mutex> guard(cachelock);
            /* search active set first */
            found = (setstore_[active_].count(elem) > 0);
            if (found) goto cleanup;

            /* search standby set */
            found = (setstore_[(active_+1)%2].count(elem) > 0);
            if (!found) goto cleanup;
        }

        /* found entry in standby set, copy to active */
        insert(elem);

    cleanup:

        return found;
    }

    /*
     * Set maximum size for elements inside cache sets
     */
    auto
    set_size(
        int size
        ) -> void
    {
        std::lock_guard<std::mutex> guard(cachelock);
        size_ = size;
    }

private:

    /* maximum size in cache */
    int size_{0};

    /* current count of elements */
    int count_{0};

    /* stores all configured elements (two sets) */
    std::unordered_set<K, H, P, A> setstore_[2];

    /* index of active set */
    int active_{0};

    /* thread synchronization */
    std::mutex cachelock;

    /* internal insert routine */
    auto
    insert_element(
        K const& elem
        ) -> bool
    {
        bool inserted = false;
        try {
            setstore_[active_].insert(elem);
            inserted = true;
        }
        catch (std::exception ex) {
            /* TODO: Do something useful. */
            throw;
        }

        return inserted;
    }
};

}

#endif
