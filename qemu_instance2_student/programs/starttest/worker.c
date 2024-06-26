#include"/root/website/headers/back.c"
#include"/root/website/headers/test.c"

#define testindex "/root/website/files/pages/testindex.html"

#define testpage "/root/website/files/pages/test.html"

#define testnotstarted "<p> Test Has Not Started.Please try on scheduled time</p>\n"
#define testfinished "<p> Test Has Finished </p>\n"
#define noattempts "<p> No Attempts Left </p>\n"

#define INPUT_MAX 4
#define types 2

#define dtimecommand " select enddate,endtime from test where testid=$1 "

#define starttest "update studenttest set status = 'ATTEMPTED' where testid=$1 and studid=$2"

#include"startpage.c"

int datetime(char **dtime , char* testid)
{
	char *params[]={testid};
	
	res=PQexecParams(con,dtimecommand,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return 1;
	}

	if(PQntuples(res)>0)
	{
		dtime[0]=PQgetvalue(res,0,0);
		dtime[1]=PQgetvalue(res,0,1);
	}
	else
	{
		return 1;
	}
	return 0;
}

static enum erno sendpage(int cfd,char** input,char* studid)
{
	char *buffer,*output,*tdetails,*dtime[2];
	int contentsize[1];

	if( (tdetails=test(input[1],studid))==NULL )
	{
		logg("Failed to generate test details\n");
		return PAGE_NA;
	}

	if( datetime(dtime,input[1]) )
	{
		logg("Failed to load Date Time\n");
		return PAGE_NA;
	}

	if( gethtmlcontent(testindex,&buffer,contentsize) != GETHTML_OK )
	{
		logg("Page gen failed\n");
		return PAGE_NA;
	}

	contentsize[0] += mylen(tdetails)+mylen(input[0])+mylen(input[1])+mylen(dtime[0])+mylen(dtime[1]);

	output = malloc(sizeof(char) * contentsize[0] );

	contentsize[0]=sprintf(output,buffer,tdetails,input[0],input[1],dtime[0],dtime[1]);

	if(sendhtmlcontent(cfd,output,contentsize[0]) !=SENDHTML_OK)
	{
		logg("Failed to send page\n");
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}

static enum erno senddata(int cfd,char* error)
{
	char *buffer,*output;
	
	int contentsize[1];

	if( gethtmlcontent(testpage,&buffer,contentsize) != GETHTML_OK )
	{
		logg("Page gen failed\n");
		return PAGE_NA;
	}

	contentsize[0] += mylen(error)+20;
	
	output = malloc(sizeof(char) * contentsize[0] );
	
	contentsize[0]=sprintf(output,buffer,error , "Try Another Test");
	
	if(sendhtmlcontent(cfd,output,contentsize[0]) !=SENDHTML_OK)
	{
		logg("Failed to send page\n");
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_NA;
}


static enum erno checkattempts(int cfd , char **input ,  char** userdetails)
{
	char * command = "select * from test t ,studenttest s  where s.attempts::bigint < t.no_of_attempts::bigint and s.testid=$1 and s.studid=$2  and t.testid=s.testid";

	char *params[2]={input[1],userdetails[0]};

	res=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

	if( PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg("db fetch failure : fun checkattempts\n");
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}

	if( PQntuples(res)==0 )
	{
		return senddata(cfd,noattempts);
	}

	return PAGE_OK;
}

static enum erno validate(int cfd , char **input , char** userdetails)
{
	if( checkattempts(cfd,input,userdetails)!=PAGE_OK )
	{
		return senddata(cfd,noattempts);
	}

	char* params[3],*command;

	params[0]=input[1];
	params[1]=input[2];
	params[2]=input[3];


	res=PQexecParams(con,(char*)intime,3,NULL,(const char* const*)params,NULL,NULL,0);

	if( PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg("db fetch failure : fun sendpage\n");
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}
	
	if( PQntuples(res) > 0 ) //test in time
	{	
		return PAGE_OK;
	}
	else
	{
		command="select * from test where testid=$1 and $2::timestamp + $3::time  <  startdate::timestamp +starttime::time ";
	
		res=PQexecParams(con,(char*)command,3,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun sendpage\n");
			return PAGE_NA;
		}	

		if(PQntuples(res)>0)  
		{
			return senddata(cfd,testnotstarted);
		}

		else
		{
			return senddata(cfd,testfinished);			
		}
	}	
}

static enum erno updatetestdetail(char *testid , char* userid)
{
	char *params[]={testid,userid};
	
	res=PQexecParams(con,starttest,2,NULL,(const char* const*) params , NULL,NULL,0);
	
	
	if( PQresultStatus(res)!=PGRES_COMMAND_OK)
	
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}

int main(int argc , char *fd[])
{
	enum erno er;

	int cfd= myatoi(fd[1]);

	int inputsize=SESSION_LEN+DATE+TIME+TESTID , size[]={SESSION_LEN,TESTID,DATE,TIME};

	char flow=0,*input[INPUT_MAX],*userdetails[2];

	if(open_log())
	{
		return EXIT_FAILURE;	
	}

	logg("---------STARTTEST-----------\n");

	if(connect_db())
	{	
		return EXIT_FAILURE;
	}

	if(cfd!=-1)
	{
		if( (getinput(cfd,input,inputsize,size,INPUT_MAX)!=INPUT_OK) )
		{
			flow=1;
		}


		if( (flow==0) && (getuserdetails(input[0],userdetails) != USER_OK) )
		{
			flow=sessionexpired(cfd);
		}

		if( (flow==0) && (validate(cfd,input,userdetails)!=PAGE_OK) )
		{
			flow=1;
		}

		if(flow==0 && (updatetestdetail(input[1],userdetails[0])!=INSERT_OK))
		{
			flow=1;
		}

		if((flow==0) && (sendpage(cfd,input,userdetails[0])!=PAGE_OK)  )
		{
			flow=1;	
		}

	}
	closedb();
	closeprog(cfd);
	return 0;
}

