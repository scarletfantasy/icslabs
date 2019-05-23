#define _POSIX_C_SOURCE 2
#include "cachelab.h"
#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include <malloc.h>
#include<math.h>

int visit(long,int);
void init_cache(int,int,int);
void printsituation(int);
void showhelp();
/*the struct of cache*/
struct vcache
{
    int s,e,b,t,v;
    long * line;
    int **count;
    
} cache;
bool v=false;
int hit=0,miss=0,evicton=0;
int main(int argc, char *argv[])
{
    
    
    
    extern char *optarg;
    extern int optind, opterr, optopt;
    int ch;
    int s=0,e=0,b=0;
    
    char* t;
    /*read the command*/
    while((ch=getopt(argc,argv,"hvs:E:b:t:"))!=-1)
    {       
            switch (ch)
            {
                case 'h':
                {
                    showhelp();
                    return 0;
                }
                case 'v':
                {                   
                    v=true;
                    break;
                }
                case 's':
                {
                    
                    
                    s=atoi(optarg);                    
                    break;
                }
                case 'E':
                {
                    e=atoi(optarg);
                    break;
                }
                case 'b':
                {
                    b=atoi(optarg);
                    break;
                }
                case 't':
                {
                    t=optarg;
                    break;
                }

            
                default:
                    break;
            }
        
    }
    
    
    init_cache(s,b,e);    
    FILE* read=fopen(t,"r");   
    char* taddr=(char*)malloc(sizeof(char)*255);        
    char* addr=(char*)malloc(sizeof(char)*255);
    long tag;
    int set=0;
    
    /*read the file*/
    while(fgets(taddr,100,read))
    {
        /*except I*/
        if(taddr[0]!=' ')
        {
            continue;
        }
        /*get the tag and set*/
        long x;
        addr=taddr;
        ++addr;
        ++addr;
        sscanf(addr,"%lx",&x);       
        set=(x>>b)-(x>>(b+s)<<s);
        tag=(x>>(b+s));        
        int loc=0;
        while(taddr[loc]!='\n')
        {
            ++loc;
        }
        taddr[loc]='\0';
        
        if(v)
        {
            printf("%s",(taddr+1));
        }
        
        /*check the command*/
        switch (taddr[1])
        {
            case 'M':
            {
                visit(tag,set);
                visit(tag,set);
                break;

            }
            case 'S':
            {
                visit(tag,set);
                break;
            }
            case 'L':
            {
                visit(tag,set);
                break;
            }
                
        
            default:
                break;
        }
        if(v)
        {
            printf("\n");
        }
        
    
    }
    
    printSummary(hit, miss, evicton);
    
}
/*the help information*/
void showhelp()
{
    printf("-h help\n-v show the trace\n-s set\n-E \n-b block\n-t tracefile\n");
}

/*initialize the cache*/
void init_cache(int s,int b,int e)
{
    const int S=(int) pow((double)2,(double)s);
    /*use long to store the lines*/
    long *line=(long*)malloc(sizeof(long)*S*e);
    cache.line=line;
    /*store the basic information*/
    cache.s=s;
    cache.b=b;
    cache.e=e;
    /*use count to check the lru 0 is the symbol of invaild and other number is the order*/
    int **count=(int **)malloc(sizeof(int*)*S);
    for(int i=0;i<S;++i)
    {
        count[i]=(int*)malloc(sizeof(int)*e);
        for(int j=0;j<e;++j)
        {
            count[i][j]=0;
        }
    }
    cache.count=count;
    
}
/*simulate visiting the cache*/
int visit(long tag,int s)
{
    int e=cache.e;
    bool flag=false;
    int tmp=0;
    /*check if there is the same tag*/
    for(int i=0;i<e;++i)
    {
        if(cache.count[s][i]!=0&&cache.line[s*e+i]==tag)
        {
            cache.line[s*e+i]=tag;
            tmp=cache.count[s][i];
            cache.count[s][i]=1;

            for(int j=0;j<e;++j)
            {
                if(cache.count[s][j]<tmp&&cache.count[s][j]!=0&&i!=j)
                {
                    cache.count[s][j]++;
                }
            }
            flag=true;
        }       
    }
    if(flag){
        
        ++hit;
        if(v)
        {printf(" hit");}
        return 1;
    }
    /*check if it has invalid place*/
    
    flag=false;
    for(int i=0;i<e;++i)
    {
        if(cache.count[s][i]==0)
        {
            cache.line[s*e+i]=tag;
            cache.count[s][i]=1;
            for(int j=0;j<e;++j)
            {
                if(cache.count[s][j]!=0&&i!=j)
                {
                    cache.count[s][j]++;
                }
            }
            flag=true;
            break;
        }
    }
    if(flag)
    {
        ++miss;
        if(v)
        {
            printf(" miss");
        }
        
        return 0;
    }
    /*actualize the eviction*/
    for(int i=0;i<e;++i)
    {
        if(cache.count[s][i]==e)
        {
            cache.line[s*e+i]=tag;
            cache.count[s][i]=1;
            for(int j=0;j<e;++j)
            {
                if(cache.count[s][j]!=0&&i!=j)
                {
                    cache.count[s][j]++;
                }
            }
            flag=true;
            break;
        }
    }
    ++evicton;
    ++miss;
    if(v)
    {
        printf(" miss eviction");
    }
    
    return 3;
    
}



