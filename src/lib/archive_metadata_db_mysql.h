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

const char *mysql_insert_hash_sql = "INSERT INTO HASH(HASH) VALUES(?);";
MYSQL_STMT *mysql_insert_hash_stmt = NULL;

const char *mysql_select_person_id_sql = "SELECT ID FROM PERSON WHERE NAME=?;";
MYSQL_STMT *mysql_select_person_id_stmt = NULL;

const char *mysql_insert_person_sql = "INSERT INTO PERSON(NAME) VALUES(?);";
MYSQL_STMT *mysql_insert_person_stmt = NULL;

const char *mysql_select_tag_id_sql = "SELECT ID FROM TAG WHERE TAG=?;";
MYSQL_STMT *mysql_select_tag_id_stmt = NULL;

const char *mysql_insert_tag_sql = "INSERT INTO TAG(TAG) VALUES(?);";
MYSQL_STMT *mysql_insert_tag_stmt = NULL;

const char *mysql_select_category_id_sql = "SELECT ID FROM CATEGORY WHERE PARENT=? AND NAME=?;";
MYSQL_STMT *mysql_select_category_id_stmt = NULL;

const char *mysql_insert_category_sql = "INSERT INTO CATEGORY(PARENT, NAME) VALUES(?, ?);";
MYSQL_STMT *mysql_insert_category_stmt = NULL;


/**
 * @return 1 on success, 0 otherwise
 */
int __mysql_prepare_stmt(MYSQL_STMT **stmt, const char *sql) {

    if (mysql == NULL) {
        fprintf(stderr, "%s", "Not connected\n");
        return 0;
    }

    if (*stmt == NULL) {
        *stmt = mysql_stmt_init(mysql);
        if (mysql_stmt_prepare(*stmt, sql, strlen(sql)) != 0) {
            fprintf(stderr, "%s\n", mysql_error(mysql));
            return 0;
        }
    }

    return 1;
}

/**
 *
 */
MYSQL_BIND *__mysql_create_bind(enum enum_field_types type, void *value) {

    MYSQL_BIND *bind = malloc(sizeof(MYSQL_BIND));
    memset(bind, 0, sizeof(MYSQL_BIND));
    bind->buffer_type = type;
    bind->buffer = value;
    bind->is_null = 0;

    return bind;
}

/**
 *
 */
MYSQL_BIND *__mysql_create_binds(int count) {

    MYSQL_BIND *bind = malloc(sizeof(MYSQL_BIND) * count);
    memset(bind, 0, sizeof(MYSQL_BIND) * count);

    for (int i = 0; i < count; i++) {
        bind[i].is_null = 0;
    }

    return bind;
}


/**
 * @return -1 on fail, 0 on no data found, 1 on success
 */
int __mysql_fetch_result(MYSQL_STMT *stmt, MYSQL_BIND *param, MYSQL_BIND *result) {

    if (mysql_stmt_bind_param(stmt, param) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return -1;
    }

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return -1;
    }

    if (mysql_stmt_store_result(stmt) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return -1;
    }

    int ret = 1;
    int s = mysql_stmt_fetch(stmt);
    if (s == MYSQL_NO_DATA) {
        ret = 0;
    } else if (s != 0) {
        fprintf(stderr, "mysql_stmt_fetch = %i, %s\n", s, mysql_error(mysql));
    }

    return ret;
}

/**
 * @return -1 on fail, insert id otherwise
 */
unsigned long __mysql_execute(MYSQL_STMT *stmt, MYSQL_BIND *param) {

    if (mysql_stmt_bind_param(stmt, param) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return -1;
    }

    return mysql_stmt_insert_id(stmt);
}

/**
 * @return id on success, -1 on fail
 */
unsigned long __mysql_add_name(const char *name, int len, MYSQL_STMT **stmt, const char *sql) {

    if (!__mysql_prepare_stmt(stmt, sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_bind(MYSQL_TYPE_STRING, (void *) name);
    p->buffer_length = len;

    unsigned long id = __mysql_execute(*stmt, p);
    if (id == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(*stmt);
    free(p);

    return id;
}

/**
 * @return id if found, -1 on error, 0 if no data found
 */
unsigned long __mysql_get_name_id(const char *name, int len, MYSQL_STMT **stmt, const char *sql) {

    if (!__mysql_prepare_stmt(stmt, sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_bind(MYSQL_TYPE_STRING, (void *) name);
    p->buffer_length = len;

    unsigned long id = 0;
    MYSQL_BIND *r = __mysql_create_bind(MYSQL_TYPE_LONG, &id);

    int ret = __mysql_fetch_result(*stmt, p, r);
    if (ret == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(*stmt);
    free(r);
    free(p);

    if (ret == 0)
        return 0;

    return id;
}

/**
 * @return id on success, -1 on fail
 */
unsigned long __mysql_add_tree_item(unsigned long parent, const char *name, int len,
                                    MYSQL_STMT **stmt, const char *sql) {

    if (!__mysql_prepare_stmt(stmt, sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &parent;
    p[1].buffer_type = MYSQL_TYPE_STRING;
    p[1].buffer = (char *) name;
    p[1].buffer_length = len;

    unsigned long id = __mysql_execute(*stmt, p);
    if (id == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(*stmt);
    free(p);

    return id;
}

/**
 * @return id if found, -1 on error, 0 if no data found
 */
unsigned long __mysql_get_tree_item_id(unsigned long parent, const char *name, int len,
                                       MYSQL_STMT **stmt, const char *sql) {

    if (!__mysql_prepare_stmt(stmt, sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &parent;
    p[1].buffer_type = MYSQL_TYPE_STRING;
    p[1].buffer = (char *) name;
    p[1].buffer_length = len;

    unsigned long id = 0;
    MYSQL_BIND *r = __mysql_create_bind(MYSQL_TYPE_LONG, &id);

    int ret = __mysql_fetch_result(*stmt, p, r);
    if (ret == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(*stmt);
    free(r);
    free(p);

    if (ret == 0)
        return 0;

    return id;
}

/**
 *
 */
void __mysql_close_stmt(MYSQL_STMT **stmt) {
    if (*stmt != NULL)
        mysql_stmt_close(*stmt);
    *stmt = NULL;
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

    __mysql_close_stmt(&mysql_select_hash_id_stmt);
    __mysql_close_stmt(&mysql_insert_hash_stmt);
    __mysql_close_stmt(&mysql_select_person_id_stmt);
    __mysql_close_stmt(&mysql_insert_person_stmt);
    __mysql_close_stmt(&mysql_select_tag_id_stmt);
    __mysql_close_stmt(&mysql_insert_tag_stmt);

    mysql_close(mysql);
}


/**
 *
 */
unsigned long archive_metadata_db_get_hash_id(const char *hash) {

    if (is_null_or_empty(hash))
        fail(EX_DATAERR, "Owner name cannot be null or empty");

    int len = strnlen(hash, 512);

    unsigned long id = __mysql_get_name_id(hash, len,
                                           &mysql_select_hash_id_stmt, mysql_select_hash_id_sql);
    if (id == 0)
        id = __mysql_add_name(hash, len,
                              &mysql_insert_hash_stmt, mysql_insert_hash_sql);

    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_get_person_id(const char *owner) {

    if (is_null_or_empty(owner))
        fail(EX_DATAERR, "Owner name cannot be null or empty");

    int len = strnlen(owner, 512);

    unsigned long id = __mysql_get_name_id(owner, len,
                                           &mysql_select_person_id_stmt, mysql_select_person_id_sql);
    if (id == 0)
        id = __mysql_add_name(owner, len,
                              &mysql_insert_person_stmt, mysql_insert_person_sql);

    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_get_category_id(const char *category) {

    unsigned long id = 0;
    unsigned long parent_id = 0;

    char *tmp = str_copy(category, strnlen(category, 1024) + 1);
    char *start = tmp;
    if (start[0] == CATEGORY_SEPARATOR)
        start++;
    char *last = start + strlen(start);
    char *sep;
    while (start < last) {
        sep = strchr(start, CATEGORY_SEPARATOR);
        if (sep == NULL)
            sep = start + strlen(start) + 1;
        *sep = '\0';

        if (start[0] != '\0') {
            id = __mysql_get_tree_item_id(parent_id, start, strlen(start),
                                          &mysql_select_category_id_stmt, mysql_select_category_id_sql);
            if (id == 0)
                id = __mysql_add_tree_item(parent_id, start, strlen(start),
                                           &mysql_insert_category_stmt, mysql_insert_category_sql);

            parent_id = id;
        }
        start = sep + 1;
    }

    free(tmp);
    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_get_tag_id(const char *tag) {

    if (is_null_or_empty(tag))
        fail(EX_DATAERR, "Tag name cannot be null or empty");

    int len = strnlen(tag, 512);

    unsigned long id = __mysql_get_name_id(tag, len,
                                           &mysql_select_tag_id_stmt, mysql_select_tag_id_sql);
    if (id == 0)
        id = __mysql_add_name(tag, len,
                              &mysql_insert_tag_stmt, mysql_insert_tag_sql);

    return id;
}

#endif //ASSETS_ARCHIVE_METADATA_DB_MYSQL