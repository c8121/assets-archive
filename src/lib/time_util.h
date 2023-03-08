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

#ifndef ASSETS_ARCHIVE_TIME_UTIL
#define ASSETS_ARCHIVE_TIME_UTIL

#include "char_util.h"

/**
 * Check if given string is a valid time format.
 *
 * Valid formats:
 *    yyyy-mm-ss hh:mm:ss
 *    now
 *
 * @return 1 if valid
 */
int is_valid_time_string(const char *time_string) {
    if (is_null_or_empty(time_string))
        return 0;

    if (is_equal("now", time_string))
        return 1;

    if (strnlen(time_string, 19) < 19) {
        fprintf(stderr, "Invalid time format, expected \"yyyy-mm-ss hh:mm:ss\": \"%s\"\n", time_string);
        return 0;
    }

    return 1;
}

/**
 * Take time string or shortcuts and convert them to "yyyy-mm-ss hh:mm:ss"
 *
 * Available shortcuts:
 *    now
 *
 * @return String on success, NULL if given time string could not be interpreted
 * Caller must free result
 */
char *get_valid_time_string(const char *time_string) {

    if (!is_valid_time_string(time_string))
        return NULL;

    char *s = malloc(20);

    if (is_equal("now", time_string)) {

        time_t secs = time(NULL);
        struct tm *local = localtime(&secs);
        sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                local->tm_hour, local->tm_min, local->tm_sec);

    } else {

        memset(s, 0, 20);
        strncpy(s, time_string, 19);

    }

    return s;

}

#endif //ASSETS_ARCHIVE_TIME_UTIL