#include"/root/website/headers/back.c"

#define access "/root/website/files/pages/access.html"

#define INPUT_MAX 2

static enum erno send_page(int cfd ,char** input)
{
	char *buffer,*output;
	int contentsize[1];

	if(gethtmlcontent(access,&buffer,contentsize)!=GETHTML_OK)
	{
		return PAGE_NA;
	}

	contentsize[0]+=mylen(input[0])+mylen(input[1]);
	output=malloc(sizeof(char)*contentsize[0]);

	contentsize[0]=sprintf(output,buffer,input[0],input[1]);

	if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
	{
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}

int main(int argc , char *fd[])
{
	int cfd , inpsize = SESSION_LEN + TESTID ;  

	int size[]={ SESSION_LEN,TESTID};

	char *input[INPUT_MAX],*userdetails[2];

	cfd = myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("------ProceedWorker----\n");

	if(connect_db())
	{
		return EXIT_FAILURE;
	}

	char flow=0;

	if(cfd  != -1)
	{

		if( getinput(cfd,input,inpsize,size,INPUT_MAX) != INPUT_OK )
		{
			flow=1;
		}


		if( (flow==0) && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}

		if((flow==0) && (send_page(cfd,input)!=PAGE_OK))
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);

	return 0;
}
