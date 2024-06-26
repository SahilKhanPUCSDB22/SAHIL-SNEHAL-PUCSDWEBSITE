//MACROS
#define klog "/usr/local/www/apache24/log/errlog"
#define CAPACITY 1024

#define SESSION_LEN 32
#define USERID 1024
#define PASSWORD 1024
#define SFNAME 1024
#define SLNAME 1024
#define ADDRESS 1024
#define SSTREAM 1024
#define DEGREE 1024
#define COLLEGEID 1024


#define FNAME_LEN 1024
#define FTYPE_LEN 1024
#define UTYPE 10
#define FSIZE_LEN 1024
#define SEG_SIZE 200

#define TESTID 1024
#define QTYPE 2
#define QNO 1024
#define QLEN 5
#define OPTIONSIZE 1024

#define TIME 10
#define DATE 12
#define TESTTYPE 2
#define RANDOM 2
#define ATTEMPTS 2
#define TYPE 2
#define REVIEW 2

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
#include<kcgi.h>
#include"functions.c"

//VARIABLES
int sockfd;
FILE* logp;
char* err;

//FUNCTIONS
//LOG
void logg(char *err)
{
	fwrite((void*)err,sizeof(char),strlen(err),logp);
}

int open_log()
{
        logp=fopen(klog ,"a");

        if ( logp == NULL)
        {
                return 1;
        }
	else
	{
		err="*********log ok************\n";
		logg(err);
	}

        return 0;
}

enum erno {INPUT_NA,INPUT_OK,FILE_BIG,SEND_OK,SEND_NA,PAGE_OK,PAGE_NA,CONNECTION_NA,CONNECTION_OK,STREAM_OK,STREAM_NA};

static enum erno chk_err(char er)
{
	enum erno re;	
	return re;
}

enum page {
	PAGE_INDEX,
	PAGE__MAX
};
const char *const pages[PAGE__MAX] = {
	"index" /* PAGE_INDEX */
};

void emit_header(struct kreq * r)
{
	khttp_head(r, kresps[KRESP_STATUS],"%s", khttps[KHTTP_200]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE],"%s", kmimetypes[r->mime]);
}

void emit_error(struct kreq * r, int er)
{
	khttp_head(r, kresps[KRESP_STATUS],"%s", khttps[er]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE],"%s", kmimetypes[KMIME_TEXT_HTML]);
	
	khttp_body(r);

	if ( KMIME_TEXT_HTML == r->mime )
	{
		khttp_puts(r, "Could not service request.");
	}
}

static enum khttp sanitise(const struct kreq *r) {
	if (r->page != PAGE_INDEX)
		return KHTTP_404;
	else if (*r->path != '\0') /* no index/xxxx */
		return KHTTP_404;
	else if (r->mime != KMIME_TEXT_HTML)
		return KHTTP_404;
	else if (r->method != KMETHOD_POST)
		return KHTTP_405;
	return KHTTP_200;
}

static enum erno check_input(struct kreq* r,int KEY__MAX)
{
	char i=0;

	struct kpair *p;

	while(i<KEY__MAX)
	{
		if( (p=r->fieldmap[i]))
		{
			i++;
		}
		else
		{
			return INPUT_NA;
		}
	}

	logg("input ok\n");

	return INPUT_OK;
}

static enum erno connect_back(int portnum)
{
	struct sockaddr_in back;

	back.sin_family=AF_INET;
	back.sin_port=htons(portnum);
	back.sin_addr.s_addr=INADDR_ANY;

	if( (sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err="socket creation error\n";
		logg(err);
		return CONNECTION_NA;
	}
	else
	{
		err="socket ok\n";
		logg(err);
	}

	int rr=1,l,size;

	while(rr!=0)
	{
		if((rr=connect(sockfd,(const struct sockaddr*)&back,(socklen_t) sizeof(back)))<0)
		{
			err="connection error";
			logg(err);
			return CONNECTION_NA;
		}
	}

	err="connect ok\n";
	logg(err);
	return CONNECTION_OK;
}

static enum erno create_stream(char **input , char INPUT_STREAM[],int num_inp)
{
	mycpy(INPUT_STREAM,input[0]);
	
	int i=1;
	
	while(i<num_inp)
	{
		if( !myappend(INPUT_STREAM,input[i++] ) )
		{
			return STREAM_NA;
		}
	}

	logg("stream ok\n");
	
	return STREAM_OK;
}


static enum erno get_page(struct kreq *r)
{
	unsigned int i=1,n=0;
	
	char *buffer = malloc(sizeof(char)*(CAPACITY+1));

	while(i!=0)
	{
		n= recv(sockfd, (void*)buffer , CAPACITY , 0);
		
		buffer[n]=0;

		if(n<=0)
		{
			err="Failed to recieve data fun\n";
			logg(err);
			return PAGE_NA;
		}	

		if(n<CAPACITY)
		{
			i=0;
		}

		khttp_puts(r,buffer);
	}

	free(buffer);

	logg("page ok\n");

	return PAGE_OK;
}

static enum erno send_stream(char * stream , int size )
{
	int n,ptr=0;

	while(size>0)
	{
		if(size < CAPACITY)
		{
			n = size;
		}
		else
		{
			n = CAPACITY;
		}

		if( ( n=send( sockfd , &stream[ptr] , n ,0 )) < 0)
		{
			logg("Failed to send data\n");
			return STREAM_NA;
		}	

		ptr+=n;
		size-=n;
	}

	logg("dstream send\n");

	return STREAM_OK;
}

void closeprog()
{
	logg("*********log close**********\n");
	close(sockfd);
	fclose(logp);
}






