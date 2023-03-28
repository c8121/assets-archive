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

#ifndef ASSETS_ARCHIVE_STORAGE
#define ASSETS_ARCHIVE_STORAGE

#include <stdlib.h>
#include <bsd/stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "../../dep/cutils/src/char_buffer_util.h"
#include "../../dep/cutils/src/file_util.h"
#include "../../dep/cutils/src/command_util.h"

#define MAX_LENGTH_HASH 254
#define SPLIT_HASH_AT 2

#define MAX_LENGTH_TIME_PERIOD 20
#define MAX_LENGTH_STORAGE_BASE_DIR 254
#define MAX_LENGTH_ARCHIVE_FILENAME 254
#define MAX_LENGTH_ARCHIVE_FILENAME_SUFFIX 20

#define COPY_BUFFER_SIZE 4096

#define DEFAULT_STORAGE_BASE_DIR "/tmp"
#define DEFAULT_STORAGE_FILE_SUFFIX ".archive"

char *archive_storage_base_dir = NULL;
int archive_storage_base_dir_len = -1;
char *archive_storage_file_suffix = NULL;
int archive_storage_file_suffix_len = -1;


char *archive_storage_file_dencode_keyfile = "";
char *archive_storage_file_encode_command = "gzip -v -c -n \"{{input_file}}\" > \"{{output_file}}\"";
char *archive_storage_file_decode_command = "gzip -v -c -d \"{{input_file}}\" > \"{{output_file}}\"";
//char *archive_storage_file_dencode_keyfile = "/tmp/dencode.key";
//char *archive_storage_file_encode_command = "gzip -v -c -n \"{{input_file}}\" | openssl enc -aes-256-cbc -pbkdf2 -pass \"file:{{key_file}}\" -e > \"{{output_file}}\"";
//char *archive_storage_file_decode_command = "openssl enc -aes-256-cbc -pbkdf2 -pass \"file:{{key_file}}\" -d -in \"{{input_file}}\" | gzip -v -c -d  > \"{{output_file}}\"";



const char *path_separator = "/";
const int path_separator_len = 1;

const char *archive_storage_temp_dirname = "tmp";
const int archive_storage_temp_dirname_len = 3;

mode_t archive_file_permissions = S_IRUSR | S_IWUSR; //rw-------
mode_t archive_dir_permissions = S_IRUSR | S_IWUSR | S_IXUSR; //rwx------

/**
 *
 */
void __archive_storage_init() {

    if (archive_storage_base_dir == NULL)
        archive_storage_base_dir = str_copy(DEFAULT_STORAGE_BASE_DIR, strlen(DEFAULT_STORAGE_BASE_DIR));
    if (archive_storage_base_dir_len == -1)
        archive_storage_base_dir_len = strnlen(archive_storage_base_dir, MAX_LENGTH_STORAGE_BASE_DIR);

    if (archive_storage_file_suffix == NULL)
        archive_storage_file_suffix = str_copy(DEFAULT_STORAGE_FILE_SUFFIX, strlen(DEFAULT_STORAGE_FILE_SUFFIX));
    if (archive_storage_file_suffix_len == -1)
        archive_storage_file_suffix_len = strnlen(archive_storage_file_suffix, MAX_LENGTH_ARCHIVE_FILENAME_SUFFIX);

    //Create seed for rand() used in archive_storage_tmpnam.
    srand(time(NULL));
}

/**
 * @return 1 on success, 0 on fail
 */
int __archive_storage_copy(const char *src, const char *dst) {

    int in = open(src, O_RDONLY);
    if (in == -1) {
        fprintf(stderr, "Failed to open file for reading: %s\n", src);
        return 0;
    }

    int out = open(dst, O_CREAT | O_WRONLY | O_TRUNC, archive_file_permissions);
    if (out == -1) {
        fprintf(stderr, "Failed to open file for writing: %s\n", dst);
        return 0;
    }

    char buf[COPY_BUFFER_SIZE];
    int r;
    while ((r = read(in, buf, COPY_BUFFER_SIZE)) > 0) {
        if (write(out, buf, r) != r) {
            fprintf(stderr, "write() returned error or incomplete write\n");
            break;
        }
    }

    if (close(out) == -1) {
        fprintf(stderr, "Failed to close output file: %s\n", dst);
        return 0;
    }

    if (close(in) == -1) {
        fprintf(stderr, "Failed to close input file: %s\n", dst);
        return 0;
    }

    return 1;
}

/**
 * Decode or encode file
 *
 * @return 1 on success, 0 on fail
 */
int __archive_storage_copy_dencode(const char *src, const char *dst, char *dencode_command) {

    if (dencode_command != archive_storage_file_encode_command
        && dencode_command != archive_storage_file_decode_command) {
        fprintf(stderr, "Invalid command, must be either '%s' or '%s'\n", archive_storage_file_encode_command,
                archive_storage_file_decode_command);
        return 0;
    }

    struct command_args *args = command_args_append(NULL, "input_file", src);
    args = command_args_append(args, "output_file", dst);

    if (!is_null_or_empty(archive_storage_file_dencode_keyfile))
        args = command_args_append(args, "key_file", archive_storage_file_dencode_keyfile);

    char *command = command_build(dencode_command, args);

    int ret = 0;
    char *command_output = command_read(command);
    if (command_output != NULL) {
        ret = 1;
        if (command_output[0] != '\0')
            fprintf(stderr, "%s", command_output);
        free(command_output);
    }

    free(command);
    command_args_free(args);

    return ret;
}


/**
 * Check if storage is valid (directory exists etc.)
 *
 * @return 1 if valid, 0 if not.
 */
int archive_storage_validate() {

    __archive_storage_init();

    if (!dir_exists(archive_storage_base_dir)) {
        fprintf(stderr, "Archive base directory does not exist: %s\n", archive_storage_base_dir);
        return 0;
    }

    if (access(archive_storage_base_dir, W_OK) != 0) {
        fprintf(stderr, "Cannot write to archive base: %s\n", archive_storage_base_dir);
        return 0;
    }

    return 1;
}


/**
 * Get a name reflecting a period of time.
 *
 * Note: Caller must free result
 */
char *archive_storage_time_period_name(time_t t) {

    char *name = malloc(MAX_LENGTH_TIME_PERIOD);
    sprintf(name, "%lx", t / 60 / 60 / 24 / 4);

    return name;
}

/**
 * Get a path in form:
 *
 * /archive_storage_base_dir/time-period/hash(0,2)/hash(2...).suffix
 *
 * Note: Caller must free result
 */
char *archive_storage_get_path_with_suffix(char *hash, char *suffix, int suffix_len) {

    __archive_storage_init();

    if (strnlen(hash, MAX_LENGTH_HASH) <= SPLIT_HASH_AT) {
        fprintf(stderr, "Invalid hash length\n");
        return NULL;
    }

    struct char_buffer *cb = NULL;

    char *period_name = archive_storage_time_period_name(time(NULL));

    cb = char_buffer_append(cb, archive_storage_base_dir, archive_storage_base_dir_len);
    cb = char_buffer_append(cb, path_separator, path_separator_len);
    cb = char_buffer_append(cb, period_name, strnlen(period_name, MAX_LENGTH_TIME_PERIOD));
    cb = char_buffer_append(cb, path_separator, path_separator_len);

    cb = char_buffer_append(cb, hash, SPLIT_HASH_AT);
    cb = char_buffer_append(cb, path_separator, path_separator_len);
    cb = char_buffer_append(cb, hash + SPLIT_HASH_AT, strnlen(hash, MAX_LENGTH_HASH) - SPLIT_HASH_AT);

    if (!is_null_or_empty(suffix))
        cb = char_buffer_append(cb, suffix, suffix_len);

    char *ret = char_buffer_copy(cb);
    char_buffer_free(cb);
    free(period_name);

    return ret;
}

/**
 * See archive_storage_get_path_with_suffix.
 * Uses archive_storage_file_suffix
 *
 * Note: Caller must free result
 */
char *archive_storage_get_path(char *hash) {

    return archive_storage_get_path_with_suffix(
            hash,
            archive_storage_file_suffix,
            archive_storage_file_suffix_len
    );
}

/**
 *
 */
int archive_storage_mkdir(const char *file_path) {

    __archive_storage_init();

    if (strstr(file_path, archive_storage_base_dir) != file_path) {
        fprintf(stderr, "Invalid file path (not within %s): %s\n", archive_storage_base_dir, file_path);
        return 0;
    }

    char *p = (char *) file_path + archive_storage_base_dir_len + 1;
    int ret;
    while ((p = strstr(p, path_separator)) != NULL) {

        *p = 0;
        if (!dir_exists(file_path)) {
            //printf("Create %s\n", file_path);
            ret = mkdir(file_path, archive_dir_permissions);
        } else {
            ret = 0;
        }
        *p = path_separator[0];

        if (ret == -1) {
            fprintf(stderr, "mkdir() failed: %s\n", file_path);
            return 0;
        }

        p++;
    }

    return 1;
}


/**
 * @return 1 on success, 0 on fail
 */
int archive_storage_add_file(const char *src, const char *dst) {

    if (strstr(dst, archive_storage_base_dir) != dst) {
        fprintf(stderr, "Invalid destination path (not within %s): %s\n", archive_storage_base_dir, dst);
        return 0;
    }

    if (!archive_storage_mkdir(dst)) {
        fprintf(stderr, "Failed to create directory for %s\n", dst);
        return 0;
    }

    if (!is_null_or_empty(archive_storage_file_encode_command)) {
        return __archive_storage_copy_dencode(src, dst, archive_storage_file_encode_command);
    } else {
        return __archive_storage_copy(src, dst);
    }
}

/**
 * Look for a file with given hash, return path.
 *
 * @return Path name if exists, NULL if not exists
 * Note: Caller must free result
 */
char *archive_storage_find_file(char *hash) {

    __archive_storage_init();

    if (strnlen(hash, MAX_LENGTH_HASH) <= SPLIT_HASH_AT) {
        fprintf(stderr, "Invalid hash length\n");
        return NULL;
    }

    char subdir[SPLIT_HASH_AT + 1];
    strncpy(subdir, hash, SPLIT_HASH_AT);

    DIR *d = opendir(archive_storage_base_dir);
    if (d == NULL) {
        fprintf(stderr, "Failed to open archive base directory: \"%s\"\n", archive_storage_base_dir);
        return NULL;
    }

    struct char_buffer *cb = NULL;
    char *path = NULL;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {

        if (e->d_name[0] == '\0' || strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;

        cb = char_buffer_append(cb, archive_storage_base_dir,
                                strnlen(archive_storage_base_dir, MAX_LENGTH_STORAGE_BASE_DIR));
        cb = char_buffer_append(cb, path_separator, path_separator_len);
        cb = char_buffer_append(cb, e->d_name, strnlen(e->d_name, MAX_LENGTH_ARCHIVE_FILENAME));
        cb = char_buffer_append(cb, path_separator, path_separator_len);

        cb = char_buffer_append(cb, subdir, SPLIT_HASH_AT);
        cb = char_buffer_append(cb, path_separator, path_separator_len);
        cb = char_buffer_append(cb, hash + SPLIT_HASH_AT, strnlen(hash, MAX_LENGTH_HASH) - SPLIT_HASH_AT);
        cb = char_buffer_append(cb, archive_storage_file_suffix, archive_storage_file_suffix_len);

        path = char_buffer_copy(cb);
        char_buffer_free(cb);
        cb = NULL;

        if (file_exists(path))
            break;

        free(path);
        path = NULL;
    }

    closedir(d);

    return path;
}

/**
 * @return 1 on success, 0 on fail
 */
int archive_storage_copy_file(const char *src, const char *dst) {

    if (strstr(src, archive_storage_base_dir) != src) {
        fprintf(stderr, "Invalid source path (not within %s): %s\n", archive_storage_base_dir, src);
        return 0;
    }

    if (!is_null_or_empty(archive_storage_file_decode_command)) {
        return __archive_storage_copy_dencode(src, dst, archive_storage_file_decode_command);
    } else {
        return __archive_storage_copy(src, dst);
    }
}

/**
 * @return temp file name
 * Note: Caller must free result
 */
char *archive_storage_tmpnam(const char *suffix) {

    __archive_storage_init();

    for (int i = 0; i < 1000; i++) {

        struct char_buffer *cb = NULL;
        cb = char_buffer_append(cb, archive_storage_base_dir,
                                strnlen(archive_storage_base_dir, MAX_LENGTH_STORAGE_BASE_DIR));
        cb = char_buffer_append(cb, path_separator, path_separator_len);
        cb = char_buffer_append(cb, archive_storage_temp_dirname, archive_storage_temp_dirname_len);
        cb = char_buffer_append(cb, path_separator, path_separator_len);

        char file_name[MAX_LENGTH_ARCHIVE_FILENAME];
        snprintf(file_name, MAX_LENGTH_ARCHIVE_FILENAME, "%jd-%i-%X%s", time(NULL), rand(), arc4random(), suffix);
        cb = char_buffer_append(cb, file_name, strnlen(file_name, MAX_LENGTH_ARCHIVE_FILENAME));

        char *path = char_buffer_copy(cb);
        char_buffer_free(cb);
        cb = NULL;

        if (!archive_storage_mkdir(path)) {
            fprintf(stderr, "Failed to create directory for %s\n", path);
            return NULL;
        }

        if (!file_exists(path) && !dir_exists(path)) {
            return path;
        }

        usleep(100);
    }

    fprintf(stderr, "Cannot create temp filename\n");
    return NULL;
}

/**
 * @return 1 on success, 0 on fail
 */
int archive_storage_write_file(const char *src, FILE *out) {

    if (strstr(src, archive_storage_base_dir) != src) {
        fprintf(stderr, "Invalid source path (not within %s): %s\n", archive_storage_base_dir, src);
        return 0;
    }

    char *tmp = archive_storage_tmpnam(".tmp");
    if (tmp == NULL)
        return 0;

    if (!archive_storage_copy_file(src, tmp))
        return 0;

    FILE *fp = fopen(tmp, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open temp-file: %s\n", tmp);
        return 0;
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        fputc(c, out);
    }

    fclose(fp);

    unlink(tmp);
    free(tmp);

    return 1;
}

#endif //ASSETS_ARCHIVE_STORAGE