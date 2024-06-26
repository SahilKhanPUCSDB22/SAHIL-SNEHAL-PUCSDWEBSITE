#include"/root/website/headers/back.c"

#define test "/root/website/files/pages/test.html"
#define start "/root/website/files/pages/starttest.html"

#define testnotstarted "<p> Test Has Not Started.Please try on scheduled time</p>\n"
#define testfinished "<p> Test Has Finished </p>\n"
#define noattempts "<p> No Attempts Left </p>\n"

#define INPUT_MAX 4

char* testdetails(char* testid)
{
	char *buffer="";

	char * command = "select testid,testname,startdate,starttime,enddate,endtime,no_of_attempts from test where testid = $1" ,*params[]={testid},*content[]={"TestID : ","TestName : ","Start Date : " , "Start Time : ","End Date : ","End Time : ","No Of Attempts : "},inps=7,i=0;

	res=PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return NULL;
	}

	int rows,cols;
	if((rows=PQntuples(res))>0)
	{
		buffer = mycat("<p>",buffer);
		while(i<inps)
		{	
			buffer = mycat( buffer , mycat( content[i] , PQgetvalue(res,0,i) ) );
			buffer = mycat( buffer , "<br>" );
			i++;
		}

		buffer=mycat(buffer,"</p>");
	}
	else
	{
		return NULL;
	}

	return buffer;
}

static enum erno senddata(int cfd,char* error,char* testid)
{
	char *buffer,*output,*tdetails;
	int contentsize[1];

	if( (tdetails=testdetails(testid))==NULL )
	{
		logg("Failed to generate test details\n");
		return PAGE_NA;
	}

	if( gethtmlcontent(test,&buffer,contentsize) != GETHTML_OK )
	{
		logg("Page gen failed\n");
		return PAGE_NA;
	}

	contentsize[0] += mylen(error)+mylen(tdetails);
	
	output = malloc(sizeof(char) * contentsize[0] );
	
	contentsize[0]=sprintf(output,buffer,tdetails,error);
	
	if(sendhtmlcontent(cfd,output,contentsize[0]) !=SENDHTML_OK)
	{
		logg("Failed to send page\n");
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}

static enum erno starttest(int cfd , char **input)
{
	char *buffer,*output,*tdetails;
	int size[1];
	
	if( (tdetails=testdetails(input[1]))==NULL )
	{
		logg("Failed to generate test details\n");
		return PAGE_NA;
	}
	
	if(gethtmlcontent(start,&buffer,size) !=GETHTML_OK)
	{
		return PAGE_NA;
	}

	size[0]+=mylen(input[0])+mylen(input[1])+mylen(tdetails);
	
	output = malloc(sizeof(char) * size[0] );
	
	size[0]=sprintf(output,buffer,tdetails,input[0],input[1]);

	if(sendhtmlcontent(cfd,output,size[0])!=SENDHTML_OK)
	{
		return 0;
	}

	return PAGE_OK;
}

static enum erno checkattempts(int cfd , char **input ,  char** userdetails)
{
	char * command = "select * from test t ,studenttest s  where s.attempts::bigint  < t.no_of_attempts::bigint and s.testid=$1 and s.studid=$2  and t.testid=s.testid";

	char *params[2]={input[1],userdetails[0]};

	res=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

	if( PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg("db fetch failure : fun checkattempts\n");
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}

	if(PQntuples(res)==0)
	{
		return senddata(cfd,noattempts,input[1]);
	}

	return ATT_OK;
}

static enum erno sendpage(int cfd , char **input , char** userdetails)
{
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
		return starttest(cfd,input);
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

		if(PQntuples(res))  
		{
			return senddata(cfd,testnotstarted,input[1]);
		}

		else
		{
			return senddata(cfd,testfinished,input[1]);			
		}
	}	
	return PAGE_NA;
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

	logg("---------CheckTestBack----------\n");

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

		if( (flow==0) && (checkattempts(cfd,input,userdetails)!=ATT_OK) )
		{
			flow=1;
		}
		
		if((flow==0) && (sendpage(cfd,input,userdetails)!=PAGE_OK)  )
		{
			flow=1;	
		}

	}
	closedb();
	closeprog(cfd);
	return 0;
}
