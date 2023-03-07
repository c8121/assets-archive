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

#ifndef ASSETS_ARCHIVE_CHAR_BUFFER_UTIL
#define ASSETS_ARCHIVE_CHAR_BUFFER_UTIL

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CHAR_BUFFER_INITIAL_FACTOR 4

struct char_buffer {
    char *s;
    size_t len;
    size_t avail;
    struct char_buffer *curr; //where to append next chunk
    struct char_buffer *next;
};

/**
 *
 */
struct char_buffer *char_buffer_create(const char *s, size_t len) {

    struct char_buffer *b = malloc(sizeof(struct char_buffer));
    b->len = len;
    b->avail = len * CHAR_BUFFER_INITIAL_FACTOR;
    b->s = malloc(b->avail);
    memset(b->s, 0, b->avail);
    b->next = NULL;
    b->curr = b;

    strncpy(b->s, s, len);
    b->avail -= len;

    return b;
}


/**
 * Append s to cb.
 *
 * @param cb Can be NULL.
 * @param len Length of s including terminator.
 * @return Pointer to cb (will be created if NULL given)
 */
struct char_buffer *char_buffer_append(struct char_buffer *cb, const char *s, size_t len) {

    if (is_null_or_empty(s) || len == 0)
        return cb;

    struct char_buffer *b;

    if (cb == NULL) {

        b = char_buffer_create(s, len);

    } else {

        b = cb->curr;

        if (b->avail >= len + 1) { ;
            strncpy(b->s + b->len, s, len);
            b->avail -= len;
            b->len += len;
        } else {
            b->next = char_buffer_create(s, len);
            cb->curr = b->next;
        }

        b = cb;
    }

    return b;
}

/**
 * @return New allocated pointer containing content of cb.
 * Note: Caller must free result.
 */
char *char_buffer_copy(struct char_buffer *cb) {

    if (cb == NULL) {
        char *s = malloc(1);
        *s = '\0';
        return s;
    }

    size_t len = 0;
    struct char_buffer *b = cb;
    while (b != NULL) {
        len += b->len;
        b = b->next;
    }
    len += 1;

    char *s = malloc(len + 1);
    char *p = s;
    b = cb;
    while (b != NULL) {
        strncpy(p, b->s, b->len);
        p += b->len;
        *p = '\0';
        b = b->next;
    }

    return s;
}

/**
 *
 */
void char_buffer_free(struct char_buffer *cb) {

    struct char_buffer *b = cb;
    struct char_buffer *f;
    while (b != NULL) {
        f = b;
        b = b->next;

        free(f->s);
        free(f);
    }

}


#endif //ASSETS_ARCHIVE_CHAR_BUFFER_UTIL