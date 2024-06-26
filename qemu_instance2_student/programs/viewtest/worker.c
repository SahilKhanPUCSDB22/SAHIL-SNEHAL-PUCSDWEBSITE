#include"/root/website/headers/back.c"
#include"/root/website/headers/test.c"

#define portown 1238
#define htmlfile "/root/website/files/pages/viewtest.html"
#define notest "/root/website/files/pages/notest.html"
#define INPUT_MAX 1
#define file "/root/website/files/testdetails/"

#define command "select t.testid , t.testname , t.startdate ,t.starttime ,s.marks from test t, studenttest s where t.testid=s.testid and s.studid=$1"

#define	part0 "<td><a href=\"#\" onclick=\"loadtest('"
#define	part1 "'); return false;\">"
#define	part2 "</a></td>\n"

static enum erno sendpage(int cfd , char **userdetails)
{
	char *buffer,*filename = mycat(file,mycat(userdetails[0],"testdetails")), *table , *output , *out;

	int contentsize[1];
	
	tp =fopen (filename,"w+");
	if(tp==NULL)
	{
		logg("Failed to open file\n");
		return PAGE_NA;
	}

	const char *params[]={userdetails[0]};

	res=PQexecParams(con,(char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}

	int rows=PQntuples(res),cols=PQnfields(res);

	if(cols>0 && rows >0)
	{
		int i=0,j=0;
		while(i<rows)
		{
			wp("<tr>\n");

			j=0;
			while(j<cols)
			{
				out=PQgetvalue(res,i,j);
				if(j)
				{
					wp("<td>"),wp(out),wp("</td>\n");
				}
				else
				{
					wp(part0),wp(out),wp(part1) ,wp(out),wp(part2);
				}

				j++;
			}

			wp("</tr>\n");
			i++;
		}

		table= malloc(sizeof(char)*total);

		if(getpage(table,filename))
		{
			logg("Failed to gen page\n");
			return PAGE_NA;
		}

		if( gethtmlcontent(htmlfile,&buffer,contentsize) != GETHTML_OK)
		{
			err="couldnt get content from page";
			logg(err);
			return PAGE_NA;
		}

		contentsize[0] = contentsize[0] + mylen(table)+SESSION_LEN+1;
		
		output=malloc(sizeof(char)*contentsize[0]);
		
		contentsize[0]=sprintf(output,buffer,table,userdetails[1]);
	}
	else
	{
		if( gethtmlcontent(notest,&buffer,contentsize) != GETHTML_OK)
		{
			err="couldnt get content from page";
			logg(err);
			return PAGE_NA;
		}
		output=buffer;
	}		

	if( sendhtmlcontent(cfd,output,contentsize[0]) != SENDHTML_OK)
	{
		err="couldnt send page\n";
		logg(err);
		return PAGE_NA;
	}

	free(output);
	free(buffer);

	return PAGE_OK;
}

int main(int argc , char *argv[])
{
	enum erno er;
	int cfd,inputsize=SESSION_LEN+1 , size[]={SESSION_LEN};
	char flow=0, *input[INPUT_MAX] , *userdetails[2];

	cfd = myatoi(argv[1]);

	if(open_log())
	{
		return EXIT_FAILURE;	
	}

	logg("-------ViewTestBackWorker-----------\n");

	if(connect_db())
	{	
		return EXIT_FAILURE;
	}

	flow=0;

	if(cfd!=-1)
	{
		if( (getinput(cfd,input,inputsize,size,INPUT_MAX)!=INPUT_OK) )
		{
			printf("ip failure\n");
			flow=1;
		}
		
		if( (flow==0) && ( getuserdetails(input[0],userdetails) != USER_OK ) )
		{
			flow = sessionexpired(cfd);
		}

		if((flow==0) && (sendpage(cfd,userdetails)!=PAGE_OK ) )
		{
			flow=1;	
		}

	}

	closedb();
	closeprog(cfd);
	return 0;
}
