#include"/root/website/headers/front.c"

#define portno 1243


enum key{
	KEY_SESS,
	KEY_TEST,
	KEY_RAND,
	KEY_ATT,
	KEY_TYPE,
	KEY_REVIEW,
	KEY_STUDL,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX]={
	{kvalid_stringne,"sessionid"},
	{kvalid_stringne,"testid"},
	{kvalid_stringne,"random"},
	{kvalid_stringne,"attempts"},
	{kvalid_stringne,"type"},
	{kvalid_stringne,"review"},
	{kvalid_stringne,"studl"},
};

static enum erno send_list(struct kreq* r)
{
	char *input = (char*)r->fieldmap[KEY_STUDL]->parsed.s;

	int i=0;
	int size = mylen(input);
	
	int ptr=0,n=0;

	while(size>0)
	{
		if(size<CAPACITY)
		{
			n=size;
		}
		else
		{
			n=CAPACITY;
		}

		if( (i=send(sockfd,&input[ptr],n,0)) < 0)
		{
			logg("Failed to send input\n");
			return SEND_NA;
		}

		size -= i;
		ptr += i;
	}

	return SEND_OK;

}
static enum erno send_input(struct kreq *r){

	if( (connect_back(portno) ) != CONNECTION_OK)
	{
		logg("Connection Failure\n");
		return SEND_NA;
	}

	char i=0;

	char *input[KEY__MAX];

	while(i < KEY__MAX){	
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}

	int size = SESSION_LEN + TESTID + RANDOM + ATTEMPTS + TYPE + REVIEW + KEY__MAX;

	char *INPUT_ST =malloc(sizeof(char)*size);

	if((create_stream(input,INPUT_ST,(KEY__MAX-1)) != STREAM_OK))
	{
		logg("Stream Creation Failed\n");
		return SEND_NA;
	}

	if( (send(sockfd,INPUT_ST,mylen(INPUT_ST),0)) < 0)
	{
		logg("Failed To Send Inputs\n");
		return SEND_NA;
	}	

	free(INPUT_ST);
	return SEND_OK;
}

int main(){
	struct kreq r;
	enum khttp er;
	enum erno re;


	if(khttp_parse(&r,keys,KEY__MAX,pages,PAGE__MAX,PAGE_INDEX)!=KCGI_OK)
	{
		return 1;
	}

	if((er=sanitise(&r))!=KHTTP_200)
	{
		emit_error(&r,er);
	}
	else
	{
		emit_header(&r);

		khttp_body(&r);

		if(open_log()){
			return EXIT_FAILURE;
		}
		
		logg("---SubmitTest----");
	
		if((check_input(&r,KEY__MAX))!=INPUT_OK)
		{
			khttp_puts(&r,"Provide Proper Input");
		}
		else
		{
			if (send_input(&r)==SEND_NA)
			{
				khttp_puts(&r,"Failed to Send Input\n");	
			}
			else
			{
				if(send_list(&r)!=SEND_OK)
				{
					khttp_puts(&r,"Failed to Send sdata\n");
				}
				else
				{
					if( get_page(&r)!=PAGE_OK )
					{
						khttp_puts(&r,"Response Invalid \n");
					}

				}	
				logg("submit test\n");
			}
		
		}
	}
	closeprog();
	khttp_free(&r);
	return 0;
}
