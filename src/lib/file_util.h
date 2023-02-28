/*
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

/*
 * Author: christian c8121 de
 */

#ifndef ASSETS_ARCHIVE_FILE_UTIL
#define ASSETS_ARCHIVE_FILE_UTIL

#include <sys/stat.h>

#include "char_util.h"

/**
 * @return 1 if it does exist and is a regular file, 0 if not
 */
int file_exists(const char *file_name) {
    if (is_null_or_empty(file_name))
        return 0;

    struct stat file_stat;
    if (stat(file_name, &file_stat) == 0) {
        if (S_ISREG(file_stat.st_mode))
            return 1;
    }
    return 0;
}

/**
 * @return 1 if it does exist and is a directory, 0 if not
 */
int dir_exists(const char *dir_name) {
    if (is_null_or_empty(dir_name))
        return 0;

    struct stat file_stat;
    if (stat(dir_name, &file_stat) == 0) {
        if (S_ISDIR(file_stat.st_mode))
            return 1;
    }
    return 0;
}

#endif //ASSETS_ARCHIVE_FILE_UTIL