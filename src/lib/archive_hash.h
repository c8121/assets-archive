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

#ifndef ASSETS_ARCHIVE_HASH
#define ASSETS_ARCHIVE_HASH

#include "../../dep/cutils/src/command_util.h"
#include "../../dep/cutils/src/char_util.h"

char *archive_hash_program = "sha256sum -z \"{{file_name}}\"";

/**
 * Generate hash based on contents of given file.
 * Note: Caller must free result.
 */
char *archive_hash(char *file_name) {

    struct command_args *args = command_args_append(NULL, "file_name", file_name);
    char *command = command_build(archive_hash_program, args);

    char *ret = command_read(command);
    if (!is_null_or_empty(ret)) {
        //Hash program might send additional output, delimited by space: ignore this
        char *e = strchr(ret, ' ');
        if (e != NULL) {
            e[0] = '\0';
        }
    }


    free(command);
    command_args_free(args);

    if (is_null_or_empty(ret))
        fprintf(stderr, "archive_hash() failed for \"%s\"\n", file_name);

    return ret;
}

#endif //ASSETS_ARCHIVE_HASH