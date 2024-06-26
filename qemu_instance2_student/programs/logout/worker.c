#include"/root/website/headers/back.c"

#define INPUT_MAX 1

#define page "/root/website/files/pages/sessiondes.html" 

int destroy(int cfd , char *session)
{
	char* command = "delete from sessions where session_id = $1";

	char *params[]={session};

	res = PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg("failed to destroy session");
		return 0;
	}
	
	char* buffer;
	int size[1];
	
	if( gethtmlcontent(page,&buffer,size)!=GETHTML_OK)
	{
		logg("Failed to inform session destruction\n");
		return 0;
	}

	if( sendhtmlcontent(cfd,buffer,size[0])!=SENDHTML_OK)
	{
		logg("Failed to inform session destruction\n");
		return 0;
	}

	free(buffer);
	return 1;
}

int main( int argc , char* argv[] )
{
	int inpsize = SESSION_LEN+INPUT_MAX,cfd=myatoi(argv[1]);

        int size[]={SESSION_LEN};	
	
	char* input[INPUT_MAX],flow=0;

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("******logoutworker*********\n");

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
		
		if( (flow==0) && destroy(cfd,input[0])!=1)
		{
			flow=1;
		}	
	}

	closedb();
	closeprog(cfd);
	return 0;
}

