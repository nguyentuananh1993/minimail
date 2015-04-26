#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>

void menuSignedIn();
void switchCaseSignedIn(int enter);
void composeEmail();
void readEmail();
void deleteEmail();
void pressToContinue();
int  signIn();
void signUp();
void settings();
void forgotPassword();
void caseSignIn();
void sendMail(char*mailTo,char*subject,char*content);
int  connectSocket(const unsigned char* smtpUrl, const unsigned short smtpPort);
int  safeRead(int socketFd, unsigned char *readData, int readLen);
int  safeWrite(int socketFd, const unsigned char *writeData, int writeLen);
void menu();
void switchCaseMenu(int enter);
void cls();

int SMTP_PORT = 7725;
int POP3_PORT = 7110;

char email[512] = {0};
char *password;
int logedIn = 0;
char smtpUrl[512] = "localhost";
int fd;

int main()
{
	cls();
	printf("Using default port: 7725 for SMTP and 7110 for POP3\n");
	
	email[strlen(email)-1] = '\0';
	int enter = 0;
	while(enter != 5){
		menu();
		scanf("%d",&enter);
		while(getchar()!='\n');
		switchCaseMenu(enter);
	}	
}
void menu(){
	cls();
	printf("\n------=== MAIL SERVER ===-------------------");
	printf("\n\t1. Sign In.");
	printf("\n\t2. Sign Up.");
	printf("\n\t3. Forgot password?");
	printf("\n\t4. Settings.");
	printf("\n\t5. Exit.");
	printf("\nSelect: ");
}
void caseSignIn(){
	cls();
	int select = 0;
	do{
		if(!logedIn){
			if(signIn()){
				logedIn=1;
				menuSignedIn();
				scanf("%d",&select);
				while(getchar()!='\n');
				switchCaseSignedIn(select);
			}
			else
				printf("Wrong username or password.\n");
		}else{
			menuSignedIn();
			scanf("%d",&select);
			while(getchar()!='\n');
			switchCaseSignedIn(select);
		}
	}while(select != 4);
}
void switchCaseMenu(int enter){
	switch(enter){
		case 1: caseSignIn(); break;
		case 2: signUp(); break;
		case 3: forgotPassword(); break;
		case 4: settings(); break;
		case 5: printf("Thank for your using.\n"); break;
	}
}

void menuSignedIn()
{
	cls();
	printf("\n------=== HELLO USER ===-------------------");
	printf("\n\t1. Compose a new email.");
	printf("\n\t2. Read email.");
	printf("\n\t3. Delete email.");
	printf("\n\t4. Exit.");
	printf("\nSelect: ");
}

void switchCaseSignedIn(int enter)
{
	
	switch(enter)
	{
		case 1: composeEmail(); break;
		case 2: readEmail(); break;
		case 3: deleteEmail(); break;
		case 4: logedIn = 0; printf("Thank for your using.\n"); break;
		default: printf("Wrong input.");
	}
}


void pressToContinue()
{
	printf("Press enter to continue!\n");
	while(getchar()!='\n');
	cls();
}

int connectSocket(const unsigned char* smtpUrl, const unsigned short smtpPort)
{
    int socketFd = -1;
    struct sockaddr_in smtpAddr = {0};
	struct hostent *host = NULL;
    	
    if(NULL == (host = gethostbyname(smtpUrl)))
    {
        perror("gethostbyname...\n");
        exit(-1);
    }

	memset(&smtpAddr, 0, sizeof(smtpAddr));
    smtpAddr.sin_family = AF_INET;
    smtpAddr.sin_port = htons(smtpPort);
    smtpAddr.sin_addr = *((struct in_addr *)host->h_addr);
    
    socketFd = socket(PF_INET, SOCK_STREAM, 0);
    if(0 > socketFd)
    {
        perror("socket...\n");
        exit(-1);
    }

    if(0 > connect(socketFd, (struct sockaddr *)&smtpAddr, sizeof(struct sockaddr)))
    {
        close(socketFd);
        perror("connect...\n");
        exit(-1);
    }

    return socketFd;
}
int safeRead(int socketFd, unsigned char *readData, int readLen)
{
#ifdef NONEED_SELECT
	return recv(socketFd, readData, readLen, 0);
#else
	fd_set readFds;
	struct timeval tv;
	int ret, len = 0;

	if ((0 >= readLen) || (NULL == readData))
	{
		return -1;
	}

	FD_ZERO(&readFds);
	FD_SET(socketFd, &readFds);

	tv.tv_sec = 5;

	while(1)
	{
		ret = select(socketFd+1, &readFds, NULL, NULL, &tv);
		if (0 > ret)
		{
			perror("select...\n");
			//reconnect or ...
		}
		else if (0 == ret)
		{
			continue;
			//reconnect or continue wait
		}
		else
		{
			/* got it */
			if(FD_ISSET(socketFd, &readFds))
			{
				//don't do read twice
				len = recv(socketFd, readData, readLen, 0);
				//take data,and leave
				break;
			}
		}
	}	

	return len;
#endif
}

int safeWrite(int socketFd, const unsigned char *writeData, int writeLen)
{
#ifdef NONEED_SELECT
	return send(socketFd, writeData, writeLen, 0);
#else
	fd_set writeFds;
	struct timeval tv;
	int ret, len = 0;

	if ((0 >= writeLen) || (NULL == writeData))
	{
		return -1;
	}

	FD_ZERO(&writeFds);
	FD_SET(socketFd, &writeFds);

	tv.tv_sec = 5;

	while(1)
	{
		ret = select(socketFd+1, NULL, &writeFds, NULL, &tv);
		if (0 > ret)
		{
			perror("select...\n");
			/* reconnect or ... */
		}
		else if (0 == ret)
		{
			continue;
			/* reconnect or continue wait */
		}
		else
		{
			if(FD_ISSET(socketFd, &writeFds))
			{
				len = send(socketFd, writeData, writeLen, 0);
				break;
			}
		}
	}

	return len;
#endif	
}
void composeEmail()
{
	cls();
	char mailTo[512] = {'\0'};
	char subject[1024] = {'\0'};
	char content[1024] = {'\0'};
	char temp[100] = {'\0'};
	if((fd = connectSocket(smtpUrl, SMTP_PORT))<0){
		printf("connect false!\n");
		exit(0);
	}else printf("Authentication to SMTP's sever complete");
	printf("\n------=== COMPOSE A NEW MAIL ===-------------------\n");
	printf(" - To: ");
	fgets( mailTo, sizeof(mailTo), stdin );
	mailTo[strlen(mailTo)-1] = '\0';
	printf(" - Subject: ");
	fgets( subject, sizeof(subject), stdin );
	printf(" - Content(press '=' after that enter to end the content): \n");
	while(strcmp(temp,"=\n")!=0){
		printf("\t");
		fgets( temp, sizeof(temp), stdin );
		if(strcmp(temp,"=\n")!=0){
			int i = 0;
			while(temp[i]!='\0'){
				content[strlen(content)+1] = '\0';
				content[strlen(content)] = temp[i++];
			}
		}
	}
	strcat(subject,content);
	sendMail(mailTo,subject,subject);
	pressToContinue();
}
void sendMail(char*mailTo,char*subject,char*content)
{
	char readData[1024]={0};
	char enter[] = "\r\n";
	char tempData[1024]={0};
	//HELO
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	safeWrite(fd, "HELO\r\n", strlen("HELO\r\n"));
	//MAIL
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"MAIL ");
	strcat(tempData,email);
	strcat(tempData,enter);
	safeRead(fd, readData, 1024);
	safeWrite(fd, tempData, strlen(tempData));
	//RCPT
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"RCPT ");
	strcat(tempData,mailTo);
	strcat(tempData,enter);
	safeRead(fd, readData, 1024);
	safeWrite(fd, tempData, strlen(tempData));
	//DATA
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	safeWrite(fd, "DATA\r\n", strlen("DATA\r\n"));
	//DATA CONTENT
	memset(&tempData,0,1024);
	int i = 0;
	int j = 0;
	while(content[i]!='\0'){
		if(content[i]!='\n')
		tempData[j++] = content[i++];
		else if(content[i] =='\0' || content[i] == '\n'){
			i++;
			tempData[j++] = '\n';
			tempData[j++] = '\0';
			memset(&readData, 0, 1024);
			safeRead(fd, readData, 1024);
			safeWrite(fd, tempData, strlen(tempData));
			if(content[i] == '\0') break;
			memset(&tempData,0,1024);
			j=0;
		}
	}
	//END WITH DOT
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	safeWrite(fd, ".\r\n", strlen(".\r\n"));
	//QUIT
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	safeWrite(fd, "QUIT\r\n", strlen("QUIT\r\n"));
}
void readEmail()
{
	char readData[1024]={0};
	char enter[] = "\r\n";
	char plus[] = "+\n";
	char tempData[1024]={0};
	char str[10];
	int select;
	if((fd = connectSocket(smtpUrl, POP3_PORT))<0){
		printf("connect false!\n");
		exit(0);
	}else printf("Authentication to POP3's sever complete");
	//USER
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"USER ");
	strcat(tempData,email);
	strcat(tempData,plus);
	safeRead(fd, readData, 1024);
	printf("%s",readData);
	safeWrite(fd, tempData, strlen(tempData));
	printf("\n------=== READ MAIL ===-------------------------------------------\n");
	printf("%-5s | %-30s | %-26s\n", "STT", "Subject","FROM");
	printf("------------------------------------------------------------------\n");
	//LIST
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	safeWrite(fd, "LIST\r\n", strlen("LIST\r\n"));
	//RETRN
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	printf("%s",readData);

	printf(" - Select (Press 0 to comeback): ");
	scanf("%d",&select);
	while(getchar()!='\n');
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"RETR ");
	sprintf(str, "%d", select);
	strcat(tempData,str);
	strcat(tempData,enter);
	safeWrite(fd, tempData, strlen(tempData));
	//QUIT
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	if(readData[0]!='-' && select!=0){
		cls();
		printf("%s",readData);
	}
	safeWrite(fd, "QUIT\r\n", strlen("QUIT\r\n"));
	if(select!=0){
		pressToContinue();
		readEmail();
	}
	else
		pressToContinue();
}
void deleteEmail()
{
	cls();
	char readData[1024]={0};
	char enter[] = "\r\n";
	char plus[] = "+\n";
	char tempData[1024]={0};
	char str[10];
	int select;
	if((fd = connectSocket(smtpUrl, POP3_PORT))<0){
		printf("connect false!\n");
		pressToContinue();
		exit(0);
	}else printf("Authentication to POP3's sever complete");

	//USER
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"USER ");
	strcat(tempData,email);
	strcat(tempData,plus);
	safeRead(fd, readData, 1024);
	printf("%s",readData);
	safeWrite(fd, tempData, strlen(tempData));
	printf("\n------=== DELETE EMAIL ===-----------------------------------------\n");
	printf("%-5s | %-30s | %-26s\n", "STT", "Subject","FROM");
	printf("------------------------------------------------------------------\n");//LIST
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	safeWrite(fd, "LIST\r\n", strlen("LIST\r\n"));

	//RETRN
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	printf("%s",readData);
	printf(" - Select (Press 0 to comeback): ");
	scanf("%d",&select);
	while(getchar()!='\n');
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"DELE ");
	sprintf(str, "%d", select);
	strcat(tempData,str);
	strcat(tempData,enter);
	safeWrite(fd, tempData, strlen(tempData));

	//QUIT
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	if(readData[0]!='-' && select!=0){
		cls();
		printf("%s",readData);
	}
	safeWrite(fd, "QUIT\r\n", strlen("QUIT\r\n"));
	if(select!=0){
		if(readData[0]=='+')
			printf("delete sucessful!\n");
		else
			printf("delete fault!\n");
		pressToContinue();
		deleteEmail();
	}
	else
		pressToContinue();

}

int signIn()
{
	char readData[1024]={0};
	char enter[] = "+\n";
	char tempData[1024]={0};
	char str[10];
	int select;
	int flag = 1;

	//CONNECT TO POP3 SERVER
	if((fd = connectSocket(smtpUrl, POP3_PORT))<0){
		printf("connect false!\n");
		pressToContinue();
		exit(0);
	}else printf("Authentication to POP3's sever complete\n");
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);

	//INPUT USER AND PASSWORD
	//check email
	do{
		int i;
		int count;
		int alpha;
		flag = 1;
		printf("enter your email: ");
		fgets( email, sizeof(email), stdin);
		email[strlen(email)-1]='\0';
		count = 0;
    	for(i = 0;i < strlen(email);i++)
        	if(email[i]=='@'){
            count++;
            alpha = i;
        }
		if(count!=1) {
			flag = 0;
		}
		count = 0;
		for(i = alpha;i < strlen(email);i++)
        	if(email[i]=='.'){
            count++;
            alpha = i;
        }

		if(count == 0 ) 
			flag = 0;
		if(email[0]=='@') {
			flag =0;
		}
		if(flag == 0) 
			printf("Wrong input. example: example@example.com\n");
	}while(flag == 0);

	//check password
	do{
		flag = 1;
		password = getpass("Password: ");
		if(strlen(password)<8 || strlen(password)>31){
			flag = 0;
			printf("Use at least 8 characters and less than 32 character.\n");
		}
	}while(flag ==0);
	
	//SEND USER COMMAND
	memset(&tempData,0,1024);
	strcpy(tempData,"USER ");
	strcat(tempData,email);
	strcat(tempData,enter);
	safeWrite(fd, tempData, strlen(tempData));

	//PASS
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"PASS ");
	strcat(tempData,password);
	strcat(tempData,enter);
	safeRead(fd, readData, 1024);
	safeWrite(fd, tempData, strlen(tempData));
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	if(readData[0]=='+')
		return 1;
	else return 0;
}
void signUp()
{
	cls();
	char lastname[512] = {0};
	char firstname[512] = {0};
	char password2[512] = {0};
	char*repassword;
	char readData[1024]={0};
	char plus[] = "+";
	char enter[] = "+\n";
	char tempData[1024]={0};
	char str[10];
	char flag = 1;

	//CONNECT TO POP3 SERVER
	if((fd = connectSocket(smtpUrl, POP3_PORT))<0){
		printf("connect false!\n");
		pressToContinue();
		exit(0);
	}else printf("Authentication to POP3's sever complete\n");
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);

	//INPUT INFO
	//check email
	do{
		int i;
		int count;
		int alpha;
		flag = 1;
		printf("enter your email: ");
		fgets( email, sizeof(email), stdin);
		email[strlen(email)-1]='\0';
		count = 0;
    	for(i = 0;i < strlen(email);i++)
        	if(email[i]=='@'){
            count++;
            alpha = i;
        }
		if(count!=1) {
			flag = 0;
		}
		count = 0;
		for(i = alpha;i < strlen(email);i++)
        	if(email[i]=='.'){
            count++;
            alpha = i;
        }

		if(count == 0 ) 
			flag = 0;
		if(email[0]=='@') {
			flag =0;
		}
		if(flag == 0) 
			printf("Wrong input. example: example@example.com\n");
	}while(flag == 0);
	
	//check password
	do{
		flag = 1;
		password = getpass("Password: ");
		repassword = getpass("Re-enter password: ");
		if(strcmp(password,repassword) !=0){
			flag = 0;
			printf("Password does not match.\n");
		}
		if(strlen(password)<8 || strlen(password)>31){
			flag = 0;
			printf("Use at least 8 characters and less than 32 character.\n");
		}
	}while(flag ==0);

	//check firstname
	do{
		flag =1;
		printf("Your first name: ");
		fgets( firstname, sizeof(firstname), stdin);
		firstname[strlen(firstname)-1]='\0';
		if(strlen(firstname)>32){
			flag = 0;
			printf("first name less than 32 character");
		}
	}while(flag ==0);

	//check lastname
	do{
		flag =1;
		printf("Your last name: ");
		fgets( lastname, sizeof(lastname), stdin);
		lastname[strlen(lastname)-1]='\0';
		if(strlen(lastname)>32){
			flag = 0;
			printf("first name less than 32 characters");
		}
	}while(flag ==0);

	//check pass level 2
	do{
		flag = 1;
		printf("Level 2 password(using for lost account): ");
		fgets( password2, sizeof(password2), stdin);
		password2[strlen(password2)-1]='\0';
		if(strlen(lastname)>63){
			flag = 0;
			printf("first name less than 64 charactesr");
		}
	}while(flag==0);

	//SEND INFO
	memset(&tempData,0,1024);
	strcpy(tempData,"CREA ");
	strcat(tempData,email);
	strcat(tempData,plus);
	strcat(tempData,firstname);
	strcat(tempData,plus);
	strcat(tempData,lastname);
	strcat(tempData,plus);
	strcat(tempData,password);
	strcat(tempData,plus);
	strcat(tempData,password2);
	strcat(tempData,enter);
	safeWrite(fd, tempData, strlen(tempData));
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	printf("%s\n",readData);

	//CHECK STATUS
	if(readData[0]=='+')
		printf("Register sucessful.\n");
	else
		printf("Register fault.\n");
	pressToContinue();
}
void forgotPassword()
{
	char readData[1024]={0};
	char enter[] = "+\n";
	char tempData[1024]={0};
	char str[10];
	int select;
	int flag = 1;

	//CONNECT TO POP3 SERVER
	if((fd = connectSocket(smtpUrl, POP3_PORT))<0){
		printf("connect false!\n");
		pressToContinue();
		exit(0);
	}else printf("Authentication to POP3's sever complete\n");
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);

	//INPUT USER AND PASSWORD
	//check email
	do{
		int i;
		int count;
		int alpha;
		flag = 1;
		printf("enter your email: ");
		fgets( email, sizeof(email), stdin);
		email[strlen(email)-1]='\0';
		count = 0;
    	for(i = 0;i < strlen(email);i++)
        	if(email[i]=='@'){
            count++;
            alpha = i;
        }
		if(count!=1) {
			flag = 0;
		}
		count = 0;
		for(i = alpha;i < strlen(email);i++)
        	if(email[i]=='.'){
            count++;
            alpha = i;
        }

		if(count == 0 ) 
			flag = 0;
		if(email[0]=='@') {
			flag =0;
		}
		if(flag == 0) 
			printf("Wrong input. example: example@example.com\n");
	}while(flag == 0);

	//check password
	do{
		flag = 1;
		password = getpass("Password: ");
		if(strlen(password)<8 || strlen(password)>31){
			flag = 0;
			printf("Use at least 8 characters and less than 32 character.\n");
		}
	}while(flag ==0);

	
	//SEND USER COMMAND
	memset(&tempData,0,1024);
	strcpy(tempData,"USER ");
	strcat(tempData,email);
	strcat(tempData,enter);
	safeWrite(fd, tempData, strlen(tempData));
	
	//PASS
	memset(&readData, 0, 1024);
	memset(&tempData,0,1024);
	strcpy(tempData,"FORG ");
	strcat(tempData,password);
	strcat(tempData,enter);
	safeRead(fd, readData, 1024);
	safeWrite(fd, tempData, strlen(tempData));
	memset(&readData, 0, 1024);
	safeRead(fd, readData, 1024);
	printf("%s",readData);
	pressToContinue();
}

void cls(){
	printf("\e[1;1H\e[2J");
}

void settings(){
	char temp[100];
	printf("enter host(default: localhost): ");
	fgets( temp, sizeof(temp), stdin );
	temp[strlen(temp)-1] = '\0';
	strcpy(smtpUrl,temp);
	printf("enter smtp port(default: 7725): ");
	fgets( temp, sizeof(temp), stdin );
	temp[strlen(temp)-1] = '\0';
	SMTP_PORT = atoi(temp);
	printf("enter pop3 port(default: 7110): ");
	fgets( temp, sizeof(temp), stdin );
	temp[strlen(temp)-1] = '\0';
	POP3_PORT = atoi(temp);
	pressToContinue();
}