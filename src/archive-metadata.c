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

//to enable strcasestr, strptime, must be before including string.h
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>

#include "cJSON/cJSON.h"
#include "cJSON/cJSON.c"

#include "cutils/src/util.h"

#include "cutils/src/cli_args.h"
#include "cutils/src/config_file.h"

#include "lib/archive_storage.h"
#include "lib/archive_metadata_json.h"

#define DEFAULT_CONFIG_FILE "./config/default-config"

struct command {
    char *name;
    void *f;
};

void command_get(int argi, int argc, char *argv[]);

void command_add_origin(int argi, int argc, char *argv[]);

struct command commands[] = {
        {"get",        &command_get},
        {"add-origin", &command_add_origin}
};
#define NUM_COMMANDS 2

/**
 *
 */
void apply_config(char *section_name, char *name, char *value) {
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
    printf("    add-origin <hash> <name> \\\n");
    printf("         [-subject <subject>] \\\n");
    printf("         [-tags [<tag>,...]] \\\n");
    printf("         [-created <yyyy-mm-ss hh:mm:ss>] \\\n");
    printf("         [-changed <yyyy-mm-ss hh:mm:ss>] \\\n");
    printf("         [-category <path/name>] \\\n");
    printf("         [-owner <name>] \\\n");
    printf("         [-participants [<name>,...]]\n");
    printf("\n");
    printf("Available options:\n");
    printf("    -config: File containing application configuration.\n");
    printf("    -help:   This message.\n");
}

/**
 *
 */
void *command_get_function(char *s) {

    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (is_equal(commands[i].name, s))
            return commands[i].f;
    }

    return NULL;
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

    archive_metadata_print(metadata);

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
                    if (argv[i][0] == '-') {
                        i--;
                        break; //next option
                    } else if (command_get_function(argv[i]) != NULL)
                        goto end; //next command
                    archive_metadata_json_add_tag(tags, argv[i]);
                }
            }

        } else if (is_equal("-created", argv[i])) {

            if (++i < argc) {
                if (origin != NULL) {
                    archive_metadata_json_set_created(origin, argv[i]);
                }
            }

        } else if (is_equal("-changed", argv[i])) {

            if (++i < argc) {
                if (origin != NULL) {
                    archive_metadata_json_set_changed(origin, argv[i]);
                }
            }

        } else if (is_equal("-category", argv[i])) {

            if (++i < argc) {
                if (origin != NULL) {
                    archive_metadata_json_set_category(origin, argv[i]);
                }
            }

        } else if (is_equal("-owner", argv[i])) {

            if (++i < argc) {
                if (origin != NULL) {
                    archive_metadata_json_set_owner(origin, argv[i]);
                }
            }

        } else if (is_equal("-subject", argv[i])) {

            if (++i < argc) {
                if (origin != NULL) {
                    archive_metadata_json_set_subject(origin, argv[i]);
                }
            }

        } else if (is_equal("-participants", argv[i])) {

            if (origin != NULL) {

                cJSON *participants = archive_metadata_json_get_participants(origin);
                if (participants == NULL)
                    fail(EX_DATAERR, "Failed to get/create participants");

                for (i++; i < argc; i++) {
                    if (argv[i][0] == '-') {
                        i--;
                        break; //next option
                    } else if (command_get_function(argv[i]) != NULL)
                        goto end; //next command
                    archive_metadata_json_add_participant(participants, argv[i]);
                }
            }
        }
    }

    end:
    if (metadata != NULL) {
        archive_metadata_print(metadata);
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
    read_config_file_from_cli_arg("-config", argc, argv, 1, DEFAULT_CONFIG_FILE,
                                  &apply_config);

    // Check environment
    if (!archive_storage_validate())
        fail(EX_IOERR, "Archive storage not valid");

    // Execute commands
    void (*f)(int, int, char *[]) = NULL;
    for (int i = 1; i < argc; i++) {
        f = command_get_function(argv[i]);
        if (f != NULL) {
            f(i, argc, argv);
            f = NULL;
        }
    }
}
