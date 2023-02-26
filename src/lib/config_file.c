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

#ifndef ASSETS_ARCHIVE_CONFIG_FILE
#define ASSETS_ARCHIVE_CONFIG_FILE

#include <stdio.h>
#include <string.h>

#include "file_util.c"
#include "char_util.c"


#define MAX_LEN_SECTION_NAME 512
#define MAX_LEN_NAME 512
#define MAX_LEN_VALUE 4096

/**
 * Read config file like this:
 *
 * <code><pre>
 * [Section]
 * name: value
 * name: value
 *
 * (...)
 * </pre></code>
 *
 * Note: Use quotation for multiline-values of to keep whitespace at beginning of values.
 *
 * @return 1 on success, 0 on failure
 */
int read_config_file(char *file_name, void (*apply_config_function)(char *section_name, char *name, char *value)) {

    if (file_exists(file_name) == 0)
        return 0;

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open config-file: %s\n", file_name);
        return 0;
    }

    char section_name[MAX_LEN_SECTION_NAME];
    memset(section_name, 0, MAX_LEN_SECTION_NAME);

    char name[MAX_LEN_NAME];
    memset(name, 0, MAX_LEN_NAME);

    char value[MAX_LEN_VALUE];
    memset(value, 0, MAX_LEN_VALUE);

    int pos = 1; //0 = section, 1 = name, 2 = value
    char *p = name;
    int last_was_newline = 0;
    int last_was_escape = 0;
    int in_quote = 0;
    int ignore = 0;

    int c;
    int l = 0;
    while ((c = fgetc(fp)) != EOF) {

        //Keep quoted string as they are
        if (c == '"') {
            if (in_quote) {
                if (!last_was_escape) {
                    in_quote = 0;
                    continue;
                } else
                    *(p--) = c; //Overwrite escape char
            } else {
                in_quote = 1;
                continue;
            }
        } else if (in_quote) {
            last_was_escape = c == '\\'; //Only check for escaped quote-char
            *(p++) = c;
            continue;
        }

        if ((l == 0 || last_was_newline) && c == '#') {
            ignore = 1;
            continue; //Begin of commented out line
        }

        if (ignore) {
            if (is_newline(c)) { //Ignore until next line
                last_was_newline = 0;
                ignore = 0;
            }
            continue;
        }

        if (c == '[' && last_was_newline) {
            //Begin of section
            pos = 0;
            p = section_name;
            memset(section_name, 0, MAX_LEN_SECTION_NAME);
        } else if (c == ']' && pos == 0) {
            //End of section, begin of name
            pos = 1;
            p = name;
            memset(name, 0, MAX_LEN_NAME);
        } else if (c == ':' && pos == 1) {
            //End of name, begin of value
            pos = 2;
            p = value;
            memset(value, 0, MAX_LEN_VALUE);
        } else if (is_newline(c) && pos == 2) {
            //End of value, begin of next name
            if (*name != 0 && *value != 0)
                apply_config_function(section_name, name, value);
            pos = 1;
            p = name;
            memset(name, 0, MAX_LEN_NAME);
        } else {
            if (is_whitespace(c)) {
                if (pos == 2 && p != value) //Ignore whitespace in names and begin of values
                    *(p++) = c;
            } else {
                *(p++) = c;
            }
        }

        last_was_newline = is_newline(c);
        l++;
    }

    if (*name != 0 && *value != 0)
        apply_config_function(section_name, name, value);

    fclose(fp);

    return 1;
}


#endif //ASSETS_ARCHIVE_CONFIG_FILE