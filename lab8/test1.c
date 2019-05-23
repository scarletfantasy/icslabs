#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>

int main()
{
    FILE* read=fopen("traces/yi.trace","r");
    
    char* taddr=(char*)malloc(sizeof(char)*255);
    char* addr=(char*)malloc(sizeof(char)*255);
    char command;
    int bit;
    
    long tag;
    while(fgets(taddr,100,read))
    {
        /*if(taddr[0]==' ')
        {
            printf("%s",taddr);
        }
        int loc=3;
        while(taddr[loc]!=',')
        {
            addr[loc-3]=taddr[loc];
            ++loc;
        }
        */
        long x;
        taddr++;
        taddr++;
        sscanf(taddr,"%lx",&x);
        printf("%ld",x);
    }
}