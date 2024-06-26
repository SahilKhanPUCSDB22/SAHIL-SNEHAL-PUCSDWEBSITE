#include"/root/website/headers/back.c"

#define INPUT_MAX 4 

#define page "/root/website/files/pages/testdone.html" 

#define query "update studenttest set status='SUBMITTED' , submitdate = $3::date , submittime=$4::time , attempts = cast(cast(attempts as integer)+1 as text)  where testid=$1 and studid=$2"

static enum erno submit(char** input , char ** userdetails )
{
	char *params[]={input[1],userdetails[0],input[2],input[3]};

	res=PQexecParams(con,query,4,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!= PGRES_COMMAND_OK )
	{
		logg("Failed to Submit Test\n");
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}
	
	return INSERT_OK;
}

static enum erno send_page(int cfd)
{
	char  * buffer;
	int size[1];
	if(gethtmlcontent(page , &buffer ,size ) !=GETHTML_OK)
	{
		logg("Failed to Open Page\n");
		return SEND_NA;
	}

	if(sendhtmlcontent(cfd,buffer,size[0])!=SENDHTML_OK)
	{
		logg("Failed to Send Page");
		return SEND_NA;
	}
	
	free(buffer);
	return SEND_OK;
}

int main( int argc , char* argv[] )
{
	int inpsize = SESSION_LEN + TESTID + DATE + TIME , cfd=myatoi(argv[1]);

        int size[]={SESSION_LEN,TESTID,DATE,TIME};	
	
	char* input[INPUT_MAX],*udetails[2],flow=0;

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("--------SubmitTestWorker-------\n");

	if(connect_db())
	{
		logg("db opening failed\n");
	}

	if(cfd!=-1)
	{
		if( getinput(cfd,input,inpsize,size,INPUT_MAX) != INPUT_OK )
		{
			flow=1;
		}	

		if( flow==0  && getuserdetails(input[0],udetails) != USER_OK )
		{
			flow=sessionexpired(cfd);
		}
		
		if(flow==0 && submit(input,udetails) != INSERT_OK)
		{
			flow=1;
		}

		if(flow==0 && send_page(cfd)!=SEND_OK)
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);
	return 0;
}

