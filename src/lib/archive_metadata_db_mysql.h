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

#ifndef ASSETS_ARCHIVE_METADATA_DB_MYSQL
#define ASSETS_ARCHIVE_METADATA_DB_MYSQL

#include <stdio.h>
#include <sysexits.h>

#include <mysql/mysql.h>

#include "util.h"
#include "char_util.h"

void *mysql = NULL;

const char *mysql_select_hash_id_sql = "SELECT ID FROM HASH WHERE HASH=?;";
MYSQL_STMT *mysql_select_hash_id_stmt = NULL;


/**
 *
 */
MYSQL_STMT *mysql_prepare_stmt(MYSQL_STMT *stmt, const char *sql) {

    if (mysql == NULL) {
        fprintf(stderr, "%s", "Not connected\n");
        return NULL;
    }

    if (stmt == NULL) {
        stmt = mysql_stmt_init(mysql);
        if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0) {
            fprintf(stderr, "%s\n", mysql_error(mysql));
            return NULL;
        }
    }

    return stmt;
}

/**
 *
 */
MYSQL_BIND *mysql_create_string_param(char *s, unsigned long l) {

    MYSQL_BIND *param = malloc(sizeof(MYSQL_BIND));
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = s;
    param[0].length = &l;
    param[0].is_null = 0;

    return param;
}

/**
 *
 */
MYSQL_STMT *mysql_close_stmt(MYSQL_STMT *stmt) {
    if (stmt != NULL)
        mysql_stmt_close(stmt);
    stmt = NULL;
    return NULL;
}

/**
 *
 */
int archive_metadata_db_connect(const char *host, const char *user, const char *pwd,
                                const char *db, unsigned int port) {

    if (mysql != NULL)
        return 0;

    mysql = mysql_init(NULL);
    if (mysql == NULL)
        fprintf(stderr, "Init failed: %s", mysql_error(mysql));

    if (mysql_real_connect(mysql, host, user, pwd, db, port, NULL, 0) == 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        mysql_close(mysql);
        return 0;
    }

    return 1;
}

/**
 *
 */
void archive_metadata_db_disconnect() {

    mysql_select_hash_id_stmt = mysql_close_stmt(mysql_select_hash_id_stmt);

}

/**
 *
 */
unsigned long archive_metadata_db_get_hash_id(const char *hash) {

    if ((mysql_select_hash_id_stmt = mysql_prepare_stmt(mysql_select_hash_id_stmt, mysql_select_hash_id_sql)) == NULL)
        fail(EX_IOERR, "Failed to prepare hash statement");

    MYSQL_BIND *p = mysql_create_string_param((char *) hash, strnlen(hash, 512));

    if (mysql_stmt_bind_param(mysql_select_hash_id_stmt, p) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 0;
    }

    printf("QUERY RESULT...\n");

    free(p);

    return 0;
}

/**
 *
 */
unsigned long archive_metadata_db_get_owner_id(const char *owner) {
    return -1;
}

/**
 *
 */
unsigned long archive_metadata_db_get_category_id(const char *category) {
    return -2;
}

/**
 *
 */
unsigned long archive_metadata_db_get_tag_id(const char *tag) {
    return -3;
}

/**
 *
 */
unsigned long archive_metadata_db_get_participant_id(const char *participant) {
    return -4;
}

#endif //ASSETS_ARCHIVE_METADATA_DB_MYSQL