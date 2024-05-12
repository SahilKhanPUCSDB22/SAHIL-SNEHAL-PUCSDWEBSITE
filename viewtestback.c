#include"back.c"

#define portown 1238
#define htmlfile "/root/website/files/pages/viewtest.html"
#define notest "/root/website/files/pages/notest.html"
#define INPUT_MAX 1

static enum erno sendpage(int cfd , char **userdetails)
{
	char *buffer;
	int contentsize[1];

	char *command="select testid,submitdate,submittime,marks from studenttest where studentid=$1::bigint";
	const char *params[]={userdetails[0]};

	res=PQexecParams(con,(char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}

	int rows=PQntuples(res),cols=PQnfields(res);

	char *temp,*table="",*part[3],*out,*output;

	part[0]= "<td><a href=\"#\" onclick=\"loadtest('"; 
	part[1]= "')\">";
	part[2]="</a></td>\n"; 

	if(cols>0 && rows >0)
	{
		int i=0,j=0;
		while(i<rows)
		{
			table=mycat("<tr>\n",table);

			j=0;
			while(j<cols)
			{
				out=PQgetvalue(res,i,j);
				if(j)
				{
					temp = mycat ( "<td>" ,mycat(out,"</td>\n") );
				}
				else
				{
					temp = mycat ( part[0], mycat(out,mycat(part[1] , mycat(out,part[2]))));
				}
					table = mycat (table,temp);
				j++;
			}

			table=mycat(table,"</tr>\n");
			i++;
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

	FILE* tp = fopen("checking","w+");

	fwrite(output,sizeof(char),contentsize[0],tp);

	free(output);
	free(buffer);

	return PAGE_OK;
}

int main(void)
{
	enum erno er;
	int cfd,inputsize=SESSION_LEN,size[]={SESSION_LEN};
	char loop=1,flow=0,*input[INPUT_MAX],*userdetails[2];

	if(open_log())
	{
		return EXIT_FAILURE;	
	}

	logg("**********viewtestback**********\n");

	if(connect_db())
	{	
		return EXIT_FAILURE;
	}

	if((sockfd=connect_front(portown))==0)
	{
		return EXIT_FAILURE;
	}

	while(loop!=0)
	{
		flow=0;
		cfd = acceptconnection(sockfd); 

		if(cfd!=-1)
		{
			if( (getinput(cfd,input,inputsize,size,INPUT_MAX)!=INPUT_OK) )
			{
				send_error(DATA_F,cfd);
				flow=1;
			}

			if( (flow==0) && (getuserdetails(input[0],userdetails) != USER_OK) )
			{
				send_error(er,cfd);
				flow=1;
			}

			if((flow==0) && (sendpage(cfd,userdetails)!=PAGE_OK)  )
			{
				send_error(DATA_F,cfd);
				flow=1;	
			}

		}
		fflush(NULL);
		close(cfd);
	}

	closeprog();
	return 0;
}
