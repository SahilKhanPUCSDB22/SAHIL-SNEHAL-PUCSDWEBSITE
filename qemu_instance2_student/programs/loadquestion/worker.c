#include"/root/website/headers/back.c"
#include"/root/website/headers/test.c"

#define INPUT_MAX 5

char* test (char** input , char * studid )
{
	char *filename = mycat(floc , mycat(studid,input[1]) );
	tp = fopen (  filename , "w+");

	if(tp==NULL)
	{
		logg("Failed to open file for index\n");
		return NULL;
	}

	char *buffer , * params[2] , *answer , *tparams[3];	

	params[0]= input[1],tparams[0]=studid;
	params[1]= input[2],tparams[1]=input[1],tparams[2]=input[2]; 

	wp(form);

	if(input[4][0]=='D')
	{
		res=PQexecParams(con,(char*)getdes,2,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun test\n");
			return NULL;
		}

		insertquestion(input[3],PQgetvalue(res,0,0));

		answer=getanswer(tparams,"D");

		if(answer[0]=='F')
		{
			logg("Failed to get answer\n");
			return NULL;
		}

		insertanswerbox(res,answer);
	}
	else if(input[4][0]=='F')
	{
		res=PQexecParams(con,(char*)getfile,2,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun test\n");
			return NULL;
		}
		
		insertquestion(input[3],PQgetvalue(res,0,0));

		wp("<textarea rows=\"30\" cols=\"120\" disabled>"),wp(PQgetvalue(res,0,1)),wp("</textarea><br>");

		answer = getanswer(tparams,"M");

		if( answer[0]=='F')
		{
			logg("Failed to create page\n");
			return NULL;
		}

		if(answer[0]!='E')
		{
			wp(label),wp("File Already Uploaded : "),wp(answer),wp(clabel);
		}

		wp(label),wp("Select your File : "),wp(clabel),wp(br);

		wp("<input type=\"file\" id=\"sfile\" name=\"sfile\"><br>");

	}
	else
	{
		//check randomize

		res=PQexecParams(con,(char*)chkrndm,1,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun test\n");
			return NULL;
		}

		char* random = PQgetvalue(res,0,0);

		res=PQexecParams(con,(char*)getmcq,2,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun test\n");
			return NULL;
		}

		insertquestion(input[3],PQgetvalue(res,0,0));

		if(random[0]=='Y')
		{
			res=PQexecParams(con,(char*)opsrand,2,NULL,(const char* const*)params,NULL,NULL,0);

			if( PQresultStatus(res)!=PGRES_TUPLES_OK)
			{
				logg(PQerrorMessage(con));
				logg("db fetch failure : fun test\n");
				return NULL;
			}
		}
		else
		{
			res=PQexecParams(con,(char*)ops,2,NULL,(const char* const*)params,NULL,NULL,0);

			if( PQresultStatus(res)!=PGRES_TUPLES_OK)
			{
				logg(PQerrorMessage(con));
				logg("db fetch failure : fun test\n");
				return NULL;
			}

		}

		answer=getanswer(tparams,"M");
		
		if(answer[0]=='F')
		{
			logg("Failed to get answer\n");
			return NULL;
		}

		insertoptions(res,answer);		
	}

	insertformbutton();

	wp(cform);

	buffer = malloc(sizeof(char)*total);

	if(getpage(buffer,filename) )
	{
		return NULL;
	}

	return buffer;
}

static enum erno sendpage(int cfd,char** input,char* studid)
{
	char *output;
	int contentsize[1];

	if( (output=test(input,studid))==NULL )
	{
		logg("Failed to generate test details\n");
		return PAGE_NA;
	}

	if(sendhtmlcontent(cfd,output,total) !=SENDHTML_OK)
	{
		logg("Failed to send page\n");
		return PAGE_NA;
	}

	free(output);

	return PAGE_OK;
}

int main(int argc , char *fd[])
{
	enum erno er;

	int cfd= myatoi(fd[1]);

	int inputsize=SESSION_LEN+TESTID+QNO+QLABEL+QTYPE , size[]={SESSION_LEN,TESTID,QNO,QLABEL,QTYPE};

	char flow=0,*input[INPUT_MAX],*userdetails[2];

	if(open_log())
	{
		return EXIT_FAILURE;	
	}

	logg("**********starttstbckworker**********\n");

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

		if((flow==0) && (sendpage(cfd,input,userdetails[0])!=PAGE_OK)  )
		{
			flow=1;	
		}

	}
	closedb();
	closeprog(cfd);
	return 0;
}

