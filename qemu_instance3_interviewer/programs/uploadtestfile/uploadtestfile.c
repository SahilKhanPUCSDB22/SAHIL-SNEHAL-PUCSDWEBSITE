#include"/root/website/headers/front.c"

#define portno 1245

enum key{
	KEY_SESSION,
	KEY_TESTID,
	KEY_UPLOADTYPE,
	KEY_QNO,
	KEY_QUESTION,
	KEY_FILE,
	KEY_FILENAME,
	KEY_FILESIZE,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX]={
	{kvalid_stringne,"sessionid"},
	{kvalid_stringne,"testid"},
	{kvalid_stringne,"utype"},
	{kvalid_stringne,"qno"},
	{kvalid_stringne,"question"},
	{NULL,"file"},
};

#define nonfileinputs 5
#define fileinputs 2

static enum erno send_input(struct kreq *r){
	
	if( (connect_back(portno) ) != CONNECTION_OK)
	{
		logg("Connection Failure\n");
		return SEND_NA;
	}

	char i=0;
	
	char *input[nonfileinputs+fileinputs],*temp = malloc(sizeof(char)* (CAPACITY+1));
	
	while(i < nonfileinputs )
	{	
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}

	input[i++]=(char*)r->fieldmap[KEY_FILE]->file;  //filename
	myitoa((int)r->fieldmap[KEY_FILE]->valsz,temp);
	input[i++]=temp;
	
	int size = SESSION_LEN + TESTID + UTYPE + QNO + CAPACITY + FNAME_LEN + FSIZE_LEN+  KEY__MAX;
	
	char *INPUT_ST  =  malloc(sizeof(char)*size);
	
	if((create_stream(input,INPUT_ST,(nonfileinputs+fileinputs)) != STREAM_OK))
	{
		logg("Stream Creation Failed\n");
		return SEND_NA;
	}

	if( (send(sockfd,INPUT_ST,mylen(INPUT_ST),0)) < 0)
	{
		logg("Failed To Send Inputs\n");
		return SEND_NA;
	}
	else
	{
		logg("Input send\n");
	}
	
	free(temp);
	free(INPUT_ST);
	return SEND_OK;
}


int getchunk( char *buffer)
{
	int n;
	if( (n=recv(sockfd,buffer,SEG_SIZE,0)) <= 0)
	{
		logg("Falied to get chunk\n");	
		return 1;
	}

	buffer[n]=0;
	logg(buffer);
	return 0;
}

enum erno send_file(struct kreq* r)
{
	char *buffer=malloc(sizeof(char)* (CAPACITY+1));
	
	if( getchunk(buffer) )
	{
		return PAGE_NA;
	}
	
	if(buffer[0]>65)
	{
		logg("Falied to get chunk no\n");	
		return PAGE_NA;
	}
	else
	{
		int segno,fsize,maxseg,pos,left,buflen,re,flag=0;

		segno = myatoi((char*)buffer);

		fsize = (int)r->fieldmap[KEY_FILE]->valsz;

		maxseg= MAX_SEG(fsize);;

		logg(",");
		char txmp[1024];
		myitoa(maxseg,txmp);
		logg(txmp);
		
		left = fsize - SEG_SIZE*segno;

		char *file = (char*) r->fieldmap[KEY_FILE]->val;

		pos = segno*SEG_SIZE;

		while(segno<maxseg && (flag==0))
		{
			if(left<SEG_SIZE)
                	{
                        	buflen=left;
                	}
                	else
                	{
                        	buflen=SEG_SIZE;
                	}
	
                	if( (re=send(sockfd,(void*)(file+pos),buflen,0)) < 0 )
                	{
                        	logg("File rec failed");
                        	flag=1;
                	}

			if( (re==buflen) && (flag==0))
			{
				left-=buflen;
				pos+=buflen;
				segno++;
			}
			else
			{
				flag=1;
			}
		}
	}
	free(buffer);
	return PAGE_OK;
}

int main(){

	struct kreq r;
	enum khttp er;
	enum erno re;

	if(khttp_parse(&r,keys,(nonfileinputs+1),pages,PAGE__MAX,PAGE_INDEX)!=KCGI_OK){
		return 1;
	}
	if((er=sanitise(&r))!=KHTTP_200){
		emit_error(&r,er);
	}
	
	else
	{
		emit_header(&r);
		
		khttp_body(&r);
		
		if(open_log())
		{
			khttp_puts(&r,"Server Log Error\n");
			khttp_free(&r);
			return EXIT_FAILURE;
		}

		logg("------UploadingFile------\n");

		if((check_input(&r,KEY__MAX-3))!=INPUT_OK)
		{
			khttp_puts(&r,"Provide Proper Input");
		}

		else
		{
			if (send_input(&r)!=SEND_OK)
			{
				khttp_puts(&r,"Failed to Send Input");	
			}
			else
			{	if(send_file(&r)!=PAGE_OK)
				{
					khttp_puts(&r,"Failed to Upload File.Please Try Again");
				}
				else
				{
					if( get_page(&r) != PAGE_OK )
					{
						khttp_puts(&r,"Response Invalid \n");
					}
				}
			}
		}
	}
	closeprog();
	khttp_free(&r);
	return 0;
}
