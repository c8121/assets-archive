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

#include <stdio.h>

#ifndef cJSON__h

#include "cJSON/cJSON.h"
#include "cJSON/cJSON.c"

#endif

#include "cutils/src/char_util.h"
#include "cutils/src/char_buffer_util.h"
#include "cutils/src/file_util.h"
#include "cutils/src/time_util.h"

#include "archive_storage.h"

#define MAX_LENGTH_JSON_STRING 4096

#define MAX_LENGTH_ORIGIN_NAME 4096

#define JSON_FILE_READ_BUFFER_LENGTH 1024

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
        archive_metadata_json_suffix_len = strnlen(archive_metadata_json_suffix, MAX_LENGTH_ARCHIVE_FILENAME_SUFFIX);
}

/**
 *
 */
cJSON *__archive_metadata_json_get_array(cJSON *container, const char *name) {

    if (container == NULL)
        return NULL;

    cJSON *array = cJSON_GetObjectItem(container, name);
    if (array == NULL) {
        array = cJSON_AddArrayToObject(container, name);
        if (array == NULL) {
            fprintf(stderr, "Failed to create tags array\n");
            return NULL;
        }
    }

    return array;
}

/**
 *
 */
cJSON *__archive_metadata_json_add_string_to_array(cJSON *array, const char *s) {

    cJSON *s_json = cJSON_CreateString(s);
    if (s_json == NULL) {
        fprintf(stderr, "Failed to create string element\n");
        return NULL;
    }

    cJSON *existing;
    cJSON_ArrayForEach(existing, array)
    {
        if (cJSON_IsString(existing) && strncmp(existing->valuestring, s, MAX_LENGTH_JSON_STRING) == 0)
            return existing;
    }

    if (!cJSON_AddItemToArray(array, s_json)) {
        fprintf(stderr, "Failed to add tag\n");
        return NULL;
    }

    return s_json;
}

/**
 *
 */
char *__archive_metadata_json_get_string(cJSON *container, const char *name) {
    cJSON *json = cJSON_GetObjectItem(container, name);
    if (json == NULL)
        return NULL;
    return json->valuestring;
}

/**
 *
 */
cJSON *__archive_metadata_json_add_string(cJSON *container, const char *name, const char *s) {

    cJSON *s_json = cJSON_CreateString(s);
    if (s_json == NULL) {
        fprintf(stderr, "Failed to create string element\n");
        return NULL;
    }

    cJSON *owner = cJSON_GetObjectItem(container, name);
    if (owner != NULL)
        cJSON_DeleteItemFromObject(container, name);

    if (!cJSON_AddItemReferenceToObject(container, name, s_json)) {
        fprintf(stderr, "Failed to add string\n");
        return NULL;
    }

    return s_json;
}

/**
 * See archive_metadata_sjon_suffix.
 * Uses archive_storage_file_suffix
 *
 * Note: Caller must free result
 */
char *archive_metadata_json_get_path(char *hash) {

    __archive_metadata_json_init();

    //First attempt in current time period container (fast)
    char *file_name = archive_storage_get_path_with_suffix(
            hash,
            archive_metadata_json_suffix,
            archive_metadata_json_suffix_len
    );
    if (file_exists(file_name))
        return file_name;

    //Second search for existing file in all containers
    char *archive_file_name = archive_storage_find_file(hash);
    if (archive_file_name != NULL) {

        freenn(file_name);

        size_t len = strlen(archive_file_name);
        char *json_file_name = malloc(len + archive_metadata_json_suffix_len + 1);
        snprintf(json_file_name, len, "%s", archive_file_name);
        snprintf(json_file_name + (len - archive_storage_file_suffix_len), archive_metadata_json_suffix_len + 1, "%s",
                 archive_metadata_json_suffix);
        free(archive_file_name);

        file_name = json_file_name;
    }

    return file_name;
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

    char buf[JSON_FILE_READ_BUFFER_LENGTH];
    while (fgets(buf, JSON_FILE_READ_BUFFER_LENGTH, fp) != NULL) {
        cb = char_buffer_append(cb, buf, strnlen(buf, JSON_FILE_READ_BUFFER_LENGTH));
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

cJSON *archive_metadata_json_get_origins(cJSON *metadata_json) {

    cJSON *origins = cJSON_GetObjectItem(metadata_json, "origins");
    if (origins == NULL)
        return NULL;

    if (!cJSON_IsArray(origins)) {
        fprintf(stderr, "Invalid JSON (origins not an array)\n");
        return NULL;
    }

    return origins;
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
    cJSON_ArrayForEach(origin, origins)
    {
        cJSON *name_json = cJSON_GetObjectItem(origin, "name");
        if (cJSON_IsString(name_json) && strncmp(name_json->valuestring, name, MAX_LENGTH_ORIGIN_NAME) == 0) {
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
char *archive_metadata_json_get_origin_name(cJSON *origin) {
    return __archive_metadata_json_get_string(origin, "name");
}

/**
 *
 */
cJSON *archive_metadata_json_set_subject(cJSON *origin, const char *subject) {
    return __archive_metadata_json_add_string(origin, "subject", subject);
}

/**
 *
 */
char *archive_metadata_json_get_subject(cJSON *origin) {
    return __archive_metadata_json_get_string(origin, "subject");
}

/**
 *
 */
cJSON *archive_metadata_json_get_tags(cJSON *origin) {
    return __archive_metadata_json_get_array(origin, "tags");
}

/**
 *
 */
cJSON *archive_metadata_json_add_tag(cJSON *tags, const char *tag) {
    return __archive_metadata_json_add_string_to_array(tags, tag);
}

/**
 *
 */
cJSON *archive_metadata_json_set_created(cJSON *origin, const char *created) {
    char *ts = get_valid_time_string(created);
    if (ts == NULL)
        return NULL;

    cJSON *ret = __archive_metadata_json_add_string(origin, "created", ts);
    free(ts);

    return ret;
}

/**
 *
 */
char *archive_metadata_json_get_created(cJSON *origin) {
    return __archive_metadata_json_get_string(origin, "created");
}

/**
 *
 */
cJSON *archive_metadata_json_set_changed(cJSON *origin, const char *changed) {
    char *ts = get_valid_time_string(changed);
    if (ts == NULL)
        return NULL;

    cJSON *ret = __archive_metadata_json_add_string(origin, "changed", ts);
    free(ts);

    return ret;
}

/**
 *
 */
char *archive_metadata_json_get_changed(cJSON *origin) {
    return __archive_metadata_json_get_string(origin, "changed");
}

/**
 *
 */
cJSON *archive_metadata_json_set_category(cJSON *origin, const char *name) {
    return __archive_metadata_json_add_string(origin, "category", name);
}

/**
 *
 */
char *archive_metadata_json_get_category(cJSON *origin) {
    return __archive_metadata_json_get_string(origin, "category");
}

/**
 *
 */
cJSON *archive_metadata_json_set_owner(cJSON *origin, const char *name) {
    return __archive_metadata_json_add_string(origin, "owner", name);
}

/**
 *
 */
char *archive_metadata_json_get_owner(cJSON *origin) {
    return __archive_metadata_json_get_string(origin, "owner");
}

/**
 *
 */
cJSON *archive_metadata_json_get_participants(cJSON *origin) {
    return __archive_metadata_json_get_array(origin, "participants");
}

/**
 *
 */
cJSON *archive_metadata_json_add_participant(cJSON *participants, const char *name) {
    return __archive_metadata_json_add_string_to_array(participants, name);
}

/**
 *
 */
void archive_metadata_print(cJSON *metadata) {
    if (metadata == NULL) {
        fprintf(stderr, "Error in archive_metadata_print: metadata is null\n");
        return;
    }
    char *json = cJSON_Print(metadata);
    printf("META-DATA:\n%s\n\n", json);
    free(json);
}

#endif //ASSETS_ARCHIVE_METADATA_JSON