#include"/root/website/headers/back.c"

#define INPUT_MAX 6

#define insert "insert into studenttest values($1,$2,NULL,NULL,'NOT ATTEMPTED',NULL,0)"

#define	update "update test set randomize=$2 ,no_of_attempts=$3 , studtype=$4 , creatorid=$5 , review=$6 , teststatus='C' where testid=$1"

#define testsubmited "/root/website/files/pages/testsubmited.html"

static enum erno getlist(int cfd,char ** slist)
{
	int ptr=0 , i=CAPACITY;

	*slist=(char *)malloc(sizeof(char)*(CAPACITY*2));

	while(i == CAPACITY)
	{
		if( (i=recv(cfd, (*slist)+ptr , CAPACITY , 0 ) ) <=0 )
		{
			return INPUT_NA;
		}
		ptr+=i;
	}

	(*slist)[i]=10;

	logg("Recieved List\n");

	return INPUT_OK;

}

static enum erno deletedb(char **input)
{
	char *params[1];
	params[0]=input[1];

	char *command="delete from studenttest where testid=$1";

	res=PQexecParams(con,(char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}

static enum erno insertdb(int cfd,char **input,char *user , char* slist)
{		
	int i=0,k=0,size=mylen(slist);

	char* studid= malloc(sizeof(char)*CAPACITY),*params[2];

	params[1]=input[1];

	{
		char *param[]={input[1],input[2],input[3],input[4],user,input[5]};

		res=PQexecParams(con,(char*)update,6,NULL,(const char* const*)param,NULL,NULL,0);
		
		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return INSERT_NA;
		}
	}

	while(i<size)
	{
		if(slist[i]==13 || slist[i]==10)
		{
			studid[k]=0;

			params[0]=(char*)studid;

			res=PQexecParams(con,(char*)insert,2,NULL,(const char* const*)params,NULL,NULL,0);

			if(PQresultStatus(res)!=PGRES_COMMAND_OK)
			{
				logg(PQerrorMessage(con));
				return INSERT_NA;
			}

			k=0;

			i=i+1;
		}
		else
		{
			studid[k]=slist[i];
			k=k+1;
		}
		i++;
	}

	return INSERT_OK;
}

static enum erno send_page(int cfd ,char** input)
{
	char *buffer,*output;
	int contentsize[1];

	if(gethtmlcontent(testsubmited,&buffer,contentsize)!=GETHTML_OK)
	{
		logg("gethtml failure");
		return PAGE_NA;
	}

	contentsize[0]+=mylen(input[0]);

	output=malloc(sizeof(char)*contentsize[0]);

	contentsize[0]=sprintf(output,buffer,input[0]);

	if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
	{
		logg("gethtml failure");
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}


int main(int argc , char *fd[])
{
	enum erno er;

	int inpsize = SESSION_LEN + TESTID + RANDOM + ATTEMPTS + TYPE + REVIEW , cfd , size[] = { SESSION_LEN , TESTID ,RANDOM , ATTEMPTS , TYPE ,REVIEW};

	char *input[INPUT_MAX],*userdetails[2],*slist;

	cfd =myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}
	
	logg("----SubmitTestWorker----\n");

	if(connect_db())
	{
		return EXIT_FAILURE;
	}

	char flow=0;
	if(cfd!=-1)
	{
		if( getinput(cfd,input,inpsize,size,INPUT_MAX) != INPUT_OK )
		{
			flow=1;
		}
		if( flow==0 && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}

		if(flow==0 && getlist(cfd,&slist)==INPUT_NA)
		{
			flow=1;
		}
		if(flow==0 && (deletedb(input)!=INSERT_OK))
		{
			flow=1;
		}

		if(flow==0 && (insertdb(cfd,input,userdetails[0],slist)!=INSERT_OK))
		{
			flow=1;
		}

		if((flow==0) && (send_page(cfd,input)!=PAGE_OK))
		{
			flow=1;
		}	
	}

	fflush(NULL);
	closedb();
	close(cfd);
	return 0;
}
