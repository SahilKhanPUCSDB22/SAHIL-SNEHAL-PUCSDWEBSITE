#include"/root/website/headers/front.c"

#define portno 1238

enum key
{
	KEY_SESS,
	KEY_TEST,
	KEY_RAND,
	KEY_ATT,
	KEY_STUD,
	KEY_REVIEW,
	KEY__MAX

};

static const struct kvalid keys[KEY__MAX]=
{
	{kvalid_stringne,"sessionid"},
	{kvalid_stringne,"testid"},
	{kvalid_stringne,"random"},
	{kvalid_stringne,"attempts"},
	{kvalid_stringne,"type"},
	{kvalid_stringne,"review"}
};

static enum erno send_input(struct kreq *r){
	
	if( (connect_back(portno) ) != CONNECTION_OK)
	{
		logg("Connection Failure\n");
		return SEND_NA;
	}

	char i=0;
	
	char *input[KEY__MAX];
	
	while(i < (KEY__MAX)){	
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}
	
	int size = SESSION_LEN + TESTID + RANDOM + ATTEMPTS + TYPE + REVIEW + KEY__MAX;
	
	char* INPUT_ST = malloc(sizeof(char)*size);
	
	if((create_stream(input,INPUT_ST,KEY__MAX) != STREAM_OK))
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
		if(open_log())
		{
			return EXIT_FAILURE;
		}

		logg("-------Assigning To All------\n"); 
		
		khttp_body(&r);

		if((check_input(&r,KEY__MAX))!=INPUT_OK)
		{
			khttp_puts(&r,"Provide Proper Input");
		}

		else
		{
			if (send_input(&r)==SEND_NA)
			{
				khttp_puts(&r,"Failed to Send Input");	
			}
			else
			{
				if( get_page(&r) )
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
