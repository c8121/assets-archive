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
 * Write archived file content to stdout.
 * Find file by its content hash.
 */

#include <stdio.h>
#include <sysexits.h>

#include "cutils/src/util.h"

#include "cutils/src/cli_args.h"
#include "cutils/src/config_file.h"

#include "lib/archive_storage.h"

#define DEFAULT_CONFIG_FILE "../config/default-config"


char *filter_command_dir = "../filter";

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
    printf("%s <hash> [options]\n", argv[0]);
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
    char *default_config_file = get_config_file_path(argc, argv, DEFAULT_CONFIG_FILE);
    read_config_file_from_cli_arg("-config", argc, argv, 1, default_config_file, &apply_config);

    // Check environment
    if (!archive_storage_validate(1))
        fail(EX_IOERR, "Archive storage not valid");

    char *hash = cli_get_arg(1, argc, argv);
    if (is_null_or_empty(hash)) {
        usage_message(argc, argv);
        return EX_USAGE;
    }

    char *path = archive_storage_find_file(hash);
    if (is_null_or_empty(path))
        fail(EX_DATAERR, "No such file");


    if (!archive_storage_write_file(path, stdout))
        fail(EX_IOERR, "Failed to cat file");
}
