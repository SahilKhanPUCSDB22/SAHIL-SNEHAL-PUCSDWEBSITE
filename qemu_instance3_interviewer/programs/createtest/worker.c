#include"/root/website/headers/back.c"

#define testpresent "/root/website/files/pages/testpresent.html"
#define page "/root/website/files/pages/insertquestion.html"
#define filetest "/root/website/files/pages/filetest.html"

#define access "/root/website/files/pages/access.html"

#define INPUT_MAX 8

#define portno 1236
#define ID 1024
#define name 1024
#define stime 12
#define etime 12
#define sdate 12
#define edate 12
#define type 2

static enum erno insertdb(char **input)

{
	char *params[]={input[1],input[2],input[3],input[4],input[5],input[6],input[7]};

	int i=0;

	char *command="insert into test values($1,$2,$3::date,$4::time,$5::date,$6::time,$7);";

	res= PQexecParams(con,(char*)command,7,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return INSERT_NA;
	}

	return INSERT_OK;
}

static enum erno send_page(int cfd ,char** input ,char** userdetails)
{
	char *buffer,*output;
	int contentsize[1];
	
	if(input[INPUT_MAX-1][0]=='F')
	{
		if(gethtmlcontent(testpresent,&buffer,contentsize)!=GETHTML_OK)
		{
			return PAGE_NA;
		}
		
		contentsize[0]+=mylen(input[0])+mylen(input[1]);
		output=malloc(sizeof(char)*contentsize[0]);
		contentsize[0]=sprintf(output,buffer,input[0],input[1]);

		if(sendhtmlcontent(cfd,buffer,contentsize[0])!=SENDHTML_OK)
		{
			return PAGE_NA;
		}
	}
	else
	{
		if(gethtmlcontent(page,&buffer,contentsize)!=GETHTML_OK)
		{
			return PAGE_NA;
		}

		contentsize[0]+=mylen(input[0])+mylen(input[1])+1;
		output=malloc(sizeof(char)*contentsize[0]);
		contentsize[0]=sprintf(output,buffer,input[0],input[1],1);
		
		if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
		{
			return PAGE_NA;
		}	
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}

static enum erno checktest(int cfd,char **input)
{
	char *buffer;
	
	int contentsize[1];

	char *command= "select * from test where testid=$1";
	
	const char *params[]={input[1]};

	res=PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);
	
	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}

	int rows=PQntuples(res);

	if(rows>0)
	{
		if(gethtmlcontent(testpresent,&buffer,contentsize)!=GETHTML_OK)
		{
			return PAGE_NA;
		}

		if(sendhtmlcontent(cfd,buffer,contentsize[0])!=SENDHTML_OK)
		{
			return PAGE_NA;
		}
		free(buffer);
		return PAGE_NA;
	}

	return PAGE_OK;
}


int main(int argc , char *fd[])
{
	int cfd , inpsize = SESSION_LEN + ID + name  + sdate + stime + edate + etime+type ;

	int size[]={ SESSION_LEN , ID , name , sdate , stime , edate , etime , type};

	char *input[INPUT_MAX],*userdetails[2];

	cfd = myatoi(fd[1]);
	
	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("-------Creating Test-----\n");

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

		if((flow==0) && (checktest(cfd,input)!=PAGE_OK))
		{
			flow=1;
		}
		
		if((flow==0) && (insertdb(input)!=INSERT_OK))
		{
			flow=1;
		}

		if((flow==0) && (send_page(cfd,input,userdetails)!=PAGE_OK))
		{
			flow=1;
		}
	}
	
	closedb();
	closeprog(cfd);
	
	return 0;
}
