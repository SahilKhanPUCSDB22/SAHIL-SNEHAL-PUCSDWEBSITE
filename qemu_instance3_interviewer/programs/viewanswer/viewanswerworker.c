#include"/root/website/headers/back.c"
#include"/root/website/headers/test.c"

#define INPUT_MAX 3

#define review "/root/website/files/pages/viewreview.html"

#define fileStart "<!DOCTYPE html>\n<html lang=\"en\">\n<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>Welcome</title>\n<link rel=\"stylesheet\" href=\"/css-bin/background1.css\">\n</head>\n<body><div class=\"front\">\n<style>\np{font-family: \'Times New Roman\', Times, serif;}\n</style>\n<div class=\"questionans\">\n"

#define queSr "<div class=\"question\">\n<p>Q"
#define point ") "
#define ansNo "<div class=\"answer\">\n<p>Ans"
#define finish " </p></br>\n</div>"

#define fileEnd "<p> finished! </p> \n</div><p><a href=\"#\" onclick=\"goBack()\">Previous Page</a></p>\n<p><a href=\"#\" onclick=\"load_page(\'/login.html\')\">Logout</a></p>\n<p><a href=\"#\" onclick=\"threeback()\">Home Page</a></p></div>\n<form hidden id=\"form\" method=\"POST\">\n<input type=\"hidden\" name=\"sessionid\" value=\"%s\">\n</form>\n<script>\nfunction goBack()\n{window.history.back();}\nfunction threeback()\n {history.go(-3);}\nfunction load_page(scriptname)\n{var form = document.getElementById(\'form\');\nform.action=scriptname;\nform.submit();\n};\n</script>\n</body>\n</html>"



#define NoTest "<h1>NO ANSWER SUBMITED</h1>\n</div><p><a href=\"#\" onclick=\"goBack()\">Previous Page</a></p>\n<p><a href=\"#\" onclick=\"threeback()\">Home Page</a></p>\n<p><a href=\"#\" onclick=\"load_page(\'/login.html\')\">Logout</a></p>\n</div>\n<form hidden id=\"form\" method=\"POST\">\n<input type=\"hidden\" name=\"sessionid\" value=\"%s\">\n</form>\n<script>\nfunction goBack()\n{window.history.back();\n}function threeback()\n {history.go(-3);}\nfunction load_page(scriptname)\n{var form = document.getElementById(\'form\');\nform.action=scriptname;\nform.submit();\n};\n</script>\n</body>\n</html>"

static enum erno mcqdisplay(char **input,char *filename,int *srno)
{

	int rows,row1,i=0; total=0; 
	
	char *command,sr,*params[]={input[1],input[2]};
	
	tp=fopen(filename,"w+");
	
	wp(fileStart);
	
	command="select ques from mquestion where qno in(select qno from manswer where testid=$1 and studid=$2 order by qno asc)";

	res=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return GEN_NA;
	}
	rows=PQntuples(res);
	if(rows<=0)
	{
		return GEN_OK;

	}
	else
	{
		command="select optext from options o join manswer m on m.qno=o.qno and m.answer=o.ono order by m.qno asc";
		PGresult *res1 = PQexec(con,command);

		if(PQresultStatus(res1)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			return GEN_NA;

		}
		row1 = PQntuples(res1);
		while(i<row1)
		{
			myitoa(*srno,&sr);
			wp(queSr);
			wp(&sr);
			wp(point);
			wp(PQgetvalue(res,i,0));
			wp(finish);
			wp(ansNo);
			wp(&sr);
			wp(point);
			wp(PQgetvalue(res1,i,0));
			wp(finish);
			*srno=*srno+1;
			i++;
		}
	}
		return GEN_OK;
}

static enum erno desdisplay(char **input,char *filename,int *srno)
{

	int rows,row1,i=0; 
	
	char *command,sr,*params[]={input[1],input[2]};
	
	
	command="select ques from dquestion where qno in(select qno from danswer where testid=$1 and studid=$2 order by qno asc)";

	res=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return GEN_NA;
	}
	rows=PQntuples(res);
	if(rows<=0)
	{
		return GEN_OK;

	}
	else
	{
		command="select answer from danswer where testid=$1 and studid=$2 order by qno asc";
		PGresult *res1 = PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

		if(PQresultStatus(res1)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			return GEN_NA;

		}
		row1 = PQntuples(res1);
		while(i<row1)
		{
			myitoa(*srno,&sr);
			wp(queSr);
			wp(&sr);
			wp(point);
			wp(PQgetvalue(res,i,0));
			wp(finish);
			wp(ansNo);
			wp(&sr);
			wp(point);
			wp(PQgetvalue(res1,i,0));
			wp(finish);
			*srno=*srno+1;
			i++;
		}
	}

		return GEN_OK;
}

static enum erno filedisplay(char **input,char *filename,int *srno)
{

	int rows,row1,i=0; 
	
	char *command,sr,*params[]={input[1],input[2]};
	
	command="select ques from fquestion where qno in(select qno from fanswer where testid=$1 and studid=$2 order by qno asc)";

	res=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return GEN_NA;
	}
	rows=PQntuples(res);
	if(rows<=0)
	{
		return GEN_OK;

	}
	else
	{
		command="select answer from fanswer where testid=$1 and studid=$2 order by qno asc";

		PGresult* res1=PQexecParams(con,(const char*)command,2,NULL,(const char* const*)params,NULL,NULL,0);
		
		if(PQresultStatus(res1)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			return GEN_NA;

		}
		row1 = PQntuples(res1);
		while(i<row1)
		{
			myitoa(*srno,&sr);
			wp(queSr);
			wp(&sr);
			wp(point);
			wp(PQgetvalue(res,i,0));
			wp(finish);
			wp(ansNo);
			wp(&sr);
			wp(point);
			wp(PQgetvalue(res1,i,0));
			wp(finish);
			*srno=*srno+1;
			i++;
		}
	}

		return GEN_OK;
}




static enum erno send_page(int cfd ,char**input,char *filename)
{
	if(total==329)
	{
		wp(NoTest);
	}
	else
	{
		wp(fileEnd);
	}
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

	contentsize[0]=total+mylen(input[0])+1;
	output=malloc(sizeof(char)*contentsize[0]);
	contentsize[0]=sprintf(output,buffer,input[0]);

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
	char *input[INPUT_MAX],*userdetails[2],flow=0;

	int streamlen= SESSION_LEN + TESTID + USERID + INPUT_MAX , size[]={SESSION_LEN,TESTID,USERID},cfd,srno=1;

	cfd  =  myatoi(argv[1]);

	char *filename;
	
	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("------------Student Answer Display-----------\n");

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
		filename=mycat(userdetails[0],"answer");
		
		if(flow==0 && (mcqdisplay(input,filename,&srno)!=GEN_OK))
		{
			flow=1;
		}
		if(flow==0 && (desdisplay(input,filename,&srno))!=GEN_OK)
		{
			flow=1;
		}
		if(flow==0 && (filedisplay(input,filename,&srno))!=GEN_OK)
		{
			flow=1;
		}
		
		if( (flow==0) && (send_page(cfd,input,filename)!=PAGE_OK))
		{
			flow=1;
		}
	}
	closedb();
	closeprog(cfd);
	return 0;
}
