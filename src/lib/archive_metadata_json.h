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

#ifndef ASSETS_ARCHIVE_METADATA_JSON
#define ASSETS_ARCHIVE_METADATA_JSON

#include "char_util.h"

#ifndef cJSON__h

#include "../3rd/cjson/cJSON.h"
#include "../3rd/cjson/cJSON.c"

#endif


cJSON *metadata_json = NULL;

/**
 *
 */
cJSON *archive_metadata_json_get() {

    if (metadata_json != NULL)
        return metadata_json;

    metadata_json = cJSON_CreateObject();

    return metadata_json;
}

/**
 *
 */
void archive_metadata_json_close() {
    if (metadata_json != NULL) {
        cJSON_Delete(metadata_json);
        metadata_json = NULL;
    }
}

/**
 *
 */
cJSON *archive_metadata_json_get_origin(const char *name) {

    if (is_null_or_empty(name))
        return NULL;

    if (archive_metadata_json_get() == NULL)
        return NULL;

    cJSON *origins = cJSON_GetObjectItem(metadata_json, "origins");
    if (origins == NULL) {
        origins = cJSON_AddArrayToObject(metadata_json, "origins");
        if (origins == NULL) {
            fprintf(stderr, "Failed to create origins\n");
            return NULL;
        }
    }

    if (!cJSON_IsArray(origins)) {
        fprintf(stderr, "Invalid JSON (origins not an array)\n");
        return NULL;
    }

    cJSON *origin;

    //Find origin by name
    cJSON_ArrayForEach(origin, origins) {
        cJSON *name_json = cJSON_GetObjectItem(origin, "name");
        if (cJSON_IsString(name_json) && strncmp(name_json->valuestring, name, 2048) == 0) {
            return origin;
        }
    }

    //Not found, create
    origin = cJSON_CreateObject();
    if (origin == NULL) {
        fprintf(stderr, "Failed to create origin\n");
        return NULL;
    }

    if (!cJSON_AddItemToArray(origins, origin)) {
        fprintf(stderr, "Failed to add origin\n");
        return NULL;
    }

    if (cJSON_AddStringToObject(origin, "name", name) == NULL) {
        fprintf(stderr, "Failed to ad origin name\n");
        return NULL;
    }

    return origin;
}

/**
 *
 */
cJSON *archive_metadata_json_get_tags(cJSON *origin) {

    if (origin == NULL)
        return NULL;

    cJSON *tags = cJSON_GetObjectItem(origin, "tags");
    if (tags == NULL) {
        tags = cJSON_AddArrayToObject(origin, "tags");
        if (tags == NULL) {
            fprintf(stderr, "Failed to create tags array\n");
            return NULL;
        }
    }

    return tags;
}

/**
 *
 */
cJSON *archive_metadata_json_add_tag(cJSON *tags, const char *tag) {

    cJSON *tag_json = cJSON_CreateString(tag);
    if (tag_json == NULL) {
        fprintf(stderr, "Failed to create tag\n");
        return NULL;
    }

    if (!cJSON_AddItemToArray(tags, tag_json)) {
        fprintf(stderr, "Failed to add tag\n");
        return NULL;
    }

    return tag_json;
}

#endif //ASSETS_ARCHIVE_METADATA_JSON