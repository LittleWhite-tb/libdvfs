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

#include "error.h"

#include <stdlib.h>

#define NB_ERROR (DVFS_ERROR_UNKNOWN+1)

char* errors[NB_ERROR] =
{
    "No error", // Never move this. Index 0 is reserved !
    "Invalid argument (did you pass a NULL pointer?)",
    "File error",
    "Unknown error" // This is also reserved as the last error
};

const char* dvfs_strerrno(int errno)
{
    // All errors should be negative
    if (errno >= DVFS_ERROR_SUCCESS)
    {
        return errors[0];
    }

    int errorIndex = abs(errno);
    if ( errorIndex >= NB_ERROR  ) // This error code is not handle
                                    // Index out of bound
    {
        return errors[NB_ERROR-1];
    }

    return errors[errorIndex];
}
