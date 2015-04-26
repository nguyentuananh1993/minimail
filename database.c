#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "database.h"

#define CHECK(db, expr, ret)                                            \
    if ((expr) != SQLITE_OK) {                                          \
        fprintf(stderr, "sqlite3: %s\n", sqlite3_errmsg((db)->db));     \
        return ret;                                                     \
    }
// DEFINE USER EXECUTE COMMAND
static const char *SQL_TABLE_USER =
    "CREATE TABLE IF NOT EXISTS user"
    "(id INTEGER PRIMARY KEY, email,firstname,lastname,pass1,pass2)";

static const char *SQL_SEND_USER =
    "INSERT INTO user (email, firstname,lastname,pass1, pass2) VALUES (?, ?, ?, ?, ?)";

static const char *SQL_LIST_USER =
    "SELECT  id,email,firstname,lastname,pass1,pass2 FROM user WHERE email = ?";
//
static const char *SQL_DELETE_USER =
    "DELETE FROM user WHERE id = ?";

static const char *SQL_SIGNIN_USER =
    "SELECT id,email,firstname,lastname,pass1,pass2 FROM user WHERE email = ? AND pass1 = ?";

static const char *SQL_FORGOTPASSWORD_USER =
    "SELECT id,email,firstname,lastname,pass1,pass2 FROM user WHERE email = ? AND pass2 = ?";

// DEFINE EMAIL EXECUTE COMMAND
static const char *SQL_TABLE_EMAIL =
    "CREATE TABLE IF NOT EXISTS email"
    "(id INTEGER PRIMARY KEY, user, message,frommail,datetime)";

static const char *SQL_SEND_EMAIL =
    "INSERT INTO email (user, message,frommail,datetime) VALUES (?, ?, ?, ?)";

static const char *SQL_LIST_EMAIL =
    "SELECT id, message,frommail FROM email WHERE user = ?";

static const char *SQL_DELETE_EMAIL =
    "DELETE FROM email WHERE id = ?";

int database_open(struct database *db, const char *file)
{
    if (sqlite3_open(file, &db->db) != 0)
        return -1;
    CHECK(db, sqlite3_exec(db->db, SQL_TABLE_EMAIL, NULL, NULL, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_SEND_EMAIL, -1, &db->sendEmail, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_LIST_EMAIL, -1, &db->listEmail, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_DELETE_EMAIL, -1, &db->delEmail, NULL), -1);
    CHECK(db, sqlite3_exec(db->db, SQL_TABLE_USER, NULL, NULL, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_SEND_USER, -1, &db->sendUser, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_SIGNIN_USER, -1, &db->signinUser, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_FORGOTPASSWORD_USER, -1, &db->forgotPasword, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_LIST_USER, -1, &db->listUser, NULL), -1);
    CHECK(db, sqlite3_prepare_v2(db->db, SQL_DELETE_USER, -1, &db->delUser, NULL), -1);
    return 0;
}

int database_close(struct database *db)
{
    return sqlite3_close(db->db);
}

int database_send_user(struct database *db, const char *email, const char *firstname, const char *lastname,const char *pass1,const char *pass2)
{
    CHECK(db, sqlite3_reset(db->sendUser), -1);
    CHECK(db, sqlite3_bind_text(db->sendUser, 1, email, -1, SQLITE_TRANSIENT), -1);
    CHECK(db, sqlite3_bind_text(db->sendUser, 2, firstname, -1, SQLITE_TRANSIENT), -1);
    CHECK(db, sqlite3_bind_text(db->sendUser, 3, lastname, -1, SQLITE_TRANSIENT), -1);
    CHECK(db, sqlite3_bind_text(db->sendUser, 4, pass1, -1, SQLITE_TRANSIENT), -1);
    CHECK(db, sqlite3_bind_text(db->sendUser, 5, pass2, -1, SQLITE_TRANSIENT), -1);
    return sqlite3_step(db->sendUser) == SQLITE_ROW ? 0 : -1;
}

int database_send_email(struct database *db, const char *user, const char *message, const char *fromMail)
{
    CHECK(db, sqlite3_reset(db->sendEmail), -1);
    CHECK(db, sqlite3_bind_text(db->sendEmail, 1, user, -1, SQLITE_TRANSIENT), -1);
    CHECK(db, sqlite3_bind_text(db->sendEmail, 3, fromMail, -1, SQLITE_TRANSIENT), -1);
    CHECK(db, sqlite3_bind_blob(db->sendEmail, 2, message, strlen(message), SQLITE_TRANSIENT), -1);
    return sqlite3_step(db->sendEmail) == SQLITE_ROW ? 0 : -1;
}

struct message_email *
database_list_email(struct database *db, const char *user)
{
    CHECK(db, sqlite3_reset(db->listEmail), NULL);
    CHECK(db, sqlite3_bind_text(db->listEmail, 1, user, -1, SQLITE_TRANSIENT),
          NULL);
    struct message_email *messages = NULL;
    while (sqlite3_step(db->listEmail) == SQLITE_ROW) {
        int id = sqlite3_column_int(db->listEmail, 0);
        const void *blob = sqlite3_column_blob(db->listEmail, 2);
        const void *blob1 = sqlite3_column_blob(db->listEmail, 1);
        int length = sqlite3_column_bytes(db->listEmail, 2);
        int length1 = sqlite3_column_bytes(db->listEmail, 1);
        struct message_email *message = malloc(sizeof(*message) + length + length1);
        message->content = (char*) malloc(sizeof(char)*length1);
        message->user = (char*) malloc(sizeof(char)*length);
        memcpy(message->content, blob1, length1);
        memcpy(message->user,blob,length);
        message->id = id;
        message->length = length;
        message->next = messages;
        messages = message;
    }
    return messages;
}

struct message_user *database_list_user_forgotpassword(struct database *db, const char *user,const char *password)
{
    CHECK(db, sqlite3_reset(db->forgotPasword), NULL);
    CHECK(db, sqlite3_bind_text(db->forgotPasword, 1, user, -1, SQLITE_TRANSIENT),NULL);
    CHECK(db, sqlite3_bind_text(db->forgotPasword, 2, password, -1, SQLITE_TRANSIENT),NULL);
    struct message_user *messages = NULL;
    while (sqlite3_step(db->forgotPasword) == SQLITE_ROW) {
        int id = sqlite3_column_int(db->forgotPasword, 0);
        const void *email = sqlite3_column_blob(db->forgotPasword, 1);
        const void *firstname = sqlite3_column_blob(db->forgotPasword, 2);
        const void *lastname = sqlite3_column_blob(db->forgotPasword, 3);
        const void *pass1 = sqlite3_column_blob(db->forgotPasword, 4);
        const void *pass2 = sqlite3_column_blob(db->forgotPasword, 5);
        int len1=sqlite3_column_bytes(db->forgotPasword, 1);
        int len2=sqlite3_column_bytes(db->forgotPasword, 2);
        int len3=sqlite3_column_bytes(db->forgotPasword, 3);
        int len4=sqlite3_column_bytes(db->forgotPasword, 4);
        int len5=sqlite3_column_bytes(db->forgotPasword, 5);

        struct message_user *message = malloc(sizeof(*message)+len1 + len2 + len3 + len4 + len5);
        message->id = id;
        message->email = (char*) malloc(len1);
        message->firstName = (char*) malloc(len2);
        message->lastName = (char*) malloc(len3);
        message->pass1 = (char*) malloc(len4);
        message->pass2 = (char*) malloc(len5);
        memcpy(message->email,email,len1);
        memcpy(message->firstName,firstname,len2);
        memcpy(message->lastName,lastname,len3);
        memcpy(message->pass1,pass1,len4);
        memcpy(message->pass2,pass2,len5);
        message->next = messages;
        messages = message;
    }
    return messages;
}
struct message_user *database_list_user(struct database *db, const char *user)
{
    CHECK(db, sqlite3_reset(db->listUser), NULL);
    CHECK(db, sqlite3_bind_text(db->listUser, 1, user, -1, SQLITE_TRANSIENT),NULL);
    struct message_user *messages = NULL;
    while (sqlite3_step(db->listUser) == SQLITE_ROW) {
        int id = sqlite3_column_int(db->listUser, 0);
        const void *email = sqlite3_column_blob(db->listUser, 1);
        const void *firstname = sqlite3_column_blob(db->listUser, 2);
        const void *lastname = sqlite3_column_blob(db->listUser, 3);
        const void *pass1 = sqlite3_column_blob(db->listUser, 4);
        const void *pass2 = sqlite3_column_blob(db->listUser, 5);
        int len1=sqlite3_column_bytes(db->listUser, 1);
        int len2=sqlite3_column_bytes(db->listUser, 2);
        int len3=sqlite3_column_bytes(db->listUser, 3);
        int len4=sqlite3_column_bytes(db->listUser, 4);
        int len5=sqlite3_column_bytes(db->listUser, 5);

        struct message_user *message = malloc(sizeof(*message)+len1 + len2 + len3 + len4 + len5);
        message->id = id;
        message->email = (char*) malloc(len1);
        message->firstName = (char*) malloc(len2);
        message->lastName = (char*) malloc(len3);
        message->pass1 = (char*) malloc(len4);
        message->pass2 = (char*) malloc(len5);
        memcpy(message->email,email,len1);
        memcpy(message->firstName,firstname,len2);
        memcpy(message->lastName,lastname,len3);
        memcpy(message->pass1,pass1,len4);
        memcpy(message->pass2,pass2,len5);
        message->next = messages;
        messages = message;
    }
    return messages;
}

struct message_user *database_list_user_signin(struct database *db, const char *user,const char *password)
{
    CHECK(db, sqlite3_reset(db->signinUser), NULL);
    CHECK(db, sqlite3_bind_text(db->signinUser, 1, user, -1, SQLITE_TRANSIENT),NULL);
    CHECK(db, sqlite3_bind_text(db->signinUser, 2, password, -1, SQLITE_TRANSIENT),NULL);
    struct message_user *messages = NULL;
    while (sqlite3_step(db->signinUser) == SQLITE_ROW) {
        int id = sqlite3_column_int(db->signinUser, 0);
        const void *email = sqlite3_column_blob(db->signinUser, 1);
        const void *firstname = sqlite3_column_blob(db->signinUser, 2);
        const void *lastname = sqlite3_column_blob(db->signinUser, 3);
        const void *pass1 = sqlite3_column_blob(db->signinUser, 4);
        const void *pass2 = sqlite3_column_blob(db->signinUser, 5);
        int len1=sqlite3_column_bytes(db->signinUser, 1);
        int len2=sqlite3_column_bytes(db->signinUser, 2);
        int len3=sqlite3_column_bytes(db->signinUser, 3);
        int len4=sqlite3_column_bytes(db->signinUser, 4);
        int len5=sqlite3_column_bytes(db->signinUser, 5);

        struct message_user *message = malloc(sizeof(*message)+len1 + len2 + len3 + len4 + len5);
        message->id = id;
        message->email = (char*) malloc(len1);
        message->firstName = (char*) malloc(len2);
        message->lastName = (char*) malloc(len3);
        message->pass1 = (char*) malloc(len4);
        message->pass2 = (char*) malloc(len5);
        memcpy(message->email,email,len1);
        memcpy(message->firstName,firstname,len2);
        memcpy(message->lastName,lastname,len3);
        memcpy(message->pass1,pass1,len4);
        memcpy(message->pass2,pass2,len5);
        message->next = messages;
        messages = message;
    }
    return messages;
}
int database_delete_email(struct database *db, int id)
{
    CHECK(db, sqlite3_reset(db->delEmail), -1);
    CHECK(db, sqlite3_bind_int(db->delEmail, 1, id), -1);
    while (sqlite3_step(db->delEmail) == SQLITE_ROW);
    return 0;
}
int database_delete_user(struct database *db, int id)
{
    CHECK(db, sqlite3_reset(db->delUser), -1);
    CHECK(db, sqlite3_bind_int(db->delUser, 1, id), -1);
    while (sqlite3_step(db->delUser) == SQLITE_ROW);
    return 0;
}