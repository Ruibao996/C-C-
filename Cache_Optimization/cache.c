#include "cache.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* Create a cache simulator according to the config */
struct cache *cache_create(struct cache_config config, struct cache *lower_level)
{
    /*YOUR CODE HERE*/

    struct cache *cache = (struct cache *)malloc(sizeof(struct cache));
    // if (cache == NULL) {
    //     return NULL;
    // }

    cache->config = config;
    cache->lower_cache = lower_level;

    cache->offset_bits = log_calculator(config.line_size);
    cache->index_bits = log_calculator(config.lines / config.ways);
    cache->tag_bits = config.address_bits - cache->offset_bits - cache->index_bits;

    cache->offset_mask = (1 << cache->offset_bits) - 1;
    cache->index_mask = ((1 << cache->index_bits) - 1) << cache->offset_bits;
    cache->tag_mask = ((1 << cache->tag_bits) - 1) << (cache->offset_bits + cache->index_bits);

    cache->lines = (struct cache_line *)malloc(sizeof(struct cache_line) * config.lines);
    if (cache->lines == NULL)
    {
        free(cache);
        return NULL;
    }

    for (uint32_t i = 0; i < config.lines; i++)
    {
        cache->lines[i].valid = false;
        cache->lines[i].dirty = false;
        cache->lines[i].tag = 0;
        cache->lines[i].last_access = 0;
        cache->lines[i].data = (uint8_t *)malloc(config.line_size);
        if (cache->lines[i].data == NULL)
        {
            for (uint32_t j = 0; j < i; j++)
            {
                free(cache->lines[j].data);
            }
            free(cache->lines);
            free(cache);
            return NULL;
        }
    }

    return cache;
}

/*
Release the resources allocated for the cache simulator.
Also writeback dirty lines

The order in which lines are evicted is:
set0-slot0, set1-slot0, set2-slot0, (the 0th way)
set0-slot1, set1-slot1, set2-slot1, (the 1st way)
set0-slot2, set1-slot2, set2-slot2, (the 2nd way)
and so on.
*/
void cache_destroy(struct cache *cache)
{
    if (cache == NULL)
    {
        return;
    }

    uint32_t sets = cache->config.lines / cache->config.ways;

    // Iterate over the cache in the order of set0-slot0, set1-slot0, ..., set0-slot1, set1-slot1, ...
    for (uint32_t way = 0; way < cache->config.ways; way++)
    {
        for (uint32_t set = 0; set < sets; set++)
        {
            uint32_t index = set * cache->config.ways + way;
            struct cache_line *line = &cache->lines[index];
            if (line->dirty)
            {
                uint32_t addr = (line->tag << (cache->index_bits + cache->offset_bits)) | (set << cache->offset_bits);
                if (cache->lower_cache != NULL)
                {
                    // Write back the entire cache line to the lower cache
                    for (uint32_t j = 0; j < cache->config.line_size; ++j)
                    {
                        cache_write_byte(cache->lower_cache, addr + j, line->data[j]);
                    }
                }
                else
                {
                    // Write back the entire cache line to main memory
                    mem_store(line->data, addr, cache->config.line_size);
                }
            }
            // Free the data in the cache line
            free(line->data);
        }
    }

    // Free the lines array
    free(cache->lines);
    // Free the cache structure
    free(cache);
}


/* Read one byte at a specific address. return hit=true/miss=false */
bool cache_read_byte(struct cache *cache, uint32_t addr, uint8_t *byte) 
{
    uint32_t index = (addr & cache->index_mask) >> cache->offset_bits;
    uint32_t tag = (addr & cache->tag_mask) >> (cache->index_bits + cache->offset_bits);
    uint32_t offset = addr & cache->offset_mask;

    // Calculate the set index and the base line index
    uint32_t set_index = index * cache->config.ways;

    for (uint32_t i = 0; i < cache->config.ways; i++) {
        struct cache_line *line = &cache->lines[set_index + i];
        if (line->valid && line->tag == tag) {
            // Cache hit
            *byte = line->data[offset];
            line->last_access = get_timestamp();
            return true;
        }
    }

    // Cache miss
    // Find the LRU line or an invalid line
    struct cache_line *lru_line = NULL;
    uint64_t oldest_access = UINT64_MAX;

    for (uint32_t i = 0; i < cache->config.ways; i++) {
        struct cache_line *line = &cache->lines[set_index + i];
        if (!line->valid) {
            lru_line = line;
            break;
        }
        if (line->last_access < oldest_access) {
            oldest_access = line->last_access;
            lru_line = line;
        }
    }

    if (lru_line->valid && lru_line->dirty && cache->config.write_back) {
        uint32_t evict_addr = (lru_line->tag << (cache->index_bits + cache->offset_bits)) | (index << cache->offset_bits);
        if (cache->lower_cache != NULL) {
            for (uint32_t i = 0; i < cache->config.line_size; ++i) {
                cache_write_byte(cache->lower_cache, evict_addr + i, lru_line->data[i]);
            }
        } else {
            mem_store(lru_line->data, evict_addr, cache->config.line_size);
        }
    }

    // Load the new line from memory
    uint32_t block_addr = addr & ~(cache->config.line_size - 1);
    if (cache->lower_cache != NULL) {
        for (uint32_t i = 0; i < cache->config.line_size; ++i) {
            cache_read_byte(cache->lower_cache, block_addr + i, &lru_line->data[i]);
        }
    } else {
        mem_load(lru_line->data, block_addr, cache->config.line_size);
    }

    lru_line->valid = true;
    lru_line->dirty = false;
    lru_line->tag = tag;
    lru_line->last_access = get_timestamp();

    *byte = lru_line->data[offset];
    return false;
}

/* Write one byte into a specific address. return hit=true/miss=false*/
bool cache_write_byte(struct cache *cache, uint32_t addr, uint8_t byte) {
    uint32_t index = (addr & cache->index_mask) >> cache->offset_bits;
    uint32_t tag = (addr & cache->tag_mask) >> (cache->index_bits + cache->offset_bits);
    uint32_t offset = addr & cache->offset_mask;

    // Calculate the set index and the base line index
    uint32_t set_index = index * cache->config.ways;

    for (uint32_t i = 0; i < cache->config.ways; i++) {
        struct cache_line *line = &cache->lines[set_index + i];
        if (line->valid && line->tag == tag) {
            // Cache hit
            line->data[offset] = byte;
            line->last_access = get_timestamp();
            if (cache->config.write_back) {
                line->dirty = true;
            } else {
                if (cache->lower_cache != NULL) {
                    cache_write_byte(cache->lower_cache, addr, byte);
                } else {
                    mem_store(line->data, addr - offset, cache->config.line_size);
                }
            }
            return true;
        }
    }

    // Cache miss
    // Find the LRU line or an invalid line
    struct cache_line *lru_line = NULL;
    uint64_t oldest_access = UINT64_MAX;

    for (uint32_t i = 0; i < cache->config.ways; i++) {
        struct cache_line *line = &cache->lines[set_index + i];
        if (!line->valid) {
            lru_line = line;
            break;
        }
        if (line->last_access < oldest_access) {
            oldest_access = line->last_access;
            lru_line = line;
        }
    }

    if (lru_line->valid && lru_line->dirty && cache->config.write_back) {
        uint32_t evict_addr = (lru_line->tag << (cache->index_bits + cache->offset_bits)) | (index << cache->offset_bits);
        if (cache->lower_cache != NULL) {
            for (uint32_t i = 0; i < cache->config.line_size; ++i) {
                cache_write_byte(cache->lower_cache, evict_addr + i, lru_line->data[i]);
            }
        } else {
            mem_store(lru_line->data, evict_addr, cache->config.line_size);
        }
    }

    // Load the new line from memory
    uint32_t block_addr = addr & ~(cache->config.line_size - 1);
    if (cache->lower_cache != NULL) {
        for (uint32_t i = 0; i < cache->config.line_size; ++i) {
            cache_read_byte(cache->lower_cache, block_addr + i, &lru_line->data[i]);
        }
    } else {
        mem_load(lru_line->data, block_addr, cache->config.line_size);
    }

    lru_line->valid = true;
    lru_line->dirty = cache->config.write_back;
    lru_line->tag = tag;
    lru_line->last_access = get_timestamp();

    lru_line->data[offset] = byte;

    if (!cache->config.write_back) {
        if (cache->lower_cache != NULL) {
            cache_write_byte(cache->lower_cache, addr, byte);
        } else {
            mem_store(lru_line->data, addr - offset, cache->config.line_size);
        }
    }

    return false;
}


uint32_t log_calculator(uint32_t num)
{
    uint32_t count = 0;
    while (num >>= 1)
    {
        count++;
    }
    return count;
}