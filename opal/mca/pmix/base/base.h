/*
 * Copyright (c) 2014-2019 Intel, Inc.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file:
 */

#ifndef MCA_PMI_BASE_H
#define MCA_PMI_BASE_H

#include "opal_config.h"
#include "opal/types.h"
#include "opal/mca/threads/threads.h"
#include "opal/mca/mca.h"
#include "opal/mca/base/mca_base_framework.h"

#include "opal/mca/pmix/pmix-internal.h"

BEGIN_C_DECLS

OPAL_DECLSPEC extern mca_base_framework_t opal_pmix_base_framework;

/**
 * Select a pmix module
 */
OPAL_DECLSPEC int opal_pmix_base_select(void);

OPAL_DECLSPEC extern bool opal_pmix_base_allow_delayed_server;

OPAL_DECLSPEC int opal_pmix_base_exchange(pmix_info_t *info,
                                          pmix_pdata_t *pdat,
                                          int timeout);

OPAL_DECLSPEC void opal_pmix_base_set_evbase(opal_event_base_t *evbase);

#define opal_pmix_condition_wait(a,b)    opal_cond_wait(a, b)
typedef opal_cond_t opal_pmix_condition_t;
#define opal_pmix_condition_broadcast(a) opal_cond_broadcast(a)
#define opal_pmix_condition_signal(a)    opal_cond_signal(a)

typedef struct {
    opal_mutex_t mutex;
    opal_pmix_condition_t cond;
    volatile bool active;
    int status;
} opal_pmix_lock_t;


typedef struct {
    opal_event_base_t *evbase;
    int timeout;
    int initialized;
    opal_pmix_lock_t lock;
} opal_pmix_base_t;

extern opal_pmix_base_t opal_pmix_base;

#define OPAL_PMIX_CONDITION_STATIC_INIT PTHREAD_COND_INITIALIZER
#define OPAL_PMIX_CONSTRUCT_LOCK(l)                     \
    do {                                                \
        OBJ_CONSTRUCT(&(l)->mutex, opal_mutex_t);       \
        opal_cond_init(&(l)->cond);                     \
        (l)->active = true;                             \
        OPAL_POST_OBJECT((l));                          \
    } while(0)

#define OPAL_PMIX_DESTRUCT_LOCK(l)          \
    do {                                    \
        OPAL_ACQUIRE_OBJECT((l));           \
        OBJ_DESTRUCT(&(l)->mutex);          \
        opal_cond_destroy(&(l)->cond);      \
    } while(0)


#if OPAL_ENABLE_DEBUG
#define OPAL_PMIX_ACQUIRE_THREAD(lck)                               \
    do {                                                            \
        opal_mutex_lock(&(lck)->mutex);                             \
        if (opal_debug_threads) {                                   \
            opal_output(0, "Waiting for thread %s:%d",              \
                        __FILE__, __LINE__);                        \
        }                                                           \
        while ((lck)->active) {                                     \
            opal_pmix_condition_wait(&(lck)->cond, &(lck)->mutex);  \
        }                                                           \
        if (opal_debug_threads) {                                   \
            opal_output(0, "Thread obtained %s:%d",                 \
                        __FILE__, __LINE__);                        \
        }                                                           \
        (lck)->active = true;                                       \
    } while(0)
#else
#define OPAL_PMIX_ACQUIRE_THREAD(lck)                               \
    do {                                                            \
        opal_mutex_lock(&(lck)->mutex);                             \
        while ((lck)->active) {                                     \
            opal_pmix_condition_wait(&(lck)->cond, &(lck)->mutex);  \
        }                                                           \
        (lck)->active = true;                                       \
    } while(0)
#endif


#if OPAL_ENABLE_DEBUG
#define OPAL_PMIX_WAIT_THREAD(lck)                                  \
    do {                                                            \
        opal_mutex_lock(&(lck)->mutex);                             \
        if (opal_debug_threads) {                                   \
            opal_output(0, "Waiting for thread %s:%d",              \
                        __FILE__, __LINE__);                        \
        }                                                           \
        while ((lck)->active) {                                     \
            opal_pmix_condition_wait(&(lck)->cond, &(lck)->mutex);  \
        }                                                           \
        if (opal_debug_threads) {                                   \
            opal_output(0, "Thread obtained %s:%d",                 \
                        __FILE__, __LINE__);                        \
        }                                                           \
        OPAL_ACQUIRE_OBJECT(&lck);                                  \
        opal_mutex_unlock(&(lck)->mutex);                           \
    } while(0)
#else
#define OPAL_PMIX_WAIT_THREAD(lck)                                  \
    do {                                                            \
        opal_mutex_lock(&(lck)->mutex);                             \
        while ((lck)->active) {                                     \
            opal_pmix_condition_wait(&(lck)->cond, &(lck)->mutex);  \
        }                                                           \
        OPAL_ACQUIRE_OBJECT(lck);                                   \
        opal_mutex_unlock(&(lck)->mutex);                           \
    } while(0)
#endif


#if OPAL_ENABLE_DEBUG
#define OPAL_PMIX_RELEASE_THREAD(lck)                   \
    do {                                                \
        if (opal_debug_threads) {                       \
            opal_output(0, "Releasing thread %s:%d",    \
                        __FILE__, __LINE__);            \
        }                                               \
        (lck)->active = false;                          \
        opal_pmix_condition_broadcast(&(lck)->cond);    \
        opal_mutex_unlock(&(lck)->mutex);               \
    } while(0)
#else
#define OPAL_PMIX_RELEASE_THREAD(lck)                   \
    do {                                                \
        assert(0 != opal_mutex_trylock(&(lck)->mutex)); \
        (lck)->active = false;                          \
        opal_pmix_condition_broadcast(&(lck)->cond);    \
        opal_mutex_unlock(&(lck)->mutex);               \
    } while(0)
#endif


#define OPAL_PMIX_WAKEUP_THREAD(lck)                    \
    do {                                                \
        opal_mutex_lock(&(lck)->mutex);                 \
        (lck)->active = false;                          \
        OPAL_POST_OBJECT(lck);                          \
        opal_pmix_condition_broadcast(&(lck)->cond);    \
        opal_mutex_unlock(&(lck)->mutex);               \
    } while(0)

END_C_DECLS

#endif
