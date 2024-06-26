#include"/root/website/headers/back.c"
#include"/root/website/headers/test.c"

#define path "/root/website/files/list/"

#define INPUT_MAX 6

#define studentlist "/root/website/files/pages/studentlist.html"

static enum erno generatecheck(char **input, char **gen){

	char *command,*filename = (mycat(path,input[1]));

	if( (tp=fopen(filename,"w+"))==NULL)
	{
		logg("Failed to open file for list\n");
		return GEN_NA;
	}

	command="select stud_id,stud_fname,stud_lname,stud_stream,stud_degree from students";
	
	res=PQexec(con,command);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return GEN_NA;
	}

	int rows=PQntuples(res);
	
	char *str0="<label style=\"font-size:14px;\" >StudId || Student Name || Student's Stream || Student's Degree</label><br><br>";
	char *str1="<input type=\"checkbox\" id=\"";
	char *str2="\"name=\"studsl\" value=\"";
	char *str3="\">\n<label for=\"";
	char *str4="\">";
	char *str5="</label><br>\n";

	int i=0;
	int j=0;
	wp(str0);

	while(i<rows){
		wp(str1);
		wp(PQgetvalue(res,i,0));
		wp(str2);
		wp(PQgetvalue(res,i,0));
		wp(str3);
		wp(PQgetvalue(res,i,0));
		wp(str4);
		while(j<5){
			wp(PQgetvalue(res,i,j));
			wp("\t");
			j++;
		}
		j=0;
		wp(str5);
		i++;
	}

	char * buffer = malloc(sizeof(char)*total);

	if(getpage(buffer,filename))
	{
		return GEN_NA;
	}

	gen[0]=buffer;

	return GEN_OK;

}
static enum erno send_page(int cfd ,char** input,char **gen)
{
	char *buffer,*output;
	int contentsize[1];
	
	if(gethtmlcontent(studentlist,&buffer,contentsize)!=GETHTML_OK)
	{
		return PAGE_NA;
	}

	contentsize[0]+=mylen(gen[0])+mylen(input[0])+mylen(input[1])+mylen(input[2])+mylen(input[3])+mylen(input[4])+mylen(input[5])+1;
	
	output=malloc(sizeof(char)*contentsize[0]);
	
	contentsize[0]=sprintf(output,buffer,gen[0],input[0],input[1],input[2],input[3],input[4],input[5]);
	
	if(sendhtmlcontent(cfd,output,contentsize[0])!=SENDHTML_OK)
	{
		return PAGE_NA;
	}

	free(buffer);
	free(output);
	

	return PAGE_OK;
}


int main(int argc , char *fd[])
{
	enum erno er;
	char *gen[1];
	
	int cfd,inpsize = TESTID+SESSION_LEN +RANDOM+ATTEMPTS+TYPE+REVIEW ;
	
	int size[] = {SESSION_LEN,TESTID,RANDOM,ATTEMPTS,TYPE,REVIEW};

	char *input[INPUT_MAX],*userdetails[2];

	cfd =myatoi(fd[1]);

	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("-----SelectStudentsWorker------");

	if(connect_db())
	{
		return EXIT_FAILURE;
	}

	char flow=0;
	if(cfd!=-1)
	{

		if( getinput(cfd,input,inpsize,size,INPUT_MAX) == INPUT_NA )
		{
			flow=1;
		}

		if( flow==0 && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=1;
		}
		if(flow==0 && (generatecheck(input,gen)!=GEN_OK))
		{
			flow=1;
		}
		if((flow==0) && (send_page(cfd,input,gen)!=PAGE_OK))
		{
			flow=1;
		}

	}
	closedb();
	closeprog(cfd);
	return 0;
}
