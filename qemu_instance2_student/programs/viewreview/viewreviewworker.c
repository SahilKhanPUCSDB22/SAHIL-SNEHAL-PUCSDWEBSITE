#include"/root/website/headers/back.c"
#include"/root/website/headers/test.c"

#define INPUT_MAX 1

#define review "/root/website/files/pages/viewreview.html"

#define fileStart "<!DOCTYPE html>\n<html lang=\"en\">\n<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>Welcome</title>\n<link rel=\"stylesheet\" href=\"/css-bin/csssnehal.css\">\n</head>\n<body>"

#define link "<a href=\"#\" onclick=\"viewtest('"
#define linkMid "'); return false;\">"
#define linkEnd "</a>"
#define Heading "<h1 style=\"background-color:lightgreen;\" >Test List</h1>\n"
#define tableStart "<table><thead>\n"
#define tableHeading "<tr>\n<th>Serial No</th>\n<th>TestId Link</th>\n<th>Test Name</th>\n</tr>\n</thead>\n<tbody>"
#define rowStart "<tr>\n"
#define columnstart "<td>"
#define columnEnd "</td>\n"
#define rowEnd "<tr>\n"
#define tableEnd "</tbody>\n</table>\n"

#define fileEnd "<form hidden id=\"view\" action=/cgi-bin/viewanswer.cgi method=\"POST\">\n <input id=\"session\" type=\"text\" name=\"sessionid\" value=\"%s\">\n<input id=\"testId\" name=\"ID\">\n</form>\n<script>\nfunction  viewtest(testId)\n{ var form=document.getElementById(\'view\'); \nvar tid=document.getElementById('testId');\n tid.value=testId;\n form.submit();\n}</script>\n</body>\n</html>"

#define NoTest "<h1>No Test Created Yet</h1>\n<p><a href=\"#\" onclick=\"goBack()\">Home Page</a></p>\n<p><a href=\"#\" onclick=\"load_page('/login.html'); return false;\">Logout</a></p>\n</div>\n<form hidden id=\"form\" method=\"POST\">\n<input type=\"hidden\" name=\"sessionid\" value=\"%s\">\n</form>\n<script>\nfunction goBack()\n{window.history.back();\n}\nfunction load_page(scriptname)\n{var form = document.getElementById(\'form\');\nform.action=scriptname;\nform.submit();\n};\n</script>\n</body>\n</html>"

static enum erno view(char **userdetails ,char *filename)
{

	int rows,i=0,sr; total=0; 
	
	char *command,srno,*params[]={userdetails[0]};
	
	
	tp=fopen(filename,"w+");
	
	wp(fileStart);
	

	command="select t.testid ,t.testname from test t , studenttest s where s.testid=t.testid and studid=$1 and t.review='Y'";

	res=PQexecParams(con,(const char*)command,1,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return PAGE_NA;
	}
	rows=PQntuples(res);
	if(rows<=0)
	{
		wp(NoTest);
		return GEN_OK;

	}
	else
	{
		wp(Heading);
		
		wp(tableStart);
		
		wp(tableHeading);
		
		while(i<rows)
		{
			myitoa(i+1,&srno);
			wp(rowStart);
			//serial Number
			wp(columnstart);
			wp(&srno);
			wp(columnEnd);
			//testid link
			wp(columnstart);
			wp(link);
			wp(PQgetvalue(res,i,0));
			wp(linkMid);
			wp(PQgetvalue(res,i,0));
			wp(linkEnd);
			wp(columnEnd);
			//test name
			wp(columnstart);
			wp(PQgetvalue(res,i,1));
			wp(columnEnd);

			wp(rowEnd);
			i=i+1;
			
		}
		wp(tableEnd);
		wp(fileEnd);
	}


		return GEN_OK;
}

static enum erno send_page(int cfd ,char* input,char *filename)
{
	char *buffer=malloc(sizeof(char)*total),*output;

	if((getpage(buffer,filename))>0)
	{
		return PAGE_NA;
	}
	int contentsize[1];
	/*if(gethtmlcontent(udetails,&buffer,contentsize)!=GETHTML_OK)
	{
		return PAGE_NA;
	}*/

	contentsize[0]=total+mylen(input)+1;
	output=malloc(sizeof(char)*contentsize[0]);
	contentsize[0]=sprintf(output,buffer,input);

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

	char *filename;
	
	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("------------Display Interviewer-----------\n");

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
		
		filename=mycat(userdetails[0],"review");
		
		if(flow==0 && (view(userdetails,filename)!=GEN_OK))
		{
			flow=1;
		}
		if( (flow==0) && (send_page(cfd,input[0],filename)!=PAGE_OK))
		{
			flow=1;
		}
	}

	closedb();
	closeprog(cfd);
	return 0;
}
