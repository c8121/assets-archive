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

#include <stdio.h>
#include <sysexits.h>

#include "3rd/cjson/cJSON.h"
#include "3rd/cjson/cJSON.c"

#include "submodules/cutils/src/util.h"

#include "submodules/cutils/src/cli_args.h"
#include "submodules/cutils/src/db_util.h"
#include "submodules/cutils/src/config_file.h"

#include "lib/archive_metadata_json.h"
#include "lib/archive_metadata_db.h"
#include "lib/archive_metadata_db_mysql.h"


#define DEFAULT_CONFIG_FILE "./config/default-config"

struct db_config db;

/**
 *
 */
void apply_config(char *section_name, char *name, char *value) {
    apply_db_config(&db, "mysql", section_name, name, value);
}


/**
 *
 */
void usage_message(int argc, char *argv[]) {
    printf("USAGE:\n");
    printf("%s <hash>\n", argv[0]);
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

    char *hash = cli_get_arg(1, argc, argv);
    if (is_null_or_empty(hash))
        fail(EX_USAGE, "Hash cannot be empty");

    // Check environment
    if (!archive_storage_validate())
        fail(EX_IOERR, "Archive storage not valid");

    // Load & apply configuration
    memset(&db, 0, sizeof(struct db_config));
    read_config_file_from_cli_arg("-config", argc, argv, 1, DEFAULT_CONFIG_FILE,
                                  &apply_config);

    // Connect DB
    if (!archive_metadata_db_connect(db.host, db.user, db.password, db.db, db.port))
        return 0;
    memset(&db, 0, sizeof(struct db_config));


    cJSON *metadata = archive_metadata_json_open(hash);
    if (metadata == NULL)
        fail(EX_DATAERR, "Failed to load metadata JSON Object");

    cJSON *origins = archive_metadata_json_get_origins(metadata);
    if (origins == NULL)
        fail(EX_DATAERR, "Metadata does not contain origins");


    cJSON *origin;
    cJSON_ArrayForEach(origin, origins) {
        printf("Add meta data for %s\n", hash);
        if (!archive_metadata_db_add(hash, origin)) {
            fail(EX_DATAERR, "Failed to add origin");
        }
    }

    cJSON_Delete(metadata);

    archive_metadata_db_disconnect();
}
