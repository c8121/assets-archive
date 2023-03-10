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

#include "lib/archive_metadata_json.h"
#include "lib/archive_metadata_db.h"
#include "lib/archive_metadata_db_mysql.h"

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
    printf("TODO: %s\n", argv[0]);
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

    char *hash = cli_get_arg(1, argc, argv);
    if (is_null_or_empty(hash))
        fail(EX_USAGE, "Hash cannot be empty");

    cJSON *metadata = archive_metadata_json_open(hash);
    if (metadata == NULL)
        fail(EX_DATAERR, "Failed to load metadata JSON Object");

    cJSON *origins = archive_metadata_json_get_origins(metadata);
    if (origins == NULL)
        fail(EX_DATAERR, "Metadata does not contain origins");

    if (!archive_metadata_db_connect(
            "localhost",
            "Test",
            "Test",
            "Test",
            3306
    )) {
        return 0;
    }

    cJSON *origin;
    cJSON_ArrayForEach(origin, origins) {
        if (!archive_metadata_db_add(hash, origin)) {
            fail(EX_DATAERR, "Failed to add origin");
        }
    }

    cJSON_Delete(metadata);

    archive_metadata_db_disconnect();
}
