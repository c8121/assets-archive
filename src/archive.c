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

#include "lib/cli_args.h"
#include "lib/config_file.h"

#include "lib/archive_hash.h"
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
    printf("%s [options] <file>\n", argv[0]);
    printf("\n");
    printf("Available options:\n");
    printf("    -config: File containing application configuration.\n");
    printf("    -help:   This message.\n");
    printf("    -s:      Print hash only or 'FAIL' on error to stdout.\n");
}

/**
 * 
 */
int main(int argc, char *argv[]) {

    if (cli_has_opt("-help", argc, argv)) {
        usage_message(argc, argv);
        return EX_OK;
    }

    int print_hash_only = cli_has_opt("-s", argc, argv);

    // Load & apply configuration
    int i = cli_get_opt_idx("-config", argc, argv);
    if (i > 0) {
        if (read_config_file(argv[i], &apply_config) == 0) {
            fprintf(stderr, "Failed to read from config file: %s\n", argv[i]);
            if (print_hash_only)
                printf("FAIL\n");
            exit(EX_IOERR);
        }
    }

    // Check environment
    if (!archive_storage_validate()) {
        fprintf(stderr, "Archive storage not valid\n");
        if (print_hash_only)
            printf("FAIL\n");
        exit(EX_IOERR);
    }

    // Add files
    int f = 1;
    char *file_name;
    while ((file_name = cli_get_arg(f++, argc, argv)) != NULL) {
        if (!file_exists(file_name)) {
            fprintf(stderr, "File not found: %s\n", file_name);
            if (print_hash_only)
                printf("FAIL\n");
            return EX_IOERR;
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
            fprintf(stderr, "Failed to obtain hash for %s\n", file_name);
            if (print_hash_only)
                printf("FAIL\n");
        }

        if (hash != NULL)
            free(hash);
    }
}
