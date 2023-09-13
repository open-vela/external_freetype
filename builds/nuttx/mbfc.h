/**
 * @file mbfc.h
 *
 */

#ifndef MBFC_H
#define MBFC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>

/*********************
 *      DEFINES
 *********************/

#define MBFC_ALIGN(value, align) ((value) & ~((align)-1))

#define MBFC_AGE_MAX 100
#define MBFC_AGE_INC(cache) ((cache)->age < MBFC_AGE_MAX ? (cache)->age++ : 0)

#ifndef MBFC_MALLOC
#define MBFC_MALLOC(size) malloc(size)
#endif

#ifndef MBFC_FREE
#define MBFC_FREE(ptr) free(ptr)
#endif

#ifndef MBFC_ASSERT
#define MBFC_ASSERT(expr) assert(expr)
#endif

#ifdef CONFIG_LIB_FREETYPE_MBFC_DEBUG
#define MBFC_LOG(fmt, ...) syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#else
#define MBFC_LOG(fmt, ...)
#endif

#define MBFC_LOG_INFO MBFC_LOG
#define MBFC_LOG_WARN MBFC_LOG

/**********************
 *      TYPEDEFS
 **********************/

typedef struct mbfc_s mbfc_t;

typedef struct {
    void* fp;
    size_t block_size;
    int cache_num;
    ssize_t (*read_cb)(void* fp, void* buf, size_t nbytes);
    off_t (*seek_cb)(void* fp, off_t pos, int whence);
} mbfc_param_t;

typedef struct mbfc_cache_s {
    off_t pos;
    uint8_t* buf;
    size_t size;
    int age;
} mbfc_cache_t;

struct mbfc_s {
    mbfc_param_t param;
    mbfc_cache_t* cache_arr;
    int cache_hit_cnt;
};

/**********************
 * STATIC FUNCTIONS
 **********************/

static mbfc_cache_t* mbfc_find_block(mbfc_t* mbfc, off_t pos)
{
    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age <= 0) {
            continue;
        }

        pos = MBFC_ALIGN(pos, mbfc->param.block_size);

        if (cache->pos == pos) {
            return cache;
        }
    }

    return NULL;
}

static mbfc_cache_t* mbfc_get_reuse(mbfc_t* mbfc)
{
    int min_age = MBFC_AGE_MAX;
    mbfc_cache_t* min_age_cache = NULL;

    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age <= 0) {
            return cache;
        }

        if (cache->age < min_age) {
            min_age = cache->age;
            min_age_cache = cache;
        }
    }

    return min_age_cache;
}

static void mbfc_age_dec_all(mbfc_t* mbfc)
{
    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age > 1) {
            cache->age--;
        }
    }
}

static mbfc_cache_t* mbfc_read_block(mbfc_t* mbfc, off_t pos)
{
    mbfc_cache_t* cache = mbfc_get_reuse(mbfc);
    MBFC_ASSERT(cache != NULL);

    pos = MBFC_ALIGN(pos, mbfc->param.block_size);
    cache->pos = pos;
    cache->age = 0;

    off_t offset = mbfc->param.seek_cb(mbfc->param.fp, pos, SEEK_SET);

    if (offset < 0) {
        MBFC_LOG_WARN("%s: cache seek %d failed\n", __func__, (int)pos);
        return NULL;
    }

    ssize_t rd = mbfc->param.read_cb(mbfc->param.fp, cache->buf, mbfc->param.block_size);

    if (rd <= 0) {
        MBFC_LOG_WARN("%s: cache read %zd != %zu, failed\n",
            __func__, rd, mbfc->param.block_size);
        return NULL;
    }

    cache->age = 1;
    cache->size = rd;

    MBFC_LOG_INFO("%s: cache read pos: %d, size = %zu\n", __func__, (int)pos, cache->size);

    return cache;
}

/**
 * Initialize a multi-block file cache parameter structure.
 * @param param: Pointer to the mbfc_param_t structure to be initialized.
 */

static void mbfc_param_init(mbfc_param_t* param)
{
    MBFC_ASSERT(param != NULL);
    memset(param, 0, sizeof(mbfc_param_t));
    param->block_size = 1024;
    param->cache_num = 8;
}

/**
 * Create a multi-block file cache.
 * @param param: Pointer to the configuration parameters for cache creation.
 * @return: Pointer to the created mbfc_t structure.
 */

static mbfc_t* mbfc_create(const mbfc_param_t* param)
{
    MBFC_ASSERT(param != NULL);
    MBFC_ASSERT(param->block_size > 0);
    MBFC_ASSERT(param->cache_num > 0);
    MBFC_ASSERT(param->read_cb != NULL);
    MBFC_ASSERT(param->seek_cb != NULL);

    mbfc_t* mbfc = MBFC_MALLOC(sizeof(mbfc_t));
    MBFC_ASSERT(mbfc != NULL);
    memset(mbfc, 0, sizeof(mbfc_t));
    mbfc->param = *param;

    mbfc->cache_arr = MBFC_MALLOC(sizeof(mbfc_cache_t) * param->cache_num);
    MBFC_ASSERT(mbfc->cache_arr != NULL);
    memset(mbfc->cache_arr, 0, sizeof(mbfc_cache_t) * param->cache_num);

    for (int i = 0; i < param->cache_num; i++) {
        mbfc->cache_arr[i].buf = MBFC_MALLOC(param->block_size);
        MBFC_ASSERT(mbfc->cache_arr[i].buf != NULL);
        memset(mbfc->cache_arr[i].buf, 0, param->block_size);
    }

    return mbfc;
}

/**
 * Delete a multi-block file cache and release associated resources.
 * @param mbfc: Pointer to the mbfc_t structure to be deleted.
 */

static void mbfc_delete(mbfc_t* mbfc)
{
    MBFC_ASSERT(mbfc != NULL);

    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];
        MBFC_LOG_INFO("%s: free cache[%d], pos %d, age = %d\n", __func__, i, (int)cache->pos, cache->age);
        MBFC_FREE(cache->buf);
    }

    MBFC_FREE(mbfc->cache_arr);
    MBFC_FREE(mbfc);
}

/**
 * Read data from a multi-block file cache or the underlying file.
 * @param mbfc: Pointer to the mbfc_t structure.
 * @param pos: The position in the file to start reading from.
 * @param buf: Pointer to the buffer where data will be stored.
 * @param nbyte: The number of bytes to read.
 * @return: The number of bytes read, or -1 on error.
 */

static ssize_t mbfc_read(mbfc_t* mbfc, off_t pos, void* buf, size_t nbyte)
{
    MBFC_ASSERT(mbfc != NULL);

    if (!buf || !nbyte) {
        return 0;
    }

    ssize_t ret = 0;
    ssize_t remain = nbyte;
    uint8_t* cur = buf;

    while (remain > 0) {
        mbfc_cache_t* cache = mbfc_find_block(mbfc, pos);

        if (cache) {
            MBFC_LOG_INFO("%s: cache hit pos %d, age = %d\n", __func__, (int)pos, cache->age);
            mbfc->cache_hit_cnt++;
        } else {
            MBFC_LOG_INFO("%s: cache miss %d\n", __func__, (int)pos);
            cache = mbfc_read_block(mbfc, pos);
            if (!cache) {
                MBFC_LOG_WARN("%s: read block failed\n", __func__);
                break;
            }
            mbfc_age_dec_all(mbfc);
        }

        MBFC_AGE_INC(cache);

        size_t cp = cache->size - (pos - cache->pos);
        cp = (cp > remain) ? remain : cp;
        memcpy(cur, cache->buf + (pos - cache->pos), cp);
        pos += cp;
        cur += cp;
        remain -= cp;
        ret += cp;
    }

    return ret;
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MBFC_H*/
