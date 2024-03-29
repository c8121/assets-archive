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
 *
 * Add file to archive directory.
 * List contents of archive directory
 */

#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>

#include "cutils/src/util.h"

#include "cutils/src/cli_args.h"
#include "cutils/src/config_file.h"

#include "lib/archive_hash.h"
#include "lib/archive_storage.h"

#define DEFAULT_CONFIG_FILE "../config/default-config"

int print_hash_only = 0;

/**
 *
 */
void apply_config(char *section_name, char *name, char *value) {
    apply_archive_storage_config("archive", section_name, name, value);
}


/**
 *
 */
void usage_message(int argc, char *argv[]) {
    printf("USAGE:\n");
    printf("\n");
    printf("Add file to archive:\n");
    printf("%s [options] <file>\n", argv[0]);
    printf("\n");
    printf("List hashes:\n");
    printf("%s list\n", argv[0]);
    printf("\n");
    printf("General available options:\n");
    printf("    -config: File containing application configuration.\n");
    printf("    -help:   This message.\n");
    printf("    -s:      Print hash only or 'FAIL' on error to stdout.\n");
}

/**
 *
 */
void list_hash(const char *dir, const char *hash) {
    if (print_hash_only)
        printf("%s\n", hash);
    else
        printf("%s %s\n", hash, dir);
}

/**
 * 
 */
int main(int argc, char *argv[]) {

    if (cli_has_opt("-help", argc, argv)) {
        usage_message(argc, argv);
        return EX_OK;
    }

    print_hash_only = cli_has_opt("-s", argc, argv);

    // Load & apply configuration
    char *default_config_file = get_config_file_path(argc, argv, DEFAULT_CONFIG_FILE);
    if (!read_config_file_from_cli_arg("-config", argc, argv, 0, default_config_file, &apply_config)) {
        if (print_hash_only)
            printf("FAIL\n");
        fail(EX_IOERR, "Failed to read from config file");
    }

    // Check environment
    if (!archive_storage_validate(0)) {
        if (print_hash_only)
            printf("FAIL\n");
        fail(EX_IOERR, "Archive storage not valid");
    }

    // Check if there is at least one file or command, show usage otherwise
    if (argc < 2) {
        usage_message(argc, argv);
        return EX_USAGE;
    }

    if (is_equal(argv[1], "list")) {
        //List hashes
        archive_storage_list_hashes(&list_hash);
    } else {
        // Add files
        int f = 1;
        char *file_name;
        while ((file_name = cli_get_arg(f++, argc, argv)) != NULL) {
            if (!file_exists(file_name)) {
                if (print_hash_only)
                    printf("FAIL\n");
                fail(EX_IOERR, "File not found");
            }
            if (!print_hash_only)
                printf("Source: %s\n", file_name);

            char *hash = archive_hash(file_name);
            if (!is_null_or_empty(hash)) {
                if (!print_hash_only)
                    printf("Hash: %s\n", hash);

                char *existing = archive_storage_find_file(hash);
                if (existing == NULL) {
                    char *path = archive_storage_get_path(hash);
                    archive_storage_add_file(file_name, path);
                    if (!print_hash_only)
                        printf("Added file to archive: %s\n", path);
                    else
                        printf("%s\n", hash);
                } else {
                    if (!print_hash_only)
                        printf("File already exists in archive: %s\n", existing);
                    else
                        printf("%s\n", hash);
                    free(existing);
                }
            } else {
                if (print_hash_only)
                    printf("FAIL\n");
                fail(EX_IOERR, "Failed to obtain hash");
            }

            if (hash != NULL)
                free(hash);
        }
    }
}
