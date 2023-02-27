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

#ifndef ASSETS_ARCHIVE_STORAGE
#define ASSETS_ARCHIVE_STORAGE

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "char_buffer_util.h"
#include "file_util.h"

char *archive_storage_base_dir = "/tmp";
char *archive_storage_file_suffix = ".archive";

const char *path_separator = "/";
const int path_separator_len = 1;

/**
 * Check if storage is valid (directory exists etc.)
 *
 * @return 1 if valid, 0 if not.
 */
int archive_storage_validate() {

    if (!dir_exists(archive_storage_base_dir)) {
        fprintf(stderr, "Archive base directory does not exist: %s\n", archive_storage_base_dir);
        return 0;
    }

    if (access(archive_storage_base_dir, W_OK) != 0) {
        fprintf(stderr, "Cannot write to archive base: %s\n", archive_storage_base_dir);
        return 0;
    }

    return 1;
}


/**
 * Get a name reflecting a period of time.
 *
 * Note: Caller must free result
 */
char *archive_storage_time_period_name(time_t t) {

    char *name = malloc(20);
    sprintf(name, "%lx", t / 60 / 60 / 24 / 4);

    return name;
}

/**
 * Get a path in form:
 *
 * /archive_storage_base_dir/time-period/hash(0,2)/hash(2...)
 *
 * Note: Caller must free result
 */
char *archive_storage_get_path(char *hash) {

    struct char_buffer *cb = NULL;

    char *period_name = archive_storage_time_period_name(time(NULL));

    cb = char_buffer_append(cb, archive_storage_base_dir, strnlen(archive_storage_base_dir, 255));
    cb = char_buffer_append(cb, path_separator, path_separator_len);
    cb = char_buffer_append(cb, period_name, strnlen(period_name, 20));
    cb = char_buffer_append(cb, path_separator, path_separator_len);

    cb = char_buffer_append(cb, hash, 2);
    cb = char_buffer_append(cb, path_separator, path_separator_len);
    cb = char_buffer_append(cb, hash + 2, strnlen(hash, 255) - 2);
    cb = char_buffer_append(cb, archive_storage_file_suffix, strnlen(archive_storage_file_suffix, 255));

    char *ret = char_buffer_copy(cb);
    char_buffer_free(cb);
    free(period_name);

    return ret;
}

#endif //ASSETS_ARCHIVE_STORAGE