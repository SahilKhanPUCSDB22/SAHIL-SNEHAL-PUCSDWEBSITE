#include"/root/website/headers/front.c"

#define portnum 1244

enum key {
	KEY_STRING1,
	KEY_STRING2,
	KEY_STRING3,
	KEY_STRING4,
	KEY_MAX
};

static const struct kvalid keys[KEY_MAX] = {
	{ kvalid_stringne, "sessionid" },
	{ kvalid_stringne, "testid" },
	{ kvalid_stringne, "sdate"},
	{ kvalid_stringne, "stime"}
};

static enum erno send_input(struct kreq *r)
{
	if(connect_back(portnum)!=CONNECTION_OK)
	{
		return SEND_NA;
	}

	int size = SESSION_LEN+TESTID+DATE+TIME+KEY_MAX;
	
	char *STREAM = malloc(sizeof(char)*size),*input[KEY_MAX];
	
	int i=0;
	while(i<KEY_MAX)
	{
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}

	if(create_stream(input,STREAM,KEY_MAX)!=STREAM_OK)
	{
		return SEND_NA;
	}

	if( send(sockfd,STREAM,mylen(STREAM),0) < 0 )
	{
		logg("Failed to send input\n");
		return SEND_NA;
	}

	return SEND_OK;
}

int main(void) {
	struct kreq r;
	enum khttp er;

	if (khttp_parse(&r, keys, KEY_MAX,pages, PAGE__MAX, PAGE_INDEX) != KCGI_OK)
	{
		return 0;
	}

	if ((er = sanitise(&r)) != KHTTP_200) 
	{
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
		else
		{
			logg("------SubmitTestInitiated-----\n");

			if( check_input(&r,KEY_MAX) != INPUT_OK)
			{
				khttp_puts(&r,"Backend Server Failure");
				logg("Failed to parse inputs\n");
			}
			else
			{
				if(send_input(&r)!=SEND_OK)
				{
					khttp_puts(&r,"Failure to Process Request\n");
					logg("Failed to send input\n");
				}
				else
				{
					if(get_page(&r)!=PAGE_OK)
					{
						khttp_puts(&r,"Failure to get response from server\n");
					}
				}
			}
		}
	}
	closeprog();
	khttp_free(&r);
	return 0;
}


