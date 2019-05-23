/*
 * proxy.c - ICS Web proxy
 *
 *
 */

#include "csapp.h"
#include <stdarg.h>
#include <sys/select.h>
typedef struct threadarg
{
    int fd;
    
    SA clientaddr;
} threadargs;
sem_t mutex;
/*
 * Function prototypes
 */
ssize_t rio_readlineb_w(rio_t *rp,void* usrbuf,size_t maxlen)
{
    ssize_t ssize;
    if((ssize=rio_readlineb(rp,usrbuf,maxlen))<0)
    {
        ssize=0;
        fprintf(stderr,"readline error");
    }
    return ssize;
}
ssize_t rio_readnb_w(rio_t *rp,void* usrbuf,size_t n)
{
    ssize_t ssize;
    if((ssize=rio_readnb(rp,usrbuf,n))<0)
    {
        ssize=0;
        fprintf(stderr,"readn error");
    }
    return ssize;
}
void rio_writen_w(int fd,void*usrbuf,size_t n)
{
    if(rio_writen(fd,usrbuf,n)!=n)
    {
        fprintf(stderr,"writen error");
    }
}
int parse_uri(char *uri, char *target_addr, char *path, char *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, size_t size);
void doit(int fd,SA* sockaddr)
{
    int is_static,serverfd,n,size=0,content_length=0;
    struct stat sbuf;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE],cgiargs[MAXLINE],requesthead[MAXLINE];
    char hostname[MAXLINE],pathname[MAXLINE],port[MAXLINE];
    rio_t rio,srio;
    rio_readinitb(&rio,fd);
    if(rio_readlineb(&rio,buf,MAXLINE)==0)
    {
        close(fd);
        return;
    }
    
    if(sscanf(buf,"%s %s %s",method,uri,version)!=3)
    {
        close(fd);
        return;
    }
    
    if(parse_uri(uri,hostname,pathname,port)!=0)
    {
        close(fd);
        return;
    }
    
    serverfd=open_clientfd(hostname,port);
    rio_readinitb(&srio,serverfd);
    sprintf(requesthead,"%s /%s %s\r\n",method,pathname,version);
    rio_writen_w(serverfd,requesthead,strlen(requesthead));
    while((n=rio_readlineb_w(&rio,buf,MAXLINE))>0)
    {
        
        rio_writen_w(serverfd,buf,strlen(buf));
        if(!strcmp("\r\n",buf))
        {
            break;
        }
        if(!strncmp("Content-Length: ", buf, 16))
        {
            sscanf(buf+16, "%d", &content_length);
        }
        
    }
    if(strcmp("GET",method))
    {
        if(content_length!=0)
        {
            for(int i=0;i<content_length;++i)
            {
                rio_readnb_w(&rio,buf,1);
                rio_writen_w(serverfd,buf,1);
            }
        }
        
        
    }
    while((n=rio_readlineb_w(&srio,buf,MAXLINE))>0)
    {
        size+=n;
        rio_writen_w(fd,buf,strlen(buf));
        if(!strcmp("\r\n",buf))
        {
            break;
        }
        if(!strncmp("Content-Length: ", buf, 16))
        {
            sscanf(buf+16, "%d", &content_length);
        }
    }
    size+=content_length;
    if(content_length!=0)
    {
        for(int i=0;i<content_length;++i)
        {
            rio_readnb_w(&srio,buf,1);
            rio_writen_w(fd,buf,1);
        }
    }
    
    
    char logstring[MAXLINE];
    format_log_entry(logstring,sockaddr,uri,size);
    /*int filefd=open("proxy.log",O_RDWR,O_APPEND);
    rio_writen(filefd,logstring,strlen(logstring));
    close(filefd);*/
    P(&mutex);
    printf("%s\n",logstring);
    V(&mutex);
    close(serverfd);
    close(fd);
    
    

}
void *thread(void *vargp)
{
    Pthread_detach(Pthread_self());
    threadargs *args=(threadargs *)vargp;
    doit(args->fd,&(args->clientaddr));
    return NULL;
}

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }
    pthread_t tid;
    char hostname[MAXLINE],port[MAXLINE];
    int listenfd,connfd;
    
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    threadargs *args;
    Sem_init(&mutex,0,1);
    Signal(SIGPIPE, SIG_IGN);
    listenfd=open_listenfd(argv[1]);
    while(1)
    {
        args=Malloc(sizeof(threadargs));
        
        clientlen=sizeof(args->clientaddr);
        connfd=Accept(listenfd,(SA*)&(args->clientaddr),&clientlen);
        Getnameinfo((SA*)&(args->clientaddr),clientlen,hostname,MAXLINE,port,MAXLINE,0);
        args->fd=connfd;
        
        Pthread_create(&tid,NULL,thread,args);
        
        
    }
    close(listenfd);
    exit(0);
}


/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, char *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    if (hostend == NULL)
        return -1;
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    if (*hostend == ':') {
        char *p = hostend + 1;
        while (isdigit(*p))
            *port++ = *p++;
        *port = '\0';
    } else {
        strcpy(port, "80");
    }

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;
        strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), the number of bytes
 * from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, size_t size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /*
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 12, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %zu", time_str, a, b, c, d, uri, size);
}


