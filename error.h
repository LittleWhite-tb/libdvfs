/*
 * libdvfs - A light library to set CPU governor and frequency
 * Copyright (C) 2013-2014 Universite de Versailles
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "dvfs_core.h"

// Error code definition
#define DVFS_ERROR_SUCCESS 0        /*!< No error */
#define DVFS_ERROR_INVALID_ARG 1    /*!< Invalid argument passed to the function */
#define DVFS_ERROR_FILE_ERROR 2     /*!< File error (use strerrno to have more information) */
#define DVFS_ERROR_UNKNOWN 3        /** Unknown error
                                        (all greater error code results in this) */

/**
 * Returns a string describing the error number
 * @param errno error number
 * @return a string describing the error. You don't have to free this.
 */
const char* dvfs_strerrno(int errno);
