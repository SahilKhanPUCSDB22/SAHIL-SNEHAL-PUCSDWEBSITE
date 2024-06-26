#include"/root/website/headers/back.c"

#define prog "/root/website/programs/insertanswer/insertanswerworker.cgi"
#define portno 1242

int main()
{
	char *argv[3],fd[CAPACITY];
	int cfd;

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("*********insertanswer***********\n");

	if((sockfd=connect_front(portno))==0)
	{
		return EXIT_FAILURE;
	}

	argv[0] = prog;			
	argv[2] = NULL;
	
	while(1)
	{
		cfd=acceptconnection(sockfd);

		if(cfd!=-1)
		{

			myitoa(cfd,fd);
			
			argv[1]=(char*)fd;

			if(fork()==0)
			{
				if( execv(prog,(char**)argv) < 0 )
				{
					logg("failed to spawn process\n");
				}
				return 0;
			}
		}

		close(cfd);
	}

	return 0;
}
