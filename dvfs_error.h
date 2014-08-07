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
#define DVFS_SUCCESS 0                            /*!< No error */
#define DVFS_ERROR_INVALID_ARG -1                  /*!< Invalid argument passed to the function */
#define DVFS_ERROR_FILE_ERROR -2                   /*!< File error (use strerrno to have more information) */
#define DVFS_ERROR_MEM_ALLOC_FAILED -3             /*!< Memory allocation failed */
#define DVFS_ERROR_RELATED_CORE_UNAVAILABLE -4     /*!< Failed to get related core */
#define DVFS_ERROR_SEMAPHORE_FAILURE -5            /*!< Failure related to semaphore function */
#define DVFS_ERROR_BUFFER_TOO_SHORT -6             /*!< A buffer was not big enough for the operation */
#define DVFS_ERROR_SET_FREQ_FILE -7                /*!< File to set frequency is not available */
#define DVFS_ERROR_INVALID_FREQ_ID -8              /*!< The freq ID is not available */
#define DVFS_ERROR_INVALID_CORE_ID -9              /*!< The core ID is not available */
#define DVFS_ERROR_CORE_UNIT_MISMATCH -10          /*!< Core is not findable in this CPU  */
#define DVFS_ERROR_UNKNOWN -11                     /** Unknown error
                                                      (all greater error code results in this) */

/**
 * Returns a string describing the error number
 * @param id_error error number
 * @return a string describing the error. You don't have to free this.
 */
const char* dvfs_strerror(int id_error);
