#include"/root/website/headers/back.c"

#define INPUT_MAX 5

//checking commands

#define alreadypresent "select * from fquestion where testid=$1 and qno=$2 and fname=$3"
#define checksize "select * from fquestion where testid=$1 and qno=$2 and fname=$3 and fsize=$4"
#define checkparts "select * from fquestion where testid=$1 and qno=$2 and fname=$3 and parts=$4"

//messages

#define incomplete "0"
#define difffile "1"
#define exists "2"
#define notexist "3"
#define toobig "4"

#define uploadfailure "Failed to Upload File.Please Try Again.\n"

static enum erno sendmessage(int cfd , char *message)
{
	if(sendhtmlcontent(cfd,message,mylen(message))!=SENDHTML_OK)
	{
		return PAGE_NA;
	}
	return PAGE_OK;
}

int error()
{
	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return 1;
	}
	return 0;
}

static enum erno checkindb( int cfd , char **input)
{
	if( myatoi(input[4]) > FILE_SIZE )
	{
		sendmessage(cfd,toobig);
		return FILE_OK;
	}

	char* params[]={input[1],input[2],input[3],input[4]};	
	
	res = PQexecParams(con,alreadypresent,3,NULL,(const char* const*)params,NULL,NULL,0);

	if(error())
	{
		return FILE_NA;
	}
	if(PQntuples(res)>0)
	{
		res = PQexecParams(con,checksize,4,NULL,(const char* const*)params,NULL,NULL,0);
		if(error())
		{
			return FILE_NA;
		}

		if(PQntuples(res)>0)
		{
			char *parts = malloc(sizeof(char)*CAPACITY);
			
			myitoa( MAX_SEG( myatoi(input[4]) ),parts);
			
			params[3] = (char*) parts ; 

			res=PQexecParams(con, checkparts , 4 ,NULL ,(const char* const*)params,NULL,NULL,0);
		        
			free(parts);

			if(error())
			{
				return FILE_NA;
			}
			if(PQntuples(res)>0)
			{
				sendmessage(cfd,exists);
			}
			else
			{
				sendmessage(cfd,incomplete);
			}
		}
		else
		{
			sendmessage(cfd,difffile);
		}
	}
	else
	{
		sendmessage(cfd,notexist);
	}

	return FILE_OK;
}

int main(int argc , char *fd[])
{
	int cfd , inpsize = SESSION_LEN + TESTID + FNAME_LEN + FSIZE_LEN + QNO;

	int size[]={ SESSION_LEN , TESTID , QNO , FNAME_LEN ,FSIZE_LEN};

	char *input[INPUT_MAX],*userdetails[2];

	cfd = myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("-----FileCheckWorker------\n");

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

		if( (flow==0) && (checkindb(cfd,input))!=FILE_OK )
		{
			sendmessage(cfd,uploadfailure);
			flow=1;
		}
	}
	
	closedb();
	closeprog(cfd);
	
	return 0;
}
