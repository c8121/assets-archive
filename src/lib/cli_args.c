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

#ifndef ASSETS_ARCHIVE_CLI_ARGS
#define ASSETS_ARCHIVE_CLI_ARGS

#include <string.h>

#define CLI_ARGS_MAX_NAME_LENGTH 128

/**
 * Check if option exists (argv-item with given name exists).
 *
 * @return 1 if name exists, 0 if not
 */
int cli_has_opt(char *name, int argc, char *argv[]) {

    size_t l = strnlen(name, CLI_ARGS_MAX_NAME_LENGTH);
    for (int i = 0; i < argc; i++) {
        if (strncmp(name, argv[i], l) == 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * Get index of option (index of argv-item preceded by argv-item with given name).
 *
 * @return index of value, 0 if name was not found
 */
int cli_get_opt_idx(char *name, int argc, char *argv[]) {

    size_t l = strnlen(name, CLI_ARGS_MAX_NAME_LENGTH);
    for (int i = 0; i < argc - 1; i++) {
        if (strncmp(name, argv[i], l) == 0) {
            return i + 1;
        }
    }

    return 0;
}

/**
 * Gets a pointer to non-option value (not preceded by a option name)
 */
char *cli_get_arg(int num, int argc, char *argv[]) {

    int idx = -1;
    int pos = 1; //0 = option name, 1 = option value, 2 = argument
    for (int i = 0; i < argc; i++) {

        if (argv[i][0] == '-') {
            pos = 0;
        } else {
            if (pos == 0)
                pos = 1;
            else
                pos = 2;
        }

        if (pos == 2) {
            idx++;
            if (idx == num)
                return argv[i];
        }
    }

    return NULL;
}


#endif //ASSETS_ARCHIVE_CLI_ARGS