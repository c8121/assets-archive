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
 * Create name for temporary file (within archive directory)
 */

#include <stdio.h>
#include <sysexits.h>

#include "cutils/src/util.h"

#include "cutils/src/cli_args.h"
#include "cutils/src/config_file.h"

#include "lib/archive_storage.h"

#define DEFAULT_CONFIG_FILE "../config/default-config"

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
    printf("%s [suffix]\n", argv[0]);
}

/**
 * 
 */
int main(int argc, char *argv[]) {

    // Load & apply configuration
    char *default_config_file = get_config_file_path(argc, argv, DEFAULT_CONFIG_FILE);
    if (!read_config_file_from_cli_arg("-config", argc, argv, 0, default_config_file, &apply_config))
        fail(EX_IOERR, "Failed to read from config file");

    printf("%s\n", archive_storage_tmpnam(argc > 1 ? argv[1] : ""));

}
