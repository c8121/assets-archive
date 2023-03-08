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

#include "lib/util.h"

#include "lib/cli_args.h"
#include "lib/config_file.h"

#include "lib/archive_storage.h"
#include "lib/archive_metadata_json.h"

struct command {
    char *name;
    void *f;
};

void command_get(int argi, int argc, char *argv[]);

void command_add_origin(int argi, int argc, char *argv[]);

struct command commands[] = {
        {"get", &command_get},
        {"add-origin", &command_add_origin}
};
#define NUM_COMMANDS 2

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
    printf("%s <command> [<command>...] [options]\n", argv[0]);
    printf("\n");
    printf("Available commands:\n");
    printf("    get <hash>\n");
    printf("    add-origin <hash> <name> [-tags [tags,...]]\n");
    printf("\n");
    printf("Available options:\n");
    printf("    -config: File containing application configuration.\n");
    printf("    -help:   This message.\n");
}

/**
 *
 */
void *get_command(char *s) {

    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (is_equal(commands[i].name, s))
            return commands[i].f;
    }

    return NULL;
}

/**
 *
 */
void print_metadata(cJSON *metadata) {
    char *json = cJSON_Print(metadata);
    printf("META-DATA:\n%s\n\n", json);
    free(json);
}

/**
 *
 */
void command_get(int argi, int argc, char *argv[]) {

    if (argc < argi + 1)
        fail(EX_USAGE, "Missing hash");

    char *hash = argv[argi + 1];
    if (is_null_or_empty(hash))
        fail(EX_DATAERR, "Hash cannot be empty");

    char *file = archive_storage_find_file(hash);
    if (file == NULL)
        fail(EX_DATAERR, "No such file");
    free(file);

    cJSON *metadata = archive_metadata_json_open(hash);
    if (metadata == NULL)
        fail(EX_DATAERR, "Failed to load/create metadata JSON Object");

    print_metadata(metadata);

    cJSON_Delete(metadata);
}

/**
 *
 */
void command_add_origin(int argi, int argc, char *argv[]) {

    char *hash = NULL;
    cJSON *metadata = NULL;
    cJSON *origin = NULL;

    for (int i = argi + 1; i < argc; i++) {
        if (i == argi + 1) {

            hash = argv[i];
            if (is_null_or_empty(hash))
                fail(EX_DATAERR, "Hash cannot be empty");

            metadata = archive_metadata_json_open(hash);
            if (metadata == NULL)
                fail(EX_DATAERR, "Failed to load/create metadata JSON Object");

        } else if (i == argi + 2) {

            if (metadata != NULL) {

                if (is_null_or_empty(argv[i]))
                    fail(EX_DATAERR, "Name cannot be empty");

                origin = archive_metadata_json_get_origin(metadata, argv[i]);
                if (origin == NULL)
                    fail(EX_DATAERR, "Failed to get/create origin JSON Object");
            }


        } else if (is_equal("-tags", argv[i])) {

            if (origin != NULL) {

                cJSON *tags = archive_metadata_json_get_tags(origin);
                if (tags == NULL)
                    fail(EX_DATAERR, "Failed to get/create tags");

                for (i++; i < argc; i++) {
                    if (argv[i][0] == '-')
                        break; //next option
                    else if (get_command(argv[i]) != NULL)
                        goto end; //next command
                    archive_metadata_json_add_tag(tags, argv[i]);
                }
            }

        }
    }

    end:
    if (metadata != NULL) {
        print_metadata(metadata);
        archive_metadata_json_close(metadata, hash);
    }
}

/**
 * 
 */
int main(int argc, char *argv[]) {

    if (cli_has_opt("-help", argc, argv)) {
        usage_message(argc, argv);
        return EX_OK;
    }

    if (argc < 2) {
        usage_message(argc, argv);
        return EX_USAGE;
    }

    // Load & apply configuration
    int i = cli_get_opt_idx("-config", argc, argv);
    if (i > 0) {
        if (read_config_file(argv[i], &apply_config) == 0)
            fail(EX_IOERR, "Failed to read from config file");
    }

    // Check environment
    if (!archive_storage_validate())
        fail(EX_IOERR, "Archive storage not valid");

    // Execute commands
    void (*f)(int, int, char *[]) = NULL;
    for (int i = 1; i < argc; i++) {
        f = get_command(argv[i]);
        if (f != NULL) {
            f(i, argc, argv);
            f = NULL;
        }
    }
}
