#include"/root/website/headers/back.c"

#define INPUT_MAX 6

#define path "/root/website/files/testfiles/"

#define getchunk "select parts from fanswer where studid=$1 and testid=$2 and  qno=$3"

#define delete "delete from fanswer where studid=$1 and testid=$2 and qno=$3"

#define insert "insert into fanswer values($1,$2,$3,$4,$5,0,NULL)"

#define updatechunk "update fanswer set parts = $1 where studid=$2 and testid=$3 and qno=$4"

#define updatecontent "update fanswer set parts = $1 , answer =  $2 where studid=$3 and testid=$4 and qno=$5"

#define uploadsuccess "File Uploaded Sucessfully"


enum indexex { session , testid , utype , qno ,fname , fsize };

static enum erno send_page(int cfd)
{	
	if(sendhtmlcontent(cfd,uploadsuccess,mylen(uploadsuccess))!=SENDHTML_OK)
	{
		return PAGE_NA;
	}
	return PAGE_OK;
}

int error()
{
	if(PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return 1;
	}
	return 0;
}

int new(char ** params)
{
	res=PQexecParams(con,delete,3,NULL,(const char * const * )params,NULL,NULL,0);
	
	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return 1;
	}	
	
	res=PQexecParams(con,insert,5,NULL,(const char * const * )params,NULL,NULL,0);
	
	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		logg(PQerrorMessage(con));
		return 1;
	}
	else
	{
//		printf("IN db\n");
	}
	return 0;
}

int old(char ** params , char *chunk)
{
	res=PQexecParams(con,getchunk,3,NULL,(const char * const * )params,NULL,NULL,0);

	if(error())
	{
		return 1;
	}

	if(PQntuples(res)>0)
	{
		chunk=PQgetvalue(res,0,0);
	}
	else
	{
		logg("No value Available in db\n");
		return 1;
	}

	return 0;
}

static enum erno updatetable(char **input  , char **userdetails , char *chunk)
{
	char *params[]={userdetails[0],input[testid],input[qno],input[fname],input[fsize]};

	if( input[2][0]=='n')
	{
		if(new(params))
		{
			return UPDATE_NA;
		}
		chunk[0]='0';
		chunk[1]=0;
	}
	else
	{
		if(old(params,chunk))
		{
			return UPDATE_NA;
		}
		
	}

	return UPDATE_OK;
}

int sendchunk(int sockfd, char *chunk)
{
	if( send(sockfd,chunk,mylen(chunk),0) < 0 )
	{
		logg("Failed to send parts\n");
		return 1;
	}
	return 0;
}

enum erno getfile(int sockfd ,char **input , char **userdetails,char *chunk)
{
	char *params[5],*command,*command2; //params command
	
	char *filename= malloc(sizeof(char)*CAPACITY*2);
	
	filename[0]=0;
	
	strcat(strcat(strcat(strcat(filename,path),userdetails[0]),input[testid]),input[qno]);
		
	char parts[CAPACITY]; //no of parts for query
	
	int n= myatoi(chunk);  //no of parts for loop

	int filesize = myatoi( (char*)input[fsize]);  //sizeof file

	int max = MAX_SEG(filesize);  //max no of segments

	int r,flag=0,x; 

	FILE* fp;

	if(sendchunk(sockfd,chunk))
	{
		return FILE_NA;
	}
	
	if(input[utype][0]=='o')
	{
		fp=fopen(filename,"a+");
		
		if(fp==NULL)
		{
			logg("Error Failure opening temp file\n");
			return FILE_NA;
		}
	}
	else
	{
		fp=fopen(filename,"w+");
		
		if(fp==NULL)
		{
			logg("Error Failure opening temp file\n");
			return FILE_NA;
		}
	}


	int buflen=0;
	
	int left= filesize - (n*SEG_SIZE);

	char buffer[SEG_SIZE*2];

	while(n<max && (flag==0))
	{
		if(left<SEG_SIZE)
		{
			buflen=left;
		}
		else
		{
			buflen=SEG_SIZE;
		}

		if( (r=recv(sockfd,(void*)buffer,buflen,0)) < 0 )
		{
			logg("File rec failed");
			flag=1;
		}

		if(r==buflen)
		{
			r=fwrite((void*)buffer,sizeof(char),r,fp);
			left-=buflen;
			n++;
		}
		else
		{
			flag=1;
		}
	}
	

	myitoa(n,parts);

	params[0]=parts;
	params[1]=userdetails[0];
	params[2]=input[testid];
	params[3]=input[qno];
	
	res=PQexecParams(con,(const char*)updatechunk,4,NULL,(const char* const*)params,NULL,NULL,0);

	if(PQresultStatus(res)!=PGRES_COMMAND_OK)
	{
		err="Query execution failed fun:parts Inclusion\n";
		logg(err);
		logg(PQerrorMessage(con));
		fclose(fp);
		return FILE_NA;
	}

	if(n==max)
	{
		fseek(fp,0,SEEK_SET);
		
		char tmpbuf[filesize+1];

		int red = fread((void*)tmpbuf,sizeof(char),filesize,fp);
		
		tmpbuf[red]=0;

		fclose(fp);

		char *paramz[]={parts,tmpbuf,userdetails[0],input[testid],input[qno]};

		res=PQexecParams(con,(const char*)updatecontent,5,NULL,(const char* const*)paramz,NULL,NULL,0);
		
		if(PQresultStatus(res)!=PGRES_COMMAND_OK)
		{
			err="Query execution failed fun:data insertion\n";
			logg(err);
			logg(PQerrorMessage(con));
			return FILE_NA;
		}

		if(remove(filename)<0)
		{
			logg("Failed to remove File");
		}	
	}
	else
	{
		fclose(fp);
		return FILE_NA;
	}

	return FILE_OK;
}

int main(int argc , char *fd[])
{
	int cfd , inpsize = SESSION_LEN + TESTID + UTYPE_LEN + FNAME_LEN + FSIZE_LEN + QNO ;

	int size[]={ SESSION_LEN , TESTID , UTYPE_LEN , QNO ,FNAME_LEN  , FSIZE_LEN };

	char *input[INPUT_MAX],*userdetails[2],chunk[CAPACITY/2];

	cfd = myatoi(fd[1]);
	
	if(open_log())
	{
		return EXIT_FAILURE;
	}

	logg("------Uploading File-----\n");

	if(connect_db())
	{
		return EXIT_FAILURE;
	}
	
	char flow=0;
	
//	printf("Started\n");
	if(cfd  != -1)
	{

		if( getinput(cfd,input,inpsize,size,INPUT_MAX) != INPUT_OK )
		{
			flow=1;
		}
		else
		{
		//	printf("input ok\n");
		}

		fflush(NULL);
		if( (flow==0) && (getuserdetails(input[0],userdetails)!=USER_OK))
		{
			flow=sessionexpired(cfd);
		}
		else
		{
		//	printf("User ok\n");
		}

		if(flow==0 &&  ( updatetable( input , userdetails,chunk )) != UPDATE_OK )
		{
			flow=1;
		}
		else
		{
		//	printf("Update ok\n");
		}

		if( (flow==0) && (getfile(cfd,input,userdetails,chunk))!=FILE_OK )
		{
			flow=1;
		}
		
		if( (flow==0) && ( send_page(cfd) ) != PAGE_OK )
		{
			flow=1;
		}
	}
	
	closedb();
	closeprog(cfd);
	
	return 0;
}
