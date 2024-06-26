#include"/root/website/headers/back.c"

#define portown 1233
#define worker "/root/website/programs/signup/worker.cgi"

int main(void)
{
	enum erno er;
	int cfd,pc=0;
	char *args[3],fd[CAPACITY];

	args[0]=worker;
	args[2]=NULL;
	
	if(open_log())
	{
		return EXIT_FAILURE;	
	}

	logg("**********signupback**********\n");
	
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
					logg("failed to spawn(loginback)");
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
