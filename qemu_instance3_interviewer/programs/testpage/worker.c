#include"/root/website/headers/back.c"

#define createtest "/root/website/files/pages/createtest.html"
#define INPUT_MAX 1

static enum erno send_page(char *input ,int sockfd)
{
	char * buffer , *output ;
	int size[1];
	
	if(gethtmlcontent(createtest,&buffer,size)!=GETHTML_OK)
	{
		logg("Login pg fetch failure(loginworker)\n");
		return PAGE_NA;
	}

	size[0]+=mylen(input);

	output = malloc(sizeof(char)*size[0]);

	size[0] = sprintf(output,buffer,input);

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

	int streamlen= SESSION_LEN + INPUT_MAX;

	int size[]={SESSION_LEN},cfd;

	cfd  =  myatoi(argv[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("-----TestPageWorker------\n");

	if(connect_db())
	{
		logg("Db connection failed (loginworker)\n");
		return EXIT_FAILURE;
	}


	if(cfd!=-1)
	{
		if( getinput(cfd,input,streamlen,size,INPUT_MAX) !=INPUT_OK )	
		{
			flow=1;
		}

		if( (flow==0) && (send_page(input[0],cfd)!=PAGE_OK))
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);
	return 0;
}
