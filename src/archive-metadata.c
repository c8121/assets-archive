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

#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>

#include "3rd/cjson/cJSON.h"
#include "3rd/cjson/cJSON.c"

#include "lib/cli_args.h"
#include "lib/config_file.h"

#include "lib/archive_storage.h"

/**
 *
 */
void apply_config(char *section_name, char *name, char *value) {
    printf("No yet done: Apply section='%s', name='%s', value='%s'\n",
           section_name, name, value);
}


/**
 *
 */
void usage_message(int argc, char *argv[]) {
    printf("USAGE:\n");
    printf("%s <hash> [options]\n", argv[0]);
    printf("\n");
    printf("Available options:\n");
    printf("    -origin <name>    : Set name of file origin (path for example).\n");
    printf("    -tags <tag[ ...]> : Tags.\n");
}

void fail(int status, char *message) {
    fprintf(stderr, "%s\n", message);
    exit(status);
}

/**
 * 
 */
int main(int argc, char *argv[]) {

    if (cli_has_opt("-help", argc, argv)) {
        usage_message(argc, argv);
        return EX_OK;
    }

    // Load & apply configuration
    int i = cli_get_opt_idx("-config", argc, argv);
    if (i > 0) {
        if (read_config_file(argv[i], &apply_config) == 0) {
            fprintf(stderr, "Failed to read from config file: %s\n", argv[i]);
            exit(EX_IOERR);
        }
    }

    // Check environment
    if (!archive_storage_validate()) {
        fprintf(stderr, "Archive storage not valid\n");
        exit(EX_IOERR);
    }

    char *hash = cli_get_arg(1, argc, argv);
    if (!is_null_or_empty(hash)) {
        char *existing = archive_storage_find_file(hash);
        if (existing != NULL) {

            printf("File: %s\n", existing);

            cJSON *metadata = cJSON_CreateObject();
            if (metadata == NULL)
                fail(EX_DATAERR, "Failed to create JSON Object");

            i = cli_get_opt_idx("-origin", argc, argv);
            if (i > 0) {
                if (cJSON_AddStringToObject(metadata, "origin", argv[i]) == NULL)
                    fail(EX_DATAERR, "Failed to add origin");
            }

            i = cli_get_opt_idx("-tag", argc, argv);
            if (i > 0) {
                cJSON *tags = cJSON_AddArrayToObject(metadata, "tags");
                if (tags == NULL)
                    fail(EX_DATAERR, "Failed to add tags");
                for (; i < argc; i++) {
                    if (argv[i][0] == '-')
                        break;

                    cJSON *tag = cJSON_CreateString(argv[i]);
                    if (tag == NULL)
                        fail(EX_DATAERR, "Failed to create tag item");
                    cJSON_AddItemToArray(tags, tag);

                }
            }

            char *json = cJSON_Print(metadata);
            printf("META-DATA:\n%s\n\n", json);

            free(json);
            cJSON_Delete(metadata);

            free(existing);
        } else {
            fprintf(stderr, "Referenced file not found: %s\n", hash);
        }
    } else {
        usage_message(argc, argv);
        return EX_USAGE;
    }
}
