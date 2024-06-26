#include"/root/website/headers/back.c"

#define INPUT_MAX 8

#define signuppage "/root/website/files/pages/signup.html" 

#define checkuser "select * from students where stud_id=$1"
#define checkcollege "select * from colleges where college_id=$1"
#define userexists "UserId already present.Please try a different one"
#define collegenexist "CollegeId does not exist.Please try again"
#define insertuser  "insert into students values($1,$2,$3,$4,$5,$6,$7,$8)"
#define accountcreated "Your account has been created.You can proceed with login.\n<a href=\"/login.html\">Click here to login</a>"
#define failedaccntcrtn "Account Creation Failed.Please try again"

int send_data(char* message,int cfd)
{
	char *buffer,*output;
	int content[1];

	if(gethtmlcontent(signuppage,&buffer,content)!=GETHTML_OK)
	{
		return 1;
	}
	
	content[0]+=mylen(message);
	output=malloc(sizeof(char)*content[0]);
	content[0]=sprintf(output,buffer,message);

	if(sendhtmlcontent(cfd,output,content[0])!=SENDHTML_OK)
	{
		return 1;
	}
	return 0;
}

static enum erno checkuserid(char * user,char *college,int cfd)
{
	char *params[1];
	params[0]=user;
	res = PQexecParams(con,(char*)checkuser,1,NULL,(const char* const*)params,NULL,NULL,0);
	
	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg("Failed to fetch user id:");
		logg(PQerrorMessage(con));
		
		if(send_data(failedaccntcrtn,cfd))
		{
			logg("Failed to send data");
		}

		return USER_NA;
	}
	
	if(PQntuples(res))
	{	
		if(send_data(userexists,cfd))
		{
			logg("Failed to send data\n");
		}
		return USER_NA;
	}
	
	//search for college id
	
	params[0]=college;
	
	res= PQexecParams(con,(char*)checkcollege,1,NULL,(const char* const*)params,NULL,NULL,0);
	
	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg("Failed to fetch user id:");
		logg(PQerrorMessage(con));
		
		if(send_data(failedaccntcrtn,cfd))
		{
			logg("Failed to send data");
		}

		return USER_NA;
	}
	
	if(PQntuples(res)==0)
	{	
		if(send_data(collegenexist,cfd))
		{
			logg("Failed to send data\n");
		}
		return USER_NA;
	}
	
	return USER_OK;
}

static enum erno adduser(char **input,int cfd)
{
	res=PQexecParams(con,(const char*)insertuser,INPUT_MAX,NULL,(const char* const*)input,NULL,NULL,0);
	
	if(PQresultStatus(res) != PGRES_COMMAND_OK )
	{
		logg("Failed to insert data\n");
		logg(PQerrorMessage(con));
		if(send_data(failedaccntcrtn,cfd))
		{
			logg("Failed to send data");
		}
		return USER_NA;
	}
	else
	{
		if(send_data(accountcreated,cfd))
		{
			logg("Failed to send signup data\n");
			return USER_NA;
		}
	}
	
	return USER_OK;
}

int main( int argc , char* argv[] )
{
	int inpsize = USERID+PASSWORD+SFNAME+SLNAME+SSTREAM+DEGREE+ADDRESS+COLLEGE,cfd=myatoi(argv[1]);

        int size[]={USERID+PASSWORD+SFNAME+SLNAME+SSTREAM+DEGREE+ADDRESS+COLLEGE};	
	
	char* input[INPUT_MAX],flow=0;

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("******signupworker*********\n");

	if(connect_db())
	{
		logg("db opening failed\n");
	}

	if(cfd!=-1)
	{
		if( getinput(cfd,input,inpsize,size,INPUT_MAX) != INPUT_OK )
		{
			flow=1;
		}	

		if((flow==0) && checkuserid(input[0],input[INPUT_MAX-1],cfd)!=USER_OK)
		{
			flow=1;
		}
		
		if((flow==0) && adduser(input,cfd)!=USER_OK)
		{
			flow=1;
		}		
	}

	closedb();
	closeprog(cfd);
	return 0;
}

