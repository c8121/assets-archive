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

#include "lib/cli_args.c"
#include "lib/config_file.c"

#include "lib/archive_hash.c"
#include "lib/archive_storage.c"
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

    // Get & check file name
    char *file_name = cli_get_arg(1, argc, argv);
    if (file_name == NULL) {
        fprintf(stderr, "Missing argument: file\n");
        usage_message(argc, argv);
        return EX_USAGE;
    } else if (!file_exists(file_name)) {
        fprintf(stderr, "File not found: %s\n", file_name);
        return EX_IOERR;
    }
    printf("FILE: %s\n", file_name);

    char *hash = archive_hash(file_name);
    printf("HASH: %s\n", hash);

    char *path = archive_storage_get_path(hash);
    printf("ARCHIVE-PATH: %s\n", path);
}
