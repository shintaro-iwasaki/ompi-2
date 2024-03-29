/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2016 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2015      Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2019      Sandia National Laboratories.  All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef OPAL_MCA_THREADS_ARGOBOTS_THREADS_ARGOBOTS_WAIT_SYNC_H
#define OPAL_MCA_THREADS_ARGOBOTS_THREADS_ARGOBOTS_WAIT_SYNC_H

#include "opal/mca/threads/argobots/threads_argobots.h"
#include <abt.h>

typedef struct ompi_wait_sync_t {
    opal_atomic_int32_t count;
    int32_t status;
    ABT_cond condition;
    ABT_mutex lock;
    struct ompi_wait_sync_t *next;
    struct ompi_wait_sync_t *prev;
    volatile bool signaling;
} ompi_wait_sync_t;

#define SYNC_WAIT(sync) \
    (opal_using_threads() ? ompi_sync_wait_mt (sync) : sync_wait_st (sync))

/* The loop in release handles a race condition between the signaling
 * thread and the destruction of the condition variable. The signaling
 * member will be set to false after the final signaling thread has
 * finished operating on the sync object. This is done to avoid
 * extra atomics in the signalling function and keep it as fast
 * as possible. Note that the race window is small so spinning here
 * is more optimal than sleeping since this macro is called in
 * the critical path. */
#define WAIT_SYNC_RELEASE(sync)                       \
    if (opal_using_threads()) {                       \
        opal_threads_argobots_ensure_init();          \
        while ((sync)->signaling) {                   \
            ABT_thread_yield();                       \
            continue;                                 \
        }                                             \
        ABT_cond_free(&(sync)->condition);            \
        ABT_mutex_free(&(sync)->lock);                \
    }

#define WAIT_SYNC_RELEASE_NOWAIT(sync)                \
    if (opal_using_threads()) {                       \
        opal_threads_argobots_ensure_init();          \
        ABT_cond_free(&(sync)->condition);            \
        ABT_mutex_free(&(sync)->lock);                \
    }


#define WAIT_SYNC_SIGNAL(sync)                        \
    if (opal_using_threads()) {                       \
        opal_threads_argobots_ensure_init();          \
        ABT_mutex_lock(sync->lock);                   \
        ABT_cond_signal(sync->condition);             \
        ABT_mutex_unlock(sync->lock);                 \
        sync->signaling = false;                      \
    }

#define WAIT_SYNC_SIGNALLED(sync)                     \
    {                                                 \
        (sync)->signaling = false;                    \
    }

OPAL_DECLSPEC int ompi_sync_wait_mt(ompi_wait_sync_t *sync);
static inline int sync_wait_st(ompi_wait_sync_t *sync)
{
    opal_threads_argobots_ensure_init();
    while (sync->count > 0) {
        opal_progress();
        ABT_thread_yield();
    }
    return sync->status;
}


#define WAIT_SYNC_INIT(sync,c)                                  \
    do {                                                        \
        (sync)->count = (c);                                    \
        (sync)->next = NULL;                                    \
        (sync)->prev = NULL;                                    \
        (sync)->status = 0;                                     \
        (sync)->signaling = (0 != (c));                         \
        if (opal_using_threads()) {                             \
            opal_threads_argobots_ensure_init();                \
            ABT_cond_create (&(sync)->condition);               \
            ABT_mutex_create (&(sync)->lock);                   \
        }                                                       \
    } while (0)

#endif /* OPAL_MCA_THREADS_ARGOBOTS_THREADS_ARGOBOTS_WAIT_SYNC_H */
