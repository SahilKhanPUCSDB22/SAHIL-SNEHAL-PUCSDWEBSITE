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

#define errlog "/root/website/log/errlog"
#define CAPACITY 1024
#define SEG_SIZE 200
#define SESSION_LEN 32
#define FNAME_LEN 1024
#define FTYPE_LEN 1024
#define FSIZE_LEN 10
#define UTYPE_LEN 4

enum erno{INPUT_OK,INPUT_NA,VALUES_OK,VALUES_NA,UID_NA,PASS_NA,DATA_F,SESSION_OK,SESSION_NA,PAGE_OK,PAGE_NA,UPDATE_NA,UPDATE_OK};

FILE* logp;
char *err;

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

enum erno getinput(int sockfd,char **input,int inpsize)
{
        int len;
        char INP_STREAM[inpsize+1];

        if( (len=recv(sockfd,INP_STREAM,inpsize,0))<0)
        {
                err="Failed to recieve inputs from kcgi fun:getinput()\n";
                logg(err);
                return INPUT_NA;
        }

        INP_STREAM[len]=0;

        if(seggregate(INP_STREAM,input)<0)
        {
                return INPUT_NA;
        }

        return INPUT_OK;
}

enum erno userdetails(char** input,char** udetails)
{
        //sanitize db and retrieve partnois

        char * command;
        char *params[1];

        char *utype = input[1];

        command="select userid,usertype from sessions where session_id=$1";

        params[0]=input[0];

        res=PQexecParams(con,(char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

        if(PQresultStatus(res)!=PGRES_TUPLES_OK)
        {
                logg(PQerrorMessage(con));
                return SAN_NA;
        }

        int rows=PQntuples(res),cols=PQnfields(res);

        if(cols>0 && rows >0)
        {
                udetails[0]=(char*)PQgetvalue(res,0,0);
                udetails[1]=PQgetvalue(res,0,1);
        }
        else
        {
                err="Userid , Type fetch failure\n";
                logg(err);
                return SAN_NA;
        }

	return SAN_OK;
}

static enum gethtmlcontent(char *htmlpage , char *buffer)
{
	FILE* fp = fopen( htmlpage ,"r");

        if(fp==NULL)
        {
                err="failed to html open page\n";
                logg(err);
                return GETHTML_NA;
        }

        fseek(fp,0,SEEK_END);

        int size = ftell(fp);

        rewind(fp);

	buffer=malloc(sizeof(char)*size);

        fread((void*)buffer,sizeof(char),size,fp);

        fclose(fp);

	return GETHTML_OK;
}

static enum sendhtmlcontent(int sockfd , char *buffer , int size)
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

	return SENDHTML_OK;
}
