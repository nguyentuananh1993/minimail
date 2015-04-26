#include <stdio.h>
#include <unistd.h>   
#include <string.h>

int main()
{
    char email[512] ={0};
    int count = 0;
    int i;
    int alpha;
    int flag = 1;
    printf("enter email:");
    fgets( email, sizeof(email), stdin);
    printf("|%s|",email);

}