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
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */


#ifndef OPAL_MCA_THREADS_PTHREADS_THREADS_PTHREADS_TSD_H
#define OPAL_MCA_THREADS_PTHREADS_THREADS_PTHREADS_TSD_H

#include <pthread.h>
#include <signal.h>

typedef pthread_key_t opal_tsd_key_t;

static inline int opal_tsd_key_delete(opal_tsd_key_t key)
{
    return pthread_key_delete(key);
}

static inline int opal_tsd_setspecific(opal_tsd_key_t key, void *value)
{
    return pthread_setspecific(key, value);
}

static inline int opal_tsd_getspecific(opal_tsd_key_t key, void **valuep)
{
    *valuep = pthread_getspecific(key);
    return OPAL_SUCCESS;
}

#endif /* OPAL_MCA_THREADS_PTHREADS_THREADS_PTHREADS_TSD_H */
