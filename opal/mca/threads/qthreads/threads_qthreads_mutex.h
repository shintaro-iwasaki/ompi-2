/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2018 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * Copyright (c) 2015-2016 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2019      Sandia National Laboratories.  All rights reserved.
 * Copyright (c) 2020      Triad National Security, LLC. All rights
 *                         reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef OPAL_MCA_THREADS_QTHREADS_THREADS_QTHREADS_MUTEX_H
#define OPAL_MCA_THREADS_QTHREADS_THREADS_QTHREADS_MUTEX_H

/**
 * @file:
 *
 * Mutual exclusion functions: Unix implementation.
 *
 * Functions for locking of critical sections.
 *
 */

#include "opal/mca/threads/qthreads/threads_qthreads.h"
#include "opal_config.h"

#include <errno.h>
#include <stdio.h>

#include "opal/class/opal_object.h"
#include "opal/sys/atomic.h"
#include "opal/util/output.h"

#include <qthread/qthread.h>

BEGIN_C_DECLS

struct opal_mutex_t {
    opal_object_t super;

    aligned_t m_lock_qthreads;
    int m_recursive;

#if OPAL_ENABLE_DEBUG
    int m_lock_debug;
    const char *m_lock_file;
    int m_lock_line;
#endif

    opal_atomic_lock_t m_lock_atomic;
};

typedef aligned_t opal_qthreads_mutex_t;

OPAL_DECLSPEC OBJ_CLASS_DECLARATION(opal_mutex_t);
OPAL_DECLSPEC OBJ_CLASS_DECLARATION(opal_recursive_mutex_t);

#if OPAL_ENABLE_DEBUG
#define OPAL_MUTEX_STATIC_INIT                                          \
    {                                                                   \
        .super = OPAL_OBJ_STATIC_INIT(opal_mutex_t),                    \
        .m_lock_qthreads = 0,                         \
        .m_recursive = 0,                                               \
        .m_lock_debug = 0,                                              \
        .m_lock_file = NULL,                                            \
        .m_lock_line = 0,                                               \
        .m_lock_atomic = OPAL_ATOMIC_LOCK_INIT,                         \
    }
#else
#define OPAL_MUTEX_STATIC_INIT                                          \
    {                                                                   \
        .super = OPAL_OBJ_STATIC_INIT(opal_mutex_t),                    \
        .m_lock_qthreads = 0,                         \
        .m_recursive = 0,                                               \
        .m_lock_atomic = OPAL_ATOMIC_LOCK_INIT,                         \
    }
#endif

#define OPAL_RECURSIVE_MUTEX_STATIC_INIT                                \
    {                                                                   \
        .super = OPAL_OBJ_STATIC_INIT(opal_mutex_t),                    \
        .m_lock_qthreads = 0,                         \
        .m_recursive = 1,                                               \
        .m_lock_atomic = OPAL_ATOMIC_LOCK_INIT,                         \
    }

/************************************************************************
 *
 * mutex operations (non-atomic versions)
 *
 ************************************************************************/

static inline void opal_mutex_create(struct opal_mutex_t *m) {
}

static inline int opal_mutex_trylock(opal_mutex_t *m)
{
    ensure_init_qthreads();

#if OPAL_ENABLE_DEBUG
    int ret = qthread_readFE_nb(NULL, &m->m_lock_qthreads);
    if (ret != 0) {
        errno = ret;
        opal_output(0, "opal_mutex_trylock() %d", ret);
    }
    return ret;
#else
    return qthread_readFE_nb(NULL, &m->m_lock_qthreads);
#endif
}

static inline void opal_mutex_lock(opal_mutex_t *m)
{
    ensure_init_qthreads();
#if OPAL_ENABLE_DEBUG
    int ret = qthread_lock(&m->m_lock_qthreads);
    if (ret != 0) {
        errno = ret;
        opal_output(0, "opal_mutex_lock() %d", ret);
    }
#else
    qthread_lock(&m->m_lock_qthreads);
#endif
}

static inline void opal_mutex_unlock(opal_mutex_t *m)
{
    ensure_init_qthreads();

#if OPAL_ENABLE_DEBUG
    int ret = qthread_unlock(&m->m_lock_qthreads);
    if (ret != 0) {
        errno = ret;
        opal_output(0, "opal_mutex_unlock() %d", ret);
    }
#else
    qthread_unlock(&m->m_lock_qthreads);
#endif
    /* For fairness of locking. */
    qthread_yield();
}

/************************************************************************
 *
 * mutex operations (atomic versions)
 *
 ************************************************************************/

#if OPAL_HAVE_ATOMIC_SPINLOCKS

/************************************************************************
 * Spin Locks
 ************************************************************************/

static inline int opal_mutex_atomic_trylock(opal_mutex_t *m)
{
    return opal_atomic_trylock(&m->m_lock_atomic);
}

static inline void opal_mutex_atomic_lock(opal_mutex_t *m)
{
    opal_atomic_lock(&m->m_lock_atomic);
}

static inline void opal_mutex_atomic_unlock(opal_mutex_t *m)
{
    opal_atomic_unlock(&m->m_lock_atomic);
}

#else

/************************************************************************
 * Standard locking
 ************************************************************************/

static inline int opal_mutex_atomic_trylock(opal_mutex_t *m)
{
    return opal_mutex_trylock(m);
}

static inline void opal_mutex_atomic_lock(opal_mutex_t *m)
{
    opal_mutex_lock(m);
}

static inline void opal_mutex_atomic_unlock(opal_mutex_t *m)
{
    opal_mutex_unlock(m);
}

#endif

typedef aligned_t* opal_cond_t;
#define OPAL_CONDITION_STATIC_INIT 0

static inline void opal_cond_create(opal_cond_t *cond) {
    ensure_init_qthreads();
}

static inline int opal_cond_init(opal_cond_t *cond) {
    return 0;
}

static inline int opal_cond_wait(opal_cond_t *cond, opal_mutex_t *lock) {
    ensure_init_qthreads();
    return qthread_readFE(*cond, &lock->m_lock_qthreads);
}

static inline int opal_cond_broadcast(opal_cond_t *cond) {
    ensure_init_qthreads();
    return qthread_fill(*cond);
}

static inline int opal_cond_signal(opal_cond_t *cond) {
    ensure_init_qthreads();
    return qthread_fill(*cond);
}

static inline int opal_cond_destroy(opal_cond_t *cond) {
    ensure_init_qthreads();
    return 0;
}

END_C_DECLS

#endif /* OPAL_MCA_THREADS_QTHREADS_THREADS_QTHREADS_MUTEX_H */
