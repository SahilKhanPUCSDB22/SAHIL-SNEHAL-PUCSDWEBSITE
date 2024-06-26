#include"/root/website/headers/back.c"
#include<sodium.h>

#define INPUT_MAX 2

#define interviewerloginpage "/root/website/files/pages/interviewerloginpage.html"
#define loginfailurepage "/root/website/files/pages/loginfailurepage.html"

#define invalidusername "Username is invalid.Please try again\n"
#define invalidpassword "Password is invalid.Please try again\n"

#define sessionfailed "Failed to create session for the user\n"

#define type "i"

#define range 9

int send_login_errors(char *error,int cfd)
{
	char *buffer,*output;
	int content[1];

	if( gethtmlcontent(loginfailurepage,&buffer,content)!= GETHTML_OK )
	{
		logg("Page fetch failure(loginworker)\n");
		return 1;
	}

	content[0]+= mylen(error);

	output=malloc(sizeof(char)*content[0]);

	content[0]=sprintf(output,buffer,error);

	sendhtmlcontent(cfd,output,content[0]);

	free(buffer);
	free(output);
	return 0;
}

static enum erno db_lookup(char **input,int cfd)
{
	enum erno error;
	char j=0;

	int rows;

	char *command;

	const char *params[2];

	command="select * from interviewers where interviewers_id=$1";	

	params[0]=input[0];

	res=PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		err="Query execution failed fun:db_lookup()\n";
		logg(err);
		logg(PQerrorMessage(con));
		return VALUES_NA;
	}

	if( (rows=PQntuples(res))==0)
	{		
		if(send_login_errors(invalidusername,cfd))
		{
			logg("Failed to Send UserId error(loginworker)\n");
		}

		return UID_NA;
	}

	command="select * from interviewers where interviewers_id=$1 and interviewers_password=$2";

	params[1]=input[1];

	res=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		err="Query execution failed fun:db_lookup()\n";
		logg(err);
		return VALUES_NA;
	}

	if( (rows=PQntuples(res))==0)
	{
		if(send_login_errors(invalidpassword,cfd))
		{
			logg("Failed to Send UserId error(loginworker)\n");
		}

		return PASS_NA;
	}

	return VALUES_OK;
}

int send_session_error(int cfd)
{
	char *buffer,*output;
	int content[0];

	if( gethtmlcontent(loginfailurepage,&buffer,content)!= GETHTML_OK )
	{
		logg("Page fetch failure(loginworker)\n");
		return 1;
	}

	content[0]+= mylen(sessionfailed);

	output=malloc(sizeof(char)*content[0]);

	content[0]=sprintf(output,buffer,sessionfailed);

	if(sendhtmlcontent(cfd,output,content[0]!=SENDHTML_OK))
	{
		logg("Failed to send session error(loginworker)\n");
		return 1;
	}

	free(buffer);
	free(output);
	return 0;
}

static enum erno create_session(unsigned char nonce[],int cfd)
{
	int Len=NONCE-1;
	if(sodium_init()==-1)
	{
		err="sodium initialized failed fun:send_session()\n";
		logg(err);
		if(send_session_error(cfd))
		{
			logg("session error failed\n");
		}
		return SESSION_NA;
	}		

	randombytes_buf(nonce,Len);

	int i=0;
	while(i<Len)
	{
		nonce[i]= 48 + (nonce[i]%range);
		i++;
	}
	nonce[Len]=0;

	logg("session id :");
	logg((char*) nonce);
	logg("\n");

	return SESSION_OK;
}

enum erno update_session(const char* nonce,char *userid ,int cfd)
{
	char * command;
	const char * params[3];

	command="insert into sessions values ( $1 , $2 ,$3)";

	params[0]=nonce;
	params[1]=userid;
	params[2]=type;

	res=PQexecParams(con,(const char*)command,3,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		err="Query execution failed fun:update_session()\n";
		logg(err);
		logg(PQerrorMessage(con));

		if(send_session_error(cfd))
		{
			logg("send session er failed\n");
		}
		return UPDATE_NA;
	}

	return UPDATE_OK;
}

static enum erno send_page(char *input , unsigned char *nonce,int sockfd)
{
	char * buffer , *output ;
	int size[1];
	
	if(gethtmlcontent(interviewerloginpage,&buffer,size)!=GETHTML_OK)
	{
		logg("Login pg fetch failure(loginworker)\n");
		return PAGE_NA;
	}

	size[0]+=mylen(input)+mylen((char*)nonce)*2;

	output = malloc(sizeof(char)*size[0]);

	size[0] = sprintf(output,buffer,input,nonce,nonce);

	if(sendhtmlcontent(sockfd,output,size[0])!=SENDHTML_OK)
	{
		logg("Failed to send login page\n");
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}



int main(int argc , char* argv[])
{
	char* input[INPUT_MAX],flow=0;
	unsigned char nonce[NONCE];

	int streamlen=USERID+PASSWORD+INPUT_MAX;

	int size[]={USERID,PASSWORD},cfd;

	cfd  =  myatoi(argv[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("**********loginworker**********\n");

	if( connect_db() )
	{
		logg("Db connection failed (loginworker)\n");
		
		closeprog(cfd);
		
		return EXIT_FAILURE;
	}

	if(cfd!=-1)
	{
		if( getinput(cfd,input,streamlen,size,INPUT_MAX) !=INPUT_OK )	
		{
			flow=1;
		}

		if( (flow==0) && (db_lookup(input,cfd)!= VALUES_OK) )
		{
			flow=1;
		}

		if((flow==0) && (create_session(nonce,cfd)!=SESSION_OK)  )
		{
			flow=1;	
		}

		if( (flow==0) && (update_session((char*)nonce,input[0],cfd)!=UPDATE_OK))
		{
			flow=1;
		}

		if( (flow==0) && (send_page(input[0],(unsigned char*)nonce,cfd)!=PAGE_OK))
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);
	return 0;
}
