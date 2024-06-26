#include"/root/website/headers/back.c"

#define page "/root/website/files/pages/insertquestion.html"

#define deldes "delete from dquestion where testid = $1 and qno = $2"
#define delmcq "delete from mquestion where testid = $1 and qno = $2"
#define delops "delete from options where testid = $1 and qno = $2"

#define insdes "insert into dquestion values ($1,$2,$3,NULL,$4)"
#define insmcq "insert into mquestion values ($1,$2,$3,NULL)"
#define insops "insert into options values   ($1,$2,$3,$4,NULL)"

#define INPUT_MAX 5
#define MINPUTS 2

static enum erno removedb(char **input , char** userdetails)
{
	char *command;
	char * params[]={input[1],input[3]};
	//if(input[2][0]=='D')
	//{
		res = PQexecParams(con ,(char*) deldes , 2 ,NULL,(const char* const*)params , NULL,NULL,0);

		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return REMOVE_NA;
		}
//	}
//	else
//	{
		res = PQexecParams(con ,(char*) delops , 2 ,NULL,(const char* const*)params , NULL,NULL,0);

		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return REMOVE_NA;
		}

		res = PQexecParams(con ,(char*) delmcq , 2 ,NULL,(const char* const*)params , NULL,NULL,0);

		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return REMOVE_NA;
		}
//	}

	return REMOVE_OK;
}

static enum erno insertdb(char **input,char *question , char **options)
{
	char *params[5];
	params[0]=input[1],params[1]=input[3],params[2]=question,params[3]=input[4];

	if(input[2][0]=='D')
	{
		
		res = PQexecParams(con,(const char*)insdes,4,NULL,(const char* const*)params,NULL,NULL,0);

		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return INSERT_NA;
		}
	}
	else
	{	
		res = PQexecParams(con,(const char*)insmcq,3,NULL,(const char* const*)params,NULL,NULL,0);

		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			logg(PQerrorMessage(con));
			return INSERT_NA;
		}

		int i=1,len=myatoi(input[4]);

		char *opno = malloc(sizeof(char)*CAPACITY);

		while(i <= len)
		{
			myitoa(i,opno);
			params[2]=(char*)opno;
			params[3]=options[i-1];
			
			res = PQexecParams(con,(const char*)insops,4,NULL,(const char* const*)params,NULL,NULL,0);

			if(PQresultStatus(res)!=PGRES_COMMAND_OK)
			{
				logg(PQerrorMessage(con));
				return INSERT_NA;
			}

			i++;
		}
	}

	return INSERT_OK;
}

static enum erno send_page(int cfd ,char** input ,char** userdetails)
{
	char *buffer,*output;
	int contentsize[1];

	if(gethtmlcontent(page,&buffer,contentsize)!=GETHTML_OK)
	{
		return PAGE_NA;
	}

	contentsize[0]+=mylen(input[0])+mylen(input[1])+1;

	output=malloc(sizeof(char)*contentsize[0]);

	contentsize[0]=sprintf(output,buffer,input[0],input[1],(myatoi(input[3])+1));

	if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
	{
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}

int getquestion(char * buffer , char **question )
{
	int i=0,j=0;

	char * qlen = malloc (sizeof(char)*CAPACITY);

	while(buffer[i]!=10)
	{	
		qlen[i]=buffer[i];
		i++;
	}

	qlen[i]=0; //finish parsing qlength 

	int len = myatoi(qlen);

	free(qlen);

	*question = malloc (sizeof(char) * (len+1) );

	i++; //move on to qstring
	
	while( len > 0 )
	{
		(*question)[j++]=buffer[i++];
		len--;
	}
	
	(*question)[j]=0;

	i++; //move forward
	
	while( ((buffer[i++]/10)-1) );

	return i;
}

long getquesopt(char * buffer , char ** question , char **options , int noptions)
{
	int i=0,j=0,k=0,l;

	char *temp,*len=malloc(sizeof(char)*CAPACITY);

	if( (i=getquestion(buffer,question))==0)
	{
		logg("Failed to qet question for option\n");
		return 0;
	}

	while(j < noptions)
	{
		k=0;

		while( buffer[i]!=13 && buffer[i] != 10 )
		{
			len[k++] = buffer[i++];
		}
		
		i+=2;  //due to space(eos)

		len[k] = 0;

		l = myatoi((char*)len);

		temp = malloc ( sizeof(char)*(l+1) );

		k=0;

		while(  l > 0 )
		{
			if(buffer[i]==13) //carriage return issues
			{
				l++;
			}

			temp[k++] = buffer[i++] ;
			l--;
		}
		
		i+=2;   //due to carriage return
		
		temp[k]=0;

		options[j] = temp;

		j++;
	}	
		
	free(len);

	return i;
}

static enum erno getqdata(int cfd,char **input,char ** question , char **options)
{
	char * buffer;

	if( get_stream( cfd , &buffer ) != STREAM_OK )
	{
		logg("Failed to recieve Stream(get_stream)\n");
		return INPUT_NA;
	}

	if( input[2][0]=='D')
	{

		if( getquestion( buffer , question ) == 0 )
		{
			logg("Failed to parse question");
			return INPUT_NA;
		}
	}
	else
	{
		if( getquesopt( buffer , question , options ,myatoi(input[4] )) == 0 )
		{
			logg("Failed to parse q-op data\n");
			return INPUT_NA;
		}
	}
	
	free(buffer);

	return INPUT_OK;
}		

int main(int argc , char *fd[])
{
	int cfd , inpsize = SESSION_LEN + TESTID + QTYPE + QNO + CAPACITY;

	int size[]={ SESSION_LEN , TESTID , QTYPE , QNO , CAPACITY};

	char *input[INPUT_MAX],*userdetails[2],*question;

	cfd = myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("-----Inserting Question----");

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
			//printf("INPUT OK\n");
		}

		char *options[ myatoi(input[4]) ];

		if( (flow==0) && (getqdata(cfd,input,&question,options))!=INPUT_OK )
		{
			flow=1;
		}
		else
		{
			//printf("QDATA OK\n");
		}

		if( (flow==0) && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}
		else
		{
			//printf("SESSION OK\n");
		}
	

		if( (flow==0) && (removedb(input,userdetails))!=REMOVE_OK)
		{
			flow=1;
		}

		if((flow==0) && (insertdb(input,question,options)!=INSERT_OK))
		{
			flow=1;
		}

		if((flow==0) && (send_page(cfd,input,userdetails)!=PAGE_OK))
		{
			flow=1;
		}
		else
		{
			//printf("Page send\n");
		}
	}

	closedb();
	closeprog(cfd);

	return 0;
}
