/*  =========================================================================
    fty-common-logging - Provides common logs

    Copyright (C) 2014 - 2018 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef FTY_LOG_H_H_INCLUDED
#define FTY_LOG_H_H_INCLUDED


#ifdef __cplusplus
//  log4cplus
#include <log4cplus/logger.h>
#include "fty_common_logging_library.h"

#else
//  log4cplus
#include <log4cplus/clogger.h>
#include "fty-log/fty_logger.h"

#endif

#endif
