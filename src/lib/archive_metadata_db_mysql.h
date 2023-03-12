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

#include "archive_metadata_db.h"
#include "archive_metadata_json.h"

void *mysql = NULL;


struct __mysql_query {
    const char *get_sql;
    MYSQL_STMT *get_stmt;
    const char *add_sql;
    MYSQL_STMT *add_stmt;
};

struct __mysql_query *__mysql_hash = NULL;
struct __mysql_query *__mysql_person = NULL;
struct __mysql_query *__mysql_tag = NULL;
struct __mysql_query *__mysql_category = NULL;
struct __mysql_query *__mysql_origin = NULL;
struct __mysql_query *__mysql_participant_mapping = NULL;
struct __mysql_query *__mysql_tag_mapping = NULL;

/**
 *
 */
struct __mysql_query *__mysql_create_query(const char *get_sql, const char *add_sql) {
    struct __mysql_query *query = malloc(sizeof(struct __mysql_query));
    query->get_sql = str_copy(get_sql, strlen(get_sql));
    query->get_stmt = NULL;
    query->add_sql = str_copy(add_sql, strlen(add_sql));
    query->add_stmt = NULL;
    return query;
}

/**
 *
 */
void __mysql_create_queries() {

    __mysql_hash = __mysql_create_query(
            "SELECT ID FROM HASH WHERE HASH=?;",
            "INSERT INTO HASH(HASH) VALUES(?);"
    );

    __mysql_person = __mysql_create_query(
            "SELECT ID FROM PERSON WHERE NAME=?;",
            "INSERT INTO PERSON(NAME) VALUES(?);"
    );

    __mysql_tag = __mysql_create_query(
            "SELECT ID FROM TAG WHERE TAG=?;",
            "INSERT INTO TAG(TAG) VALUES(?);"
    );

    __mysql_category = __mysql_create_query(
            "SELECT ID FROM CATEGORY WHERE PARENT=? AND NAME=?;",
            "INSERT INTO CATEGORY(PARENT, NAME) VALUES(?, ?);"
    );

    __mysql_origin = __mysql_create_query(
            "SELECT ID FROM ORIGIN WHERE HASH=? AND NAME=?;",
            "INSERT INTO ORIGIN(HASH, NAME, SUBJECT, OWNER, CATEGORY, CREATED, CHANGED) VALUES(?, ?, ?, ?, ?, ?, ?);"
    );

    __mysql_participant_mapping = __mysql_create_query(
            "SELECT ID FROM ORIGIN_PARTICIPANT WHERE ORIGIN=? AND PERSON=?;",
            "INSERT INTO ORIGIN_PARTICIPANT(ORIGIN, PERSON) VALUES(?, ?);"
    );

    __mysql_tag_mapping = __mysql_create_query(
            "SELECT ID FROM ORIGIN_TAG WHERE ORIGIN=? AND TAG=?;",
            "INSERT INTO ORIGIN_TAG(ORIGIN, TAG) VALUES(?, ?);"
    );
}

/**
 *
 */
void __mysql_close_query(struct __mysql_query *query) {
    if (query->get_stmt != NULL)
        mysql_stmt_close(query->get_stmt);
    query->get_stmt = NULL;

    if (query->add_stmt != NULL)
        mysql_stmt_close(query->add_stmt);
    query->add_stmt = NULL;
}

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
unsigned long __mysql_add_name(const char *name, int len, struct __mysql_query *query) {

    if (!__mysql_prepare_stmt(&query->add_stmt, query->add_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_bind(MYSQL_TYPE_STRING, (void *) name);
    p->buffer_length = len;

    unsigned long id = __mysql_execute(query->add_stmt, p);
    if (id == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(query->add_stmt);
    free(p);

    return id;
}

/**
 * @return id if found, -1 on error, 0 if no data found
 */
unsigned long __mysql_get_name_id(const char *name, int len, struct __mysql_query *query) {

    if (!__mysql_prepare_stmt(&query->get_stmt, query->get_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_bind(MYSQL_TYPE_STRING, (void *) name);
    p->buffer_length = len;

    unsigned long id = 0;
    MYSQL_BIND *r = __mysql_create_bind(MYSQL_TYPE_LONG, &id);

    int ret = __mysql_fetch_result(query->get_stmt, p, r);
    if (ret == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(query->get_stmt);
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
                                    struct __mysql_query *query) {

    if (!__mysql_prepare_stmt(&query->add_stmt, query->add_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &parent;
    p[1].buffer_type = MYSQL_TYPE_STRING;
    p[1].buffer = (char *) name;
    p[1].buffer_length = len;

    unsigned long id = __mysql_execute(query->add_stmt, p);
    if (id == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(query->add_stmt);
    free(p);

    return id;
}

/**
 * @return id if found, -1 on error, 0 if no data found
 */
unsigned long __mysql_get_tree_item_id(unsigned long parent, const char *name, int len,
                                       struct __mysql_query *query) {

    if (!__mysql_prepare_stmt(&query->get_stmt, query->get_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &parent;
    p[1].buffer_type = MYSQL_TYPE_STRING;
    p[1].buffer = (char *) name;
    p[1].buffer_length = len;

    unsigned long id = 0;
    MYSQL_BIND *r = __mysql_create_bind(MYSQL_TYPE_LONG, &id);

    int ret = __mysql_fetch_result(query->get_stmt, p, r);
    if (ret == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(query->get_stmt);
    free(r);
    free(p);

    if (ret == 0)
        return 0;

    return id;
}

/**
 * @return id on success, -1 on fail
 */
unsigned long __mysql_add_origin(unsigned long hash_id, const char *name, const char *subject,
                                 unsigned long owner_id, unsigned long category_id,
                                 const char *created, const char *changed) {

    if (!__mysql_prepare_stmt(&__mysql_origin->add_stmt, __mysql_origin->add_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(7);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &hash_id;
    p[1].buffer_type = MYSQL_TYPE_STRING;
    p[1].buffer = (char *) name;
    p[1].buffer_length = strnlen(name, MAX_LENGTH_ORIGIN_NAME);
    p[2].buffer_type = !is_null_or_empty(subject) ? MYSQL_TYPE_STRING : MYSQL_TYPE_NULL;
    p[2].buffer = (char *) subject;
    p[2].buffer_length = !is_null_or_empty(subject) ? strnlen(subject, MAX_LENGTH_SUBJECT) : 0;
    p[3].buffer_type = MYSQL_TYPE_LONG;
    p[3].buffer = &owner_id;
    p[4].buffer_type = MYSQL_TYPE_LONG;
    p[4].buffer = &category_id;
    p[5].buffer_type = !is_null_or_empty(created) ? MYSQL_TYPE_STRING : MYSQL_TYPE_NULL;
    p[5].buffer = (char *) created;
    p[5].buffer_length = !is_null_or_empty(created) ? strlen(created) : 0;
    p[6].buffer_type = !is_null_or_empty(changed) ? MYSQL_TYPE_STRING : MYSQL_TYPE_NULL;
    p[6].buffer = (char *) changed;
    p[6].buffer_length = !is_null_or_empty(changed) ? strlen(changed) : 0;

    unsigned long id = __mysql_execute(__mysql_origin->add_stmt, p);
    if (id == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(__mysql_origin->add_stmt);
    free(p);

    return id;
}

/**
 * @return id if found, -1 on error, 0 if no data found
 */
unsigned long __mysql_get_origin_id(unsigned long hash_id, const char *name) {

    if (!__mysql_prepare_stmt(&__mysql_origin->get_stmt, __mysql_origin->get_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &hash_id;
    p[1].buffer_type = MYSQL_TYPE_STRING;
    p[1].buffer = (char *) name;
    p[1].buffer_length = strnlen(name, MAX_LENGTH_ORIGIN_NAME);

    unsigned long id = 0;
    MYSQL_BIND *r = __mysql_create_bind(MYSQL_TYPE_LONG, &id);

    int ret = __mysql_fetch_result(__mysql_origin->get_stmt, p, r);
    if (ret == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(__mysql_origin->get_stmt);
    free(r);
    free(p);

    if (ret == 0)
        return 0;

    return id;
}

/**
 * @return id on success, -1 on fail
 */
unsigned long __mysql_add_mapping(unsigned long key1, unsigned long key2, struct __mysql_query *query) {

    if (!__mysql_prepare_stmt(&query->add_stmt, query->add_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &key1;
    p[1].buffer_type = MYSQL_TYPE_LONG;
    p[1].buffer = &key2;

    unsigned long id = __mysql_execute(query->add_stmt, p);
    if (id == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(query->add_stmt);
    free(p);

    return id;
}

/**
 * @return id if found, -1 on error, 0 if no data found
 */
unsigned long __mysql_get_mapping_id(unsigned long key1, unsigned long key2, struct __mysql_query *query) {

    if (!__mysql_prepare_stmt(&query->get_stmt, query->get_sql))
        fail(EX_IOERR, "Failed to prepare statement");

    MYSQL_BIND *p = __mysql_create_binds(2);
    p[0].buffer_type = MYSQL_TYPE_LONG;
    p[0].buffer = &key1;
    p[1].buffer_type = MYSQL_TYPE_LONG;
    p[1].buffer = &key2;

    unsigned long id = 0;
    MYSQL_BIND *r = __mysql_create_bind(MYSQL_TYPE_LONG, &id);

    int ret = __mysql_fetch_result(query->get_stmt, p, r);
    if (ret == -1)
        fail(EX_IOERR, "Failed to execute statement");

    mysql_stmt_free_result(query->get_stmt);
    free(r);
    free(p);

    if (ret == 0)
        return 0;

    return id;
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

    __mysql_create_queries();

    return 1;
}

/**
 *
 */
void archive_metadata_db_disconnect() {

    __mysql_close_query(__mysql_hash);
    __mysql_close_query(__mysql_person);
    __mysql_close_query(__mysql_tag);
    __mysql_close_query(__mysql_category);
    __mysql_close_query(__mysql_origin);
    __mysql_close_query(__mysql_participant_mapping);
    __mysql_close_query(__mysql_tag_mapping);

    mysql_close(mysql);
}


/**
 *
 */
unsigned long archive_metadata_db_get_hash_id(const char *hash) {

    if (is_null_or_empty(hash))
        fail(EX_DATAERR, "Owner name cannot be null or empty");

    int len = strnlen(hash, MAX_LENGTH_HASH);

    unsigned long id = __mysql_get_name_id(hash, len, __mysql_hash);
    if (id == 0)
        id = __mysql_add_name(hash, len, __mysql_hash);

    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_get_person_id(const char *owner) {

    if (is_null_or_empty(owner))
        fail(EX_DATAERR, "Owner name cannot be null or empty");

    int len = strnlen(owner, MAX_LENGTH_PERSON_NAME);

    unsigned long id = __mysql_get_name_id(owner, len, __mysql_person);
    if (id == 0)
        id = __mysql_add_name(owner, len, __mysql_person);

    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_get_category_id(const char *category) {

    unsigned long id = 0;
    unsigned long parent_id = 0;

    char *tmp = str_copy(category, strnlen(category, MAX_LENGTH_CATEGORY_PATH) + 1);
    char *start = tmp;
    if (start[0] == CATEGORY_SEPARATOR)
        start++;
    char *last = start + strlen(start);
    char *sep;
    size_t len;
    while (start < last) {
        sep = strchr(start, CATEGORY_SEPARATOR);
        if (sep == NULL)
            sep = start + strnlen(start, MAX_LENGTH_CATEGORY_NAME) + 1;
        *sep = '\0';
        len = strnlen(start, MAX_LENGTH_CATEGORY_NAME);

        if (start[0] != '\0') {
            id = __mysql_get_tree_item_id(parent_id, start, len, __mysql_category);
            if (id == 0)
                id = __mysql_add_tree_item(parent_id, start, len, __mysql_category);

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

    int len = strnlen(tag, MAX_LENGTH_TAG_NAME);

    unsigned long id = __mysql_get_name_id(tag, len, __mysql_tag);
    if (id == 0)
        id = __mysql_add_name(tag, len, __mysql_tag);

    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_get_origin_id(unsigned long hash_id, const char *name, const char *subject,
                                                unsigned long owner_id, unsigned long category_id,
                                                const char *created, const char *changed) {

    if (is_null_or_empty(name))
        fail(EX_DATAERR, "Origin name cannot be null or empty");

    unsigned long id = __mysql_get_origin_id(hash_id, name);
    if (id == 0)
        id = __mysql_add_origin(hash_id, name, subject,
                                owner_id, category_id,
                                created, changed
        );

    return id;

}

/**
 *
 */
unsigned long archive_metadata_db_assign_participant(unsigned long origin_id, unsigned long person_id) {

    unsigned long id = __mysql_get_mapping_id(origin_id, person_id, __mysql_participant_mapping);
    if (id == 0)
        id = __mysql_add_mapping(origin_id, person_id, __mysql_participant_mapping);

    return id;
}

/**
 *
 */
unsigned long archive_metadata_db_assign_tag(unsigned long origin_id, unsigned long tag_id) {

    unsigned long id = __mysql_get_mapping_id(origin_id, tag_id, __mysql_tag_mapping);
    if (id == 0)
        id = __mysql_add_mapping(origin_id, tag_id, __mysql_tag_mapping);

    return id;

}

#endif //ASSETS_ARCHIVE_METADATA_DB_MYSQL