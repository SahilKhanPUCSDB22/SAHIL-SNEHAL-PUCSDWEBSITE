#include"/root/website/headers/front.c"

#define portno 1242

enum key{
	KEY_SESS,
	KEY_TEST,
	KEY_QTYPE,
	KEY_QNO,
	KEY_ANS,
	KEY__MAX

};

static const struct kvalid keys[KEY__MAX]={
	{kvalid_stringne,"sessionid"},
	{kvalid_stringne,"testid"},
	{kvalid_stringne,"qtype"},
	{kvalid_stringne,"qno"},
	{kvalid_stringne,"answer"},
};

int sendanswer( char* answer)
{
	if((send(sockfd,answer,mylen(answer),0))<0)
	{
		logg("Failed to Send Answer\n");
		return 1;
	}

	return 0;
}

static enum erno send_input(struct kreq *r){
	
	if( (connect_back(portno) ) != CONNECTION_OK)
	{
		logg("Connection Failure\n");
		return SEND_NA;
	}

	char i=0;
	
	char *input[KEY__MAX];
	
	while(i < (KEY__MAX)-1){	
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}
	
	int size = SESSION_LEN +TESTID + QTYPE + QNO + KEY__MAX;
	
	char INPUT_ST[size];
	
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
	
	if( sendanswer( (char *)r->fieldmap[KEY_ANS]->parsed.s ) )
	{
		logg("Failed to send answer\n");
		return SEND_NA;
	}

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
			khttp_puts(&r,"Server Log Error");
			khttp_free(&r);
			return EXIT_FAILURE;
		}

		logg("------------Insert Question--------------\n");

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
				if( get_page(&r)!=PAGE_OK )
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
