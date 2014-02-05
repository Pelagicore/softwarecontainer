/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DEBUG_H
#define DEBUG_H

#include "pelagicore-log.h"

/*! \brief Debug helpers
 *  \file debug.h
 */

LOG_IMPORT_DEFAULT_CONTEXT(Pelagicontain_DefaultLogContext)

#define debug(args ...) log_debug(args)
#define warning(args ...) log_warning(args)

#endif /* DEBUG_H */
