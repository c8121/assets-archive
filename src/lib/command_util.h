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

#ifndef ASSETS_ARCHIVE_COMMAND_UTIL
#define ASSETS_ARCHIVE_COMMAND_UTIL

#include "char_util.h"
#include "char_buffer_util.h"

#define COMMAND_READ_BUFFER_LENGTH 512

struct command_args {
    char *name;
    char *value;
    struct command_args *next;
};


/**
 * @param args Can be NULL
 */
struct command_args *command_args_append(struct command_args *args, const char *name, char *value) {

    struct command_args *ret = malloc(sizeof(struct command_args));
    if (args == NULL) {
        ret->next = NULL;
    } else {
        ret->next = args;
    }

    ret->name = str_copy(name, strlen(name));
    ret->value = str_copy(value, strlen(value));
    return ret;

}

/**
 *
 */
char *command_args_get(char *name, struct command_args *args) {

    struct command_args *a = args;
    while (a != NULL) {
        if (strcmp(name, a->name) == 0)
            return a->value;
        a = a->next;
    }
    return NULL;

}

void command_args_free(struct command_args *args) {

    struct command_args *a = args;
    struct command_args *f;
    while (a != NULL) {
        f = a;
        a = a->next;

        free(f->name);
        free(f->value);
        free(f);
    }
}

/**
 * Execute command and return output
 *
 * @return Output of command.
 * Note: Caller must free result;
 */
char *command_read(char *command) {

    FILE *cmd = popen(command, "r");
    if (cmd == NULL) {
        fprintf(stderr, "Failed to execute: %s\n", command);
        return NULL;
    }

    struct char_buffer *cb = NULL;

    char line[COMMAND_READ_BUFFER_LENGTH];
    while (fgets(line, COMMAND_READ_BUFFER_LENGTH, cmd)) {
        cb = char_buffer_append(cb, line, strnlen(line, COMMAND_READ_BUFFER_LENGTH));
    }

    if (feof(cmd)) {
        pclose(cmd);
    } else {
        fprintf(stderr, "Broken pipe: %s\n", command);
    }

    char *s = char_buffer_copy(cb);
    char_buffer_free(cb);

    return s;
}

/**
 * Take all names arguments in insert the values in to given command.
 *
 * Example:
 * command = "cat {{file_name}}"
 *
 * @return Parsed command
 * Note: Caller must free result
 */
char *command_build(char *command, struct command_args *args) {

    struct char_buffer *cb = NULL;

    char *s = command;
    char *t = command + strnlen(command, 4096);

    while (s < t) {
        char *b = strstr(s, "{{");
        if (b != NULL) {

            cb = char_buffer_append(cb, s, b - s);

            char *e = strstr(b, "}}");
            if (e != NULL) {

                size_t len = e - b;
                char name[len];
                strncpy(name, b + 2, len);
                name[len - 2] = '\0';
                char *value = command_args_get(name, args);

                if (value != NULL) {
                    cb = char_buffer_append(cb, value, strlen(value));
                }

                s = e + 2;
            } else {
                cb = char_buffer_append(cb, b, b - s);
                break;
            }
        } else {
            cb = char_buffer_append(cb, s, t - s);
            break;
        }
    }

    char *c = char_buffer_copy(cb);
    char_buffer_free(cb);

    return c;
}


#endif //ASSETS_ARCHIVE_COMMAND_UTIL