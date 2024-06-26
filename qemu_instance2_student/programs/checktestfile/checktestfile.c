#include"/root/website/headers/front.c"

#define portno 1245

enum key{
	KEY_STRING1,
	KEY_STRING2,
	KEY_QNO,
	KEY_FILENAME,
	KEY__MAX
};

#define nonfileinputs 3
#define fileinputs 2

static const struct kvalid keys[KEY__MAX]={
	{kvalid_stringne,"sessionid"},
	{kvalid_stringne,"testid"},
	{kvalid_stringne,"qno"},
	{NULL,"file"}
};

static enum erno send_input(struct kreq *r){
	
	if( (connect_back(portno) ) != CONNECTION_OK)
	{
		logg("Connection Failure\n");
		return SEND_NA;
	}

	char i=0;
	
	char *input[nonfileinputs+fileinputs],*tmp=malloc(sizeof(char)*(CAPACITY+1));;
	
	while(i < nonfileinputs)
	{	
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}
	
	input[i++]=(char*)r->fieldmap[KEY_FILENAME]->file;

	myitoa((int)r->fieldmap[KEY_FILENAME]->valsz,tmp);

	input[i++]=tmp;
	
	int size = SESSION_LEN + TESTID + QNO + FNAME_LEN +FSIZE_LEN+ KEY__MAX + 1;
	
	char *INPUT_ST  =  malloc(sizeof(char)*size);
	
	if((create_stream(input,INPUT_ST,fileinputs+nonfileinputs) != STREAM_OK))
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
	
	free(tmp);
	free(INPUT_ST);
	return SEND_OK;
}

int main(){
	struct kreq r;
	enum khttp er;
	enum erno re;

	

	if(khttp_parse(&r,keys,KEY__MAX,pages,PAGE__MAX,PAGE_INDEX)!=KCGI_OK){
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
			return EXIT_FAILURE;
		}
	
		logg("-------Checking File------\n");

		if((check_input(&r,KEY__MAX))!=INPUT_OK)
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
			{
				if( get_page(&r) != PAGE_OK )
				{
					khttp_puts(&r,"Response Invalid \n");
				}
			}
		}
	}
	closeprog();
	khttp_free(&r);
	return 0;
}
