#include "/root/website/headers/back.c"

#define INPUT_MAX 4

#define testsubmited "/root/website/files/pages/testsubmited.html"

#define getlimit "select charlimit from dquestion where testid=$1 and qno=$2"

#define delmcq "delete from manswer where testid=$1 and studid=$2 and qno=$3"

#define deldes "delete from danswer where testid=$1 and studid=$2 and qno=$3"

#define insmcq "insert into manswer values($1,$2,$3,$4)"

#define insdes "insert into danswer values($1,$2,$3,$4)"

#define inserted "Your answer was recorded"

int getanswer(int cfd , char ** answer ,int size)
{
	*answer = malloc(sizeof(char)*size);
	
	int n;
	
	if( (n=recv(cfd,(*answer),size,0) )<= 0 )
	{
		logg("Failed to recieve answer\n");
		return 1;
	}
	
	(*answer)[n]=0;
	
	return 0;
}

static enum erno getqanswer(int cfd , char **input , char** answer)
{
	int size;
	char *msg;

	if(input[2][0]=='M')
	{
		size=CAPACITY;
	}
	else
	{
		char *params[2]={input[1],input[3]};
		
		res=PQexecParams(con,(char*)getlimit,2,NULL,(const char* const*)params,NULL,NULL,0);
		
		if(PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			return INSERT_NA;
		}

		if(PQntuples(res)>0)
		{
			msg=PQgetvalue(res,0,0);
			size = myatoi(msg);
		}
		else
		{
			return SEND_NA;
		}
	}
	
		
	if( getanswer(cfd,answer,size++) )
	{
		return SEND_NA;
	}

	return SEND_OK;
}

static enum erno deleteold(char **input ,char *studid)
{
	char *command;

	char *params[]={input[1],studid,input[3]};
	
	if(input[2][0]=='M')
	{
		command=delmcq;
	}
	else
	{
		command=deldes;
	}
	
	res=PQexecParams(con,(char*)command,3,NULL,(const char* const*)params,NULL,NULL,0);
			
	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}

static enum erno insertdb(char **input,char * answer , char *userdetails[2])
{
	char *command;

	char *params[]={userdetails[0],input[1],input[3],answer};

	if(input[2][0]=='M')
	{
		command=insmcq;
	}
	else
	{

		command=insdes;
	}
		
	res=PQexecParams(con,(char*)command,4,NULL,(const char* const*)params,NULL,NULL,0);
			
	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}

static enum erno send_page(int cfd ,char** input)
{	
	if(sendhtmlcontent(cfd,inserted,mylen(inserted))!=SENDHTML_OK)
	{
		return PAGE_NA;
	}
	return PAGE_OK;
}

int main(int argc , char *fd[])
{
	enum erno er;

	int cfd,inpsize = TESTID+ SESSION_LEN + QTYPE+ QNO , size[] = { SESSION_LEN , TESTID , QTYPE , QNO};

	char *input[INPUT_MAX],*userdetails[2],*answer;

	cfd =myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	if(connect_db())
	{
		return EXIT_FAILURE;
	}

	char flow=0;
	if(cfd!=-1)
	{

		if( getinput(cfd,input,inpsize,size,INPUT_MAX) == INPUT_NA )
		{
			flow=1;
		}

		if( flow==0 && (getuserdetails(input[0],userdetails))!=USER_OK)
		{
			flow=sessionexpired(cfd);
		}
		
		if( flow==0 && getqanswer(cfd,input,&answer)!=SEND_OK)
		{
			flow=1;
		}

		if(flow==0 && (deleteold(input,userdetails[0]))!=INSERT_OK)
		{
			flow=1;
		}

		if(flow==0 && (insertdb(input,answer,userdetails))!=INSERT_OK)
		{
			flow=1;
		}

		if((flow==0) && (send_page(cfd,input))!=PAGE_OK)
		{
			flow=1;
		}
		
	}
	closedb();
	closeprog(cfd);
	return 0;
}
