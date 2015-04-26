#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "database.h"
#include "pop3.h"

void pop3_handler(FILE *client, void *arg)
{
    pop3(client, (const char *) arg);
}

#define RESPOND(client, format, args...)        \
    fprintf(client, format "\n", args)

void pop3(FILE *client, const char *dbfile)
{
    struct database db;
    if (database_open(&db, dbfile) != 0) {
        RESPOND(client, "%s", "-ERR");
        fclose(client);
        return;
    } else {
        RESPOND(client, "+OK %s", "POP3 server ready");
    }

    char user[512] = {0};
    char pass[512] = {0};
    char email[512] = {0};
    char pass1[512] = {0};
    char pass2[512] = {0};
    char firstname[512] = {0};
    char lastname[512] = {0};

    struct message_email *messages = NULL;
    struct message_user *messages_user = NULL;
    while (!feof(client)) {
        char line[512];
        printf("Client message: %s", line);
        if(fgets(line, sizeof(line), client)){}
        char command[5] = {line[0], line[1], line[2], line[3]};
        if(strcmp(command,"CREA") == 0){
            int i;
            int count = 0;
            int errorCreate = 0;
             if (strlen(line) > 5) {
                char *a = line + 5, *b = email,*c = firstname,*d = lastname,*e = pass1,*f = pass2;
                for(i = 0;i<strlen(line);i++)
                    if(line[i] =='+')
                        count++;
                if(count==5){
                    while (*a!='+')
                        *(b++) = *(a++);
                    *b = '\0';
                    a++;
                    while (*a!='+')
                        *(c++) = *(a++);
                    *c = '\0';
                    a++;
                    while (*a!='+')
                        *(d++) = *(a++);
                    *d = '\0';
                    a++;
                    while (*a!='+')
                        *(e++) = *(a++);
                    *e = '\0';
                    a++;
                    while (*a!='+')
                        *(f++) = *(a++);
                    *f = '\0';
                    i=0;
                    messages_user = database_list_user_signin(&db,email,pass1);
                    for(struct message_user *m = messages_user; m; m = m->next){
                            i++;
                        }
                    if(i!=0){
                        errorCreate = 1;
                    } else
                        errorCreate = 0;
                    if(!errorCreate){
                        database_send_user(&db,email,firstname,lastname,pass1,pass2);
                        RESPOND(client,"%s","+OK");
                    }else RESPOND(client,"%s","-ER");
                }
            }
        }else if (strcmp(command, "USER") == 0) {
             if (strlen(line) > 5) {
                char *a = line + 5, *b = user;
                while (*a!='+')
                    *(b++) = *(a++);
                *b = '\0';
                RESPOND(client, "%s", "+OKUSER");
            } else {
                RESPOND(client, "%s", "-ERRUSER");
            }
            printf("|%s|\n",user);
            messages = database_list_email(&db, user);
        } else if (strcmp(command, "PASS") == 0) {
                if (strlen(line) > 5) {
                    char *a = line + 5, *b = pass;
                    while (*a!='+')
                        *(b++) = *(a++);
                    *b = '\0';
                    if(!user[0])
                        RESPOND(client,"%s","-ERR enter user first");
                    else{
                        messages_user = database_list_user_signin(&db,user,pass);
                        int i = 0;
                        for(struct message_user *m = messages_user; m; m = m->next){
                            i++;
                        }
                        if(i!=1){
                            RESPOND(client,"%s","-ER");
                        } else
                            RESPOND(client,"%s","+OK");
                }
            }
        }  else if (strcmp(command, "FORG") == 0) {
                if (strlen(line) > 5) {
                    char *a = line + 5, *b = pass;
                    while (*a!='+')
                        *(b++) = *(a++);
                    *b = '\0';
                    if(!user[0])
                        RESPOND(client,"%s","-ERR enter user first");
                    else{
                        messages_user = database_list_user_forgotpassword(&db,user,pass);
                        int i = 0;
                        for(struct message_user *m = messages_user; m; m = m->next){
                            i++;
                            RESPOND(client, "email: %s\npassword: %s", m->email, m->pass1);
                        }
                        if(i!=1){
                            RESPOND(client,"%s","Wrong level 2 password.");
                        }
                }
            }
        }else if (strcmp(command, "STAT") == 0) {
            int count = 0, size = 0;
            for (struct message_email *m = messages; m; m = m->next) {
                count++;
                size += m->length;
            }
            RESPOND(client, "+OK %d %d", count, size);
        } else if (strcmp(command, "LIST") == 0) {
            int temp1=0;
            for (struct message_email *m = messages; m; m = m->next){
                temp1++;
                char temp[1000];
                char tempuser[1000];
                int i=0;
                while(m->content[i]!='\n'){
                    temp[i] = m->content[i];
                    i++;
                }
                temp[i]='\0';
                i=0;
                while(m->user[i]!='\n' && m->user[i]!='\r'){
                    tempuser[i] = m->user[i];
                    i++;
                }
                tempuser[i]='\0';
               
                RESPOND(client, "%-5d | %-30s | %-30s", m->id, temp,tempuser);
            }
            if(temp1==0)
                RESPOND(client,"%s","inbox is empty!");
}else if (strcmp(command, "RETR") == 0) {
            int id = atoi(line + 4);
            int found = 0;
            for (struct message_email *m = messages; m; m = m->next)
                if (m->id == id) {
                    found = 1;
                    RESPOND(client, " - Mail from: %s", m->user);
                    RESPOND(client, " - Subject: %s",m->content);
                }
            RESPOND(client, "%s", found ? "." : "-ERR");
        } else if (strcmp(command, "DELE") == 0) {
            int id = atoi(line + 4);
            int found = 0;
            for (struct message_email *m = messages; m; m = m->next) {
                if (m->id == id) {
                    found = 1;
                    RESPOND(client, "%s", "+OK");
                    database_delete_email(&db, id);
                    break;
                }
            }
            if (!found)
                RESPOND(client, "%s", "-ERRor");
        } else if (strcmp(command, "TOP ") == 0) {
            int id, lines;
            sscanf(line, "TOP %d %d", &id, &lines);
            int found = 0;
            for (struct message_email *m = messages; m; m = m->next) {
                if (m->id == id) {
                    RESPOND(client, "%s", "+OK");
                    found = 1;
                    char *p = m->content;
                    while (*p && memcmp(p, "\r\n\r\n", 4) != 0) {
                        fputc(*p, client);
                        p++;
                    }
                    if (*p) {
                        p += 4;
                        int line = 0;
                        while (*p && line < lines) {
                            if (*p == '\n')
                                line++;
                            p++;
                        }
                    }
                    break;
                }
            }
            RESPOND(client, "%s", found ? "\r\n." : "-ERR");
        } else if (strcmp(command, "QUIT") == 0) {
            RESPOND(client, "%s", "+OK");
            break;
        } else {
            RESPOND(client, "%s", "-ERR");
        }
    }

    while (messages) {
        struct message_email *dead = messages;
        messages = messages->next;
        free(dead);
    }
    fclose(client);
    database_close(&db);
}

