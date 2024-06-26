#include"/root/website/headers/back.c"

#define portown 1241
#define worker "/root/website/programs/loadquestion/worker.cgi"

int main(void)
{
	int cfd,pc=0;
	char *args[3],fd[CAPACITY];

	args[0]=worker;
	args[2]=NULL;
	
	if(open_log())
	{
		return EXIT_FAILURE;	
	}

	logg("**********loadquestionback**********\n");
	
	if((sockfd=connect_front(portown))==0)
	{
		return EXIT_FAILURE;
	}

	while(1)
	{
		cfd = acceptconnection(sockfd); 

		if(cfd!=-1)
		{
			myitoa(cfd,fd);
			
			if(!fork())
			{
				args[1]=(char*)fd;

				if( execv(worker,(char**)args) <0 )
				{
					logg("failed to spawn");
				}
				else
				{
					logg("exec ok\n");
				}
			}	
		}
		else
		{
			logg("conn closed\n");
		}
		
		close(cfd);

		pc++;

		if(pc==PROCESSLIMIT)
		{
			while(pc>0)
			{
				wait(NULL);
				pc--;
			}
		}
	}
	closeprog(sockfd);
	return 0;
}
