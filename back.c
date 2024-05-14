//MACROS
#define errlog "/root/website/log/errlog"
#define CAPACITY 1024
#define SEG_SIZE 200
#define SESSION_LEN 32
#define FNAME_LEN 1024
#define FTYPE_LEN 1024
#define FSIZE_LEN 10
#define UTYPE_LEN 4
#define TESTID 1024
#define TIME 10
#define DATE 12

//HEADERFILES
#include<stdarg.h>
#include<stddef.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<errno.h>
#include<libpq-fe.h>
#include<ctype.h>
#include"functions.c"


//ERRORS
enum erno{INPUT_OK,INPUT_NA,VALUES_OK,VALUES_NA,UID_NA,PASS_NA,DATA_F,SESSION_OK,SESSION_NA,PAGE_OK,PAGE_NA,UPDATE_NA,UPDATE_OK,USER_OK,USER_NA,GETHTML_OK,SENDHTML_OK,};

//VARIABLES 
int sockfd;

FILE* logp;
char *err;

PGconn* con;
PGresult* res;

struct sockaddr_in senaddr;
socklen_t LEN;

//FUNCTIONS
void logg(char *err)
{
	fwrite((void*)err,sizeof(char),mylen(err),logp);
}

void send_error(enum erno er,int sockfd)
{
	char buffer[1];
	if(er==PASS_NA)
	{
		buffer[0]='p';
	}
	else if(er==UID_NA)
	{
		buffer[0]='u';
	}
	else
	{
		buffer[0]='d';
	}

	int n=send(sockfd,(void*)buffer,1,0);

	if( n < 0)
	{
		err="failed to send error fun:send_error\n";
		logg(err);
	}
}

int open_log()
{
	logp=fopen(errlog ,"a");

	if ( logp == NULL)
	{
		return 1;
	}
	else
	{
		err="*********log ok********\n";
		logg(err);
	}
	return 0;
}

int connect_db()
{
	con=PQconnectdb("host=192.168.1.10 dbname=pucsd_hackerrank user=postgres password=1234 port=5432");
	
	if(PQstatus(con)!=CONNECTION_OK)
	{
		logg(PQerrorMessage(con));
		err="Database connection failed\n";
		logg(err);
		return 1;
	}
	else
	{
		err="db connection ok\n";
		logg(err);
	}

	return 0;
}

int connect_front(int portown)
{
	int sockfd;
	senaddr.sin_family=AF_INET;
	senaddr.sin_port=htons(portown);
	senaddr.sin_addr.s_addr=INADDR_ANY;

	LEN = sizeof(senaddr);

	if( (sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err="Socket syscall failed\n";
		logg(err);
		return 0;
	}

	if( bind(sockfd,(const struct sockaddr*)&senaddr,LEN) <0)
	{
		err="Binding failed \n";
		logg(err);
		return 0;
	}

	if( listen(sockfd,9999999) != 0)
	{
		err="cannot listen\n";
		logg(err);
		return 0;
	}

	err="connection ok\n";

	logg(err);

	return sockfd;
}

int acceptconnection(int sockfd)
{
	return accept(sockfd,(struct sockaddr*)&senaddr,(socklen_t*) &LEN);
}

int seggregate(char *stream,char **input,int size[],int INPUT_MAX)
{
        int i=0,j=0,k;
                
        while(i<INPUT_MAX)
        {       
                input[i]=(char*)malloc(sizeof(char)*(size[i]+1));
                i++;
        }

        i=0;

        while(i<INPUT_MAX)
        {       
                k=0;
                while(stream[j]!=10)
                {
                        input[i][k]=stream[j];
                        k++;
                        j++;
                }
                input[i][k]=0;
                i++;
                j++;
        }

        return 1;
}

enum erno getinput(int sockfd,char **input,int inpsize,int size[],int INPUT_MAX)
{
        int len;
        char INP_STREAM[inpsize+1];

        if( (len=recv(sockfd,INP_STREAM,inpsize,0))<0)
        {
                err="Failed to recieve inputs\n";
                logg(err);
                return INPUT_NA;
        }

        INP_STREAM[len]=0;

        if(seggregate(INP_STREAM,input,size,INPUT_MAX)<0)
        {
                return INPUT_NA;
        }

	len=0;
	while(len<INPUT_MAX)
	{
		logg(input[len++]);
		logg("#");
	}

	logg("input ok\n");
        return INPUT_OK;
}

enum erno getuserdetails(char* session,char** udetails)
{
        //sanitize db and retrieve partnois

        char * command;
        char *params[1];

        command="select userid from sessions where session_id=$1";

        params[0]=session;

        res=PQexecParams(con,(char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

        if(PQresultStatus(res)!=PGRES_TUPLES_OK)
        {
                logg(PQerrorMessage(con));
                return USER_NA;
        }

        int rows=PQntuples(res),cols=PQnfields(res);

        if(cols>0 && rows >0)
        {
                udetails[0]=(char*)PQgetvalue(res,0,0);
                udetails[1]=session;
        }
        else
        {
                err="Userid fetch failure\n";
                logg(err);
                return USER_NA;
        }


	logg("user ok\n");

	return USER_OK;
}

static enum erno gethtmlcontent(char *htmlpage , char **buffer,int contentsize[])
{
	FILE* fp = fopen( htmlpage ,"r");

        if(fp==NULL)
        {
                err="failed to open html page\n";
                logg(err);
                return PAGE_NA;
        }

	logg( mycat(htmlpage," opened\n"));

        fseek(fp,0,SEEK_END);

        contentsize[0] = ftell(fp);

        rewind(fp);

	*buffer= malloc ( sizeof(char)*contentsize[0] );

        fread((void*)*buffer,sizeof(char),contentsize[0],fp);

        fclose(fp);

	logg("content ok\n");

	return GETHTML_OK;
}

static enum erno sendhtmlcontent(int sockfd , char *buffer , int size)
{
	int nb,n,ptr=0;

        while(size>0)
        {
                if(size<CAPACITY)
                {
                        n=size;
                }
                else
                {
                        n=CAPACITY;
                }

                nb=send(sockfd,(buffer+ptr),n,0);
                
		if( nb < 0 )
                {
                        err="Failed to send page fun:send_page()\n";
                        logg(err);
                        return PAGE_NA;
                }

                ptr+=n;
                size-=n;
        }

	logg("send ok\n");

	return SENDHTML_OK;
}

void closeprog()
{
	logg("close\n");
	close(sockfd);
	PQfinish(con);
	fclose(logp);
}
