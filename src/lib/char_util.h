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

#ifndef ASSETS_ARCHIVE_CHAR_UTIL
#define ASSETS_ARCHIVE_CHAR_UTIL

#include <string.h>

/**
 * @return A copy of given string
 * Caller must free result
 */
char *str_copy(const char *s, size_t len) {

    char *copy = malloc(len + 1);
    memset(copy, 0, len + 1);
    strncpy(copy, s, len);

    return copy;
}

/**
 * @return 1 if it is newline , 0 if not
 */
int is_newline(int c) {
    if (c == '\n' || c == '\r')
        return 1;
    return 0;
}

/**
 * @return 1 if it is whitespace , 0 if not
 */
int is_whitespace(int c) {
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
        return 1;
    return 0;
}

/**
 * @return 1 if s == NULL or *s == 0
 */
int is_null_or_empty(const char *s) {
    if (s == NULL || *s == 0)
        return 1;
    return 0;
}


#endif //ASSETS_ARCHIVE_CHAR_UTIL