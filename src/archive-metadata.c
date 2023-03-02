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
#include "lib/archive_metadata_json.h"

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
    printf("%s <hash> <command> [options]\n", argv[0]);
    printf("\n");
    printf("Available commands:\n");
    printf("    add-origin <name> [tags,...]\n");
    printf("\n");
    printf("Available options:\n");
    printf("    -config: File containing application configuration.\n");
    printf("    -help:   This message.\n");
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
        if (read_config_file(argv[i], &apply_config) == 0)
            fail(EX_IOERR, "Failed to read from config file\n");
    }

    // Check environment
    if (!archive_storage_validate())
        fail(EX_IOERR, "Archive storage not valid\n");

    char *hash = cli_get_arg(1, argc, argv);
    char *command = cli_get_arg(2, argc, argv);
    char *name = cli_get_arg(3, argc, argv);

    if (!is_null_or_empty(hash) && !is_null_or_empty(command) && !is_null_or_empty(name)) {

        char *existing = archive_storage_find_file(hash);
        if (existing != NULL) {

            printf("File: %s\n", existing);

            if (strcasecmp(command, "add-origin") == 0) {

                cJSON *origin = archive_metadata_json_get_origin(name);
                if (origin == NULL)
                    fail(EX_DATAERR, "Failed to get/create JSON Object");

                cJSON *tags = archive_metadata_json_get_tags(origin);
                if (tags == NULL)
                    fail(EX_DATAERR, "Failed to get/create tags");

                for (int i = 4; i < argc; i++) {
                    if (argv[i][0] == '-')
                        break;
                    archive_metadata_json_add_tag(tags, argv[i]);
                }

                char *json = cJSON_Print(archive_metadata_json_get());
                printf("META-DATA:\n%s\n\n", json);
                free(json);

                archive_metadata_json_close();

            } else {
                fprintf(stderr, "Unknown command: '%s'\n", command);
            }

            free(existing);
        } else {
            fprintf(stderr, "Referenced file not found: %s\n", hash);
        }
    } else {
        usage_message(argc, argv);
        return EX_USAGE;
    }
}
