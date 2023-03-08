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

#ifndef cJSON__h

#include <stdio.h>

#include "../3rd/cjson/cJSON.h"
#include "../3rd/cjson/cJSON.c"

#endif

#include "char_util.h"
#include "char_buffer_util.h"
#include "file_util.h"
#include "archive_storage.h"

#define DEFAULT_ARCHIVE_METADATA_SUFFIX ".json"

char *archive_metadata_json_suffix = NULL;
int archive_metadata_json_suffix_len = -1;

/**
 *
 */
void __archive_metadata_json_init() {

    if (archive_metadata_json_suffix == NULL)
        archive_metadata_json_suffix = str_copy(DEFAULT_ARCHIVE_METADATA_SUFFIX,
                                                strlen(DEFAULT_ARCHIVE_METADATA_SUFFIX));
    if (archive_metadata_json_suffix_len == -1)
        archive_metadata_json_suffix_len = strnlen(archive_metadata_json_suffix, 255);
}

/**
 * See archive_metadata_sjon_suffix.
 * Uses archive_storage_file_suffix
 *
 * Note: Caller must free result
 */
char *archive_metadata_json_get_path(char *hash) {

    __archive_metadata_json_init();

    return archive_storage_get_path_with_suffix(
            hash,
            archive_metadata_json_suffix,
            archive_metadata_json_suffix_len
    );
}


/**
 * Load JSON metadata from given file_name
 */
cJSON *archive_metadata_json_load(char *file_name) {

    if (!file_exists(file_name)) {
        fprintf(stderr, "Metadata file does not exist: %s\n", file_name);
        return NULL;
    }

    struct char_buffer *cb = NULL;
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open metadata file for reading: %s\n", file_name);
        return NULL;
    }

    char buf[1024];
    while (fgets(buf, 1024, fp) != NULL) {
        cb = char_buffer_append(cb, buf, strnlen(buf, 1024));
    }

    fclose(fp);

    char *json = char_buffer_copy(cb);
    char_buffer_free(cb);

    cJSON *ret = cJSON_Parse(json);
    free(json);

    return ret;
}

/**
 * @return 1 on success, 0 on fail
 */
int archive_metadata_json_write(cJSON *metadata_json, char *file_name) {

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open metadata file for writing: %s\n", file_name);
        return 0;
    }

    char *json = cJSON_Print(metadata_json);
    fputs(json, fp);
    free(json);

    fclose(fp);

    return 1;
}

/**
 * Load metadata (if exists), otherwise create empty JSON object.
 */
cJSON *archive_metadata_json_open(char *hash) {

    char *file_name = archive_metadata_json_get_path(hash);
    if (file_name == NULL)
        return NULL;

    cJSON *metadata_json = archive_metadata_json_load(file_name);
    if (metadata_json == NULL)
        metadata_json = cJSON_CreateObject();

    return metadata_json;
}

/**
 * Save metadata an free memory
 */
void archive_metadata_json_close(cJSON *metadata_json, char *hash) {

    char *file_name = archive_metadata_json_get_path(hash);

    if (file_name != NULL)
        archive_metadata_json_write(metadata_json, file_name);
    else
        fprintf(stderr, "Cannot save metadata\n");

    if (metadata_json != NULL) {
        cJSON_Delete(metadata_json);
        metadata_json = NULL;
    }
}

/**
 *
 */
cJSON *archive_metadata_json_get_origin(cJSON *metadata_json, const char *name) {

    if (is_null_or_empty(name))
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

    cJSON *existing;
    cJSON_ArrayForEach(existing, tags) {
        if (cJSON_IsString(existing) && strncmp(existing->valuestring, tag, 1024) == 0)
            return existing;
    }

    if (!cJSON_AddItemToArray(tags, tag_json)) {
        fprintf(stderr, "Failed to add tag\n");
        return NULL;
    }

    return tag_json;
}

#endif //ASSETS_ARCHIVE_METADATA_JSON