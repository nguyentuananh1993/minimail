#pragma once

#include <sqlite3.h>

struct database {
    sqlite3 *db;
    sqlite3_stmt *sendEmail, *listEmail, *delEmail, *sendUser, *listUser, *delUser,*signinUser,*forgotPasword;
};

struct message_email {
    int id;
    int length;
    char*user;
    char*content;
    struct message_email *next;
    
};
struct message_user{
	int id;
	char*email;
	char*firstName;
	char*lastName;
	char*pass1;
	char*pass2;
	struct message_user *next;
};

int database_open(struct database *db, const char *file);
int database_close(struct database *db);
int database_send_user(struct database *db, const char *email, const char *firstname, const char *lastname,const char *pass1,const char *pass2);
int database_send_email(struct database *db, const char *user, const char *message, const char *fromMail);
struct message_email *database_list_email(struct database *db, const char *user);
struct message_user *database_list_user(struct database *db, const char *user);
struct message_user *database_list_user_signin(struct database *db, const char *user,const char *password);
struct message_user *database_list_user_forgotpassword(struct database *db, const char *user,const char *password);
int database_delete_email(struct database *db, int id);
int database_delete_user(struct database *db, int id);