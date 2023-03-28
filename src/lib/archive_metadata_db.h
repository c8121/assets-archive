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

#ifndef ASSETS_ARCHIVE_METADATA_DB
#define ASSETS_ARCHIVE_METADATA_DB

#include <stdio.h>

#ifndef cJSON__h

#include "cJSON/cJSON.h"
#include "cJSON/cJSON.c"

#endif

#include "cutils/src/char_util.h"
#include "archive_metadata_json.h"

#define MAX_LENGTH_SUBJECT 1024
#define MAX_LENGTH_PERSON_NAME 254
#define MAX_LENGTH_TAG_NAME 254
#define MAX_LENGTH_CATEGORY_NAME 254
#define MAX_LENGTH_CATEGORY_PATH 4096

#define CATEGORY_SEPARATOR '/'

/**
 * @return 1 on success, 0 otherwise
 */
int archive_metadata_db_connect(const char *host, const char *user, const char *pwd, const char *db, unsigned int port);

void archive_metadata_db_disconnect();

unsigned long archive_metadata_db_get_hash_id(const char *category);

unsigned long archive_metadata_db_get_category_id(const char *category);

unsigned long archive_metadata_db_get_person_id(const char *owner);

unsigned long archive_metadata_db_get_tag_id(const char *tag);

unsigned long archive_metadata_db_get_origin_id(unsigned long hash_id, const char *name, const char *subject,
                                                unsigned long owner_id, unsigned long category_id,
                                                const char *created, const char *changed);

unsigned long archive_metadata_db_assign_participant(unsigned long origin_id, unsigned long person_id);

unsigned long archive_metadata_db_assign_tag(unsigned long origin_id, unsigned long tag_id);

/**
 * @return 1 on success, 0 otherwise
 */
int archive_metadata_db_add(const char *hash, cJSON *origin) {

    if (origin == NULL) {
        fprintf(stderr, "Origin can not be NULL");
        return 0;
    }

    unsigned long hash_id = archive_metadata_db_get_hash_id(hash);

    char *name = archive_metadata_json_get_origin_name(origin);

    char *created = archive_metadata_json_get_created(origin);
    char *changed = archive_metadata_json_get_changed(origin);

    char *category = archive_metadata_json_get_category(origin);
    unsigned long category_id = !is_null_or_empty(category) ? archive_metadata_db_get_category_id(category) : 0;

    char *subject = archive_metadata_json_get_subject(origin);

    char *owner = archive_metadata_json_get_owner(origin);
    unsigned long owner_id = !is_null_or_empty(owner) ? archive_metadata_db_get_person_id(owner) : 0;

    unsigned long origin_id = archive_metadata_db_get_origin_id(hash_id, name, subject,
                                                                owner_id, category_id,
                                                                created, changed
    );

    cJSON *tags = archive_metadata_json_get_tags(origin);
    cJSON *tag;
    unsigned long tag_id;
    cJSON_ArrayForEach(tag, tags) {
        if (!is_null_or_empty(tag->valuestring)) {
            tag_id = archive_metadata_db_get_tag_id(tag->valuestring);
            archive_metadata_db_assign_tag(origin_id, tag_id);
        }
    }

    cJSON *participants = archive_metadata_json_get_participants(origin);
    cJSON *participant;
    unsigned long participant_id;
    cJSON_ArrayForEach(participant, participants) {
        if (!is_null_or_empty(participant->valuestring)) {
            participant_id = archive_metadata_db_get_person_id(participant->valuestring);
            archive_metadata_db_assign_participant(origin_id, participant_id);
        }
    }

    return 1;
}


#endif //ASSETS_ARCHIVE_METADATA_DB