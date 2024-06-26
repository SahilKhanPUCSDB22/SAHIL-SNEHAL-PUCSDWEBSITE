#include"/root/website/headers/back.c"

#define INPUT_MAX 5

//checking commands

#define checkfile " select fname from fanswer where testid=$1 and studid=$2 and qno=$3"
#define alreadypresent "select * from fanswer where testid=$1 and studid=$2 and qno=$3 and fname=$4 "
#define checksize "select * from fanswer where testid=$1 and studid=$2 and qno=$3 and fname=$4 and fsize=$5"
#define checkparts "select * from fanswer where testid=$1 and studid=$2 and qno=$3 and fname=$4 and parts=$5"

//messages

#define incomplete "0"
#define difffile "1"
#define exists "2"
#define notexist "3"
#define toobig "4"
#define notsame "5"

#define uploadfailure "Failed to Upload File.Please Try Again.\n"

enum indexes { sessionid , testid , qno , filename , filesize };

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

int checkname ( char ** params)
{
	res = PQexecParams(con,checkfile,3,NULL,(const char* const*)params,NULL,NULL,0);

	if( error() )
	{
		return 2;
	}

	if(PQntuples(res)>0)
	{
		if ( mycmp(params[3],(char*)PQgetvalue(res,0,0)) )
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 3;
	}
}


static enum erno checkindb( int cfd , char **input ,char * userid)
{
	if( myatoi(input[4]) > FILE_SIZE )
	{
		sendmessage(cfd,toobig);
		return FILE_OK;
	}


	char* params[]={input[testid],userid,input[2],input[3],input[4]};	
	
	int ret;
	
	if( (ret=checkname (params)) == 1 )
	{
		sendmessage(cfd,notsame);
	}
	else if(ret==2)
	{
		return FILE_NA;
	}
	else if(ret==3)
	{
		sendmessage(cfd,notexist);
	}
			
	res = PQexecParams(con,alreadypresent,4,NULL,(const char* const*)params,NULL,NULL,0);

	if(error())
	{
		return FILE_NA;
	}
	if(PQntuples(res)>0)
	{
		res = PQexecParams(con,checksize,5,NULL,(const char* const*)params,NULL,NULL,0);
		if(error())
		{
			return FILE_NA;
		}

		if(PQntuples(res)>0)
		{
			char *parts = malloc(sizeof(char)*CAPACITY);
			
			myitoa( MAX_SEG( myatoi(input[4]) ),parts);
			
			params[4] = (char*) parts ; 

			res=PQexecParams(con, checkparts , 5 ,NULL ,(const char* const*)params,NULL,NULL,0);
		        
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
		else
		{
			//printf("Input OK\n");
		}

		if( (flow==0) && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}

		if( (flow==0) && (checkindb(cfd,input,userdetails[0]))!=FILE_OK )
		{
			sendmessage(cfd,uploadfailure);
			flow=1;
		}
	}
	
	closedb();
	closeprog(cfd);
	
	return 0;
}
