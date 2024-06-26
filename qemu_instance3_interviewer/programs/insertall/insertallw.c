#include"/root/website/headers/back.c"

#define INPUT_MAX 6
#define portno 1236

#define testsubmited "/root/website/files/pages/testsubmited.html"

static enum erno insertdb(char **input, char *userdetails[2])
{
	char *command;

	const char *params[2];

	params[0]=input[1];

	command="delete from studenttest where testid=$1";

	res=PQexecParams(con,(char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}


	command="select stud_id from students";

	params[1]=input[1];
	res=PQexec(con,command);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	int rows=PQntuples(res);
	int i=0;

	command="insert into studenttest values($1,$2,NULL,NULL,'NOT ATTEMPTED',NULL,0)";

	PGresult *res1 ;

	while(i<rows)
	{
		params[0]=PQgetvalue(res,i,0);

		res1=PQexecParams(con,(char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

		if(PQresultStatus(res1)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return INSERT_NA;
		}

		i++;
	}

	char *param[]={input[1],input[2],input[3],input[4],userdetails[0],input[5]};


	command="update test set randomize=$2 ,no_of_attempts=$3,studtype=$4,creatorid=$5,review=$6,teststatus='C' where testid=$1";

	res=PQexecParams(con,(char*)command,6,NULL,(const char* const*)param,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}


static enum erno send_page(int cfd ,char** input)
{
	char *buffer,*output;

	int contentsize[1];

	if(gethtmlcontent(testsubmited,&buffer,contentsize)!=GETHTML_OK)
	{
		return PAGE_NA;
	}

	contentsize[0]+=mylen(input[0]);
	
	output = malloc(sizeof(char)*contentsize[0]);
	contentsize[0]=sprintf(output,buffer,input[0]);

	if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
	{
		return PAGE_NA;
	}

	free(output);
	free(buffer);
	return PAGE_OK;
}


int main(int argc , char *fd[])
{
	enum erno er;

	int cfd,inpsize = TESTID + SESSION_LEN + RANDOM + ATTEMPTS + TYPE+REVIEW ;
	int size[] = {SESSION_LEN,TESTID,RANDOM,ATTEMPTS,TYPE,REVIEW};

	char *input[INPUT_MAX],*userdetails[2];

	cfd =myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("-------InsertAll------\n");

	if(connect_db())
	{
		return EXIT_FAILURE;
	}


	char flow=0;
	if(cfd!=-1)
	{

		if( getinput(cfd,input,inpsize,size,INPUT_MAX) == INPUT_NA )
		{
			flow=1;
		}

		if( flow==0 && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}

		if(flow==0 && (insertdb(input,userdetails)!=INSERT_OK)){
			flow=1;
		}

		if((flow==0) && (send_page(cfd,input)!=PAGE_OK))
		{
			flow=1;
		}	
	}
	closedb();
	close(cfd);
	return 0;
}
