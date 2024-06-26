#include"/root/website/headers/back.c"

#define INPUT_MAX 1

#define udetails "/root/website/files/pages/userdetails.html"

#define str0 "<h2>Hello "
#define str1 "</h2><br>"
#define str2 "<h4> Address : "
#define str3 "<h4> Stream :"
#define str4 "<h4> Degree :"
#define str5 "<h4> College ID :"
#define str6 "<h4> College Name :"
#define str7 "</h4><br>"

static enum erno details(char **userdetails ,char **gen)
{

	char *command,*params[]={userdetails[0]};

	command="select * from students where stud_id=$1";

	res=PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}


	int rows=PQntuples(res);

	if(rows>0)
	{
		int i=0,size=0,cols=PQnfields(res);
		while(i<cols)
		{
			size+=mylen((char*)PQgetvalue(res,0,i));
			i++;
		}
		*gen=malloc(sizeof(char)*(size+CAPACITY) );

				i=0;

				strcat(gen[0],str0);

				strcat(gen[0],PQgetvalue(res,i,1));
				
				strcat(gen[0],"\t");

				strcat(gen[0],PQgetvalue(res,i,2));

				strcat(gen[0],str1);

				strcat(gen[0],str2);

				strcat(gen[0],PQgetvalue(res,i,3));

				strcat(gen[0],str7);

				strcat(gen[0],str3);

				strcat(gen[0],PQgetvalue(res,i,4));

				strcat(gen[0],str7);
			
				strcat(gen[0],str4);

				strcat(gen[0],PQgetvalue(res,i,5));

				strcat(gen[0],str7);

				strcat(gen[0],str5);

				strcat(gen[0],PQgetvalue(res,i,7));

				strcat(gen[0],str7);

				strcat(gen[0],str6);

				params[0]=PQgetvalue(res,i,7);

				command="select college_name from colleges where college_id=$1";

				res=PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

				if(PQresultStatus(res)!=PGRES_TUPLES_OK)
				{
					logg(PQerrorMessage(con));
					return PAGE_NA;
				}
				if(PQntuples(res)>0)
				{
					strcat(gen[0],PQgetvalue(res,0,0));
					strcat(gen[0],str7);
				}
	}
	else
	{
		return GEN_NA;
	}
	return GEN_OK;
}

static enum erno send_page(int cfd ,char* input,char *gen)
{
	char *buffer,*output;
	int contentsize[1];

	if(gethtmlcontent(udetails,&buffer,contentsize)!=GETHTML_OK)
	{
		return PAGE_NA;
	}

	contentsize[0]+=mylen(gen)+mylen(input)+1;
	output=malloc(sizeof(char)*contentsize[0]);
	contentsize[0]=sprintf(output,buffer,gen,input);

	if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
	{
		return PAGE_NA;
	}

	free(buffer);
	free(output);

	return PAGE_OK;
}




int main(int argc , char* argv[])
{
	char *gen,*input[INPUT_MAX],*userdetails[2],flow=0;

	int streamlen= SESSION_LEN + INPUT_MAX , size[]={SESSION_LEN},cfd;

	cfd  =  myatoi(argv[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("------------Display Student-----------\n");

	if(connect_db())
	{
		logg("Db connection failed (loginworker)\n");
		return EXIT_FAILURE;
	}


	if(cfd!=-1)
	{
		if( getinput(cfd,input,streamlen,size,INPUT_MAX) !=INPUT_OK )	
		{
			flow=1;
		}

		if(flow==0 && (getuserdetails(input[0],userdetails) != USER_OK))
		{
			flow=sessionexpired(cfd);

		}
		if(flow==0 && (details(userdetails,&gen)!=GEN_OK))
		{
			flow=1;
		}
		if( (flow==0) && (send_page(cfd,input[0],gen)!=PAGE_OK))
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);
	return 0;
}
