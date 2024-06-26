#include"/root/website/headers/back.c"

#define INPUT_MAX 4

#define manswer "delete from manswer where testid=$1 and studid=$2 and qno=$3"
#define danswer "delete from danswer where testid=$1 and studid=$2 and qno=$3"
#define fanswer "delete from fanswer where testid=$1 and studid=$2 and qno=$3"

#define ok "Answer Cleared"

static enum erno deleteold( char **input ,char *studid)
{
	char *command,*params[]={input[1],studid,input[3]};

	if(input[2][0]=='M')
	{
		command=manswer;
	}

	if(input[2][0]=='D')
	{
		command=danswer;
	}

	if(input[2][0]=='F')
	{
		command=fanswer;
	}

	res=PQexecParams(con,(char*)command,3,NULL,(const char* const*)params,NULL,NULL,0);
			
	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}

static enum erno send_page(int cfd )
{
	if(sendhtmlcontent(cfd,ok,mylen(ok))!=SENDHTML_OK)
	{
		return PAGE_NA;
	}

	return PAGE_OK;
}


int main(int argc , char *fd[])
{
	enum erno er;

	int cfd,inpsize = TESTID+SESSION_LEN+QTYPE+QNO , size[] = {SESSION_LEN,TESTID,QTYPE,QNO};

	char *input[INPUT_MAX],*userdetails[2];

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

		if( flow==0 && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}
		
		if(flow==0 && (deleteold(input,userdetails[0])!=INSERT_OK))
		{
			flow=1;
		}

		if((flow==0) && (send_page(cfd)!=PAGE_OK))
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);
	return 0;
}
