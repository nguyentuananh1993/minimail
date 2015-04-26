#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>
#include "server.h"
#include "database.h"
#include "smtp.h"
#include "pop3.h"

#define ERROR(format, args...)                                  \
    do {                                                        \
        fprintf(stderr, "%s: " format "\n", argv[0], args);     \
        exit(EXIT_FAILURE);                                     \
    } while (0)

int main(int argc, char **argv)
{

    /* Configuration */
    int smtp_port = 0;
    int pop3_port = 0;
    char *dbfile = "email.db";
    if(argc ==1){
        printf("Using default port:\n - SMTP's port: 7725.\n - POP3's port: 7110.\n");
        smtp_port = 7725;
        pop3_port = 7110;
    }else
    if(argc != 3){
        printf("error input: %s [smpt_port] [POP3_port]\n",argv[0]);
        exit(0);
    }else{
        smtp_port = atoi(argv[1]);
        pop3_port = atoi(argv[2]);
    }
    printf("This mail server can connect via telnet as client\n");
    /* Option parsing */
    static struct option options[] = {
        {"smtp-port", required_argument, 0, 's'},
        {"pop3-port", required_argument, 0, 'p'},
        {"database",  required_argument, 0, 'd'},
        {0}
    };
    int option;
    while ((option = getopt_long(argc, argv, "", options, NULL)) != -1) {
        switch (option) {
        case 's':
            smtp_port = atoi(optarg);
            break;
        case 'p':
            pop3_port = atoi(optarg);
            break;
        case 'd':
            dbfile = optarg;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    /* Ensure database works */
    struct database db;
    if (database_open(&db, dbfile) != 0)
        ERROR("failed to open database, %s", dbfile);
    database_close(&db);
    
    // struct message_email *mess;
    // mess = message_email *database_list_email(db,"anhnt@gmail.com");
    // for (struct message_email *m = mess; m; m = m->next){
    //             char temp[1000];
    //             int i=0;
    //             while(m->content[i]!='\n'){
    //                 temp[i] = m->content[i];
    //                 i++;
    //             }
    //             temp[i]='\0';
    //             if(temp1==0)
    //                 RESPOND(client, "%-5d | %-30s | %-30s", m->id, temp,m->user);
    //         }
    /* Launch servers. */
    struct server smtp = {
        .port = smtp_port,
        .handler = smtp_handler,
        .arg = dbfile
    };
    struct server pop3 = {
        .port = pop3_port,
        .handler = pop3_handler,
        .arg = dbfile
    };
    if (server_start(&smtp) != 0)
        ERROR("%s", "failed to start SMTP server");
    if (server_start(&pop3) != 0)
        ERROR("%s", "failed to start POP3 server");
    pthread_join(smtp.thread, NULL);
    pthread_join(pop3.thread, NULL);
    
    return 0;
}
