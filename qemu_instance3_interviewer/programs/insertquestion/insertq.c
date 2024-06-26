#include"/root/website/headers/front.c"

#define portnum 1241

enum key {
	KEY_SESSION,
	KEY_TESTID,
	KEY_QTYPE,
	KEY_QNO,
	KEY_NOP,
	KEY_QUESTION,
	KEY_OPTIONS,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX] = 
{
	{ kvalid_stringne, "sessionid"},
	{ kvalid_stringne, "testid"},
	{ kvalid_stringne, "qtype"},
	{ kvalid_stringne, "qno"},
	{ kvalid_stringne, "nop"},
	{ kvalid_stringne, "question" },
	{ kvalid_stringne, "options" }
};

static enum erno send_input(struct kreq* r )
{
	if ( connect_back(portnum)!=CONNECTION_OK)
	{
		return SEND_NA;
	}

	int i=0 ,streamsize = SESSION_LEN + TESTID + QTYPE + QNO + CAPACITY + KEY__MAX;
	
	char *input[KEY__MAX-2];
	
	while(i<(KEY__MAX-2))
	{
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}

	char *INP_STREAM = malloc(sizeof(char)*streamsize);

	if( create_stream(input,INP_STREAM,(KEY__MAX-2))!=STREAM_OK )
	{
		return SEND_NA;
	}

	if( send(sockfd,INP_STREAM , mylen(INP_STREAM) , 0) < 0 )
	{
		logg("Failed to send inputs to server\n");
		return SEND_NA;
	}

	free(INP_STREAM);

	return SEND_OK;
}	

static enum erno sendqdata(struct kreq * r)
{
	char *quesopt[4];
	quesopt[1] = (char*) r->fieldmap[KEY_QUESTION]->parsed.s;
	quesopt[3] = (char*) r->fieldmap[KEY_OPTIONS]->parsed.s;

	int ql = mylen( quesopt[1] ) , ol = mylen( quesopt[3] );

	quesopt[0]=malloc(sizeof(char)*(ql+1));
	quesopt[2]=malloc(sizeof(char)*(ol+1));

	myitoa(ql,quesopt[0]);

	myitoa(ol,quesopt[2]);

	char *stream = malloc(sizeof(char)* ( (ql*2) + (ol*2) + 5) );

	if( create_stream(quesopt , stream , 4) != STREAM_OK)
	{
		logg("Failed to create qa stream\n");
		return SEND_NA;
	}

	int size = mylen(stream);
	
	if( send_stream(stream,size) != STREAM_OK)
	{
		logg("Failed to send qdata\n");
		return SEND_NA;
	}

	free(stream);
	free(quesopt[0]);
	free(quesopt[2]);

	return SEND_OK;
}

int main(void)
{
	struct kreq r;
	enum khttp er;
	enum erno re;
	
	if (khttp_parse( &r , keys , KEY__MAX , pages , PAGE__MAX , PAGE_INDEX ) != KCGI_OK )
	{
		return 0;
	}

	if( (er=sanitise(&r)) != KHTTP_200)
	{
		emit_error(&r,er);
		return 0;
	}
	else 
	{
		emit_header(&r);		
		
		khttp_body(&r);
		
		if(open_log())
		{
			khttp_puts(&r,"Failed to open log");
			khttp_free(&r);
			return 0;
		}
		
		logg("------Insertq Initiated----\n");
			
		if(check_input(&r,KEY__MAX)!=INPUT_OK)
		{
			khttp_puts(&r,"INPUT ERROR");
		}
		else
		{
			if(send_input(&r)!=SEND_OK)
			{
				khttp_puts(&r,"Failed to Connect to server.Please try again");
			}
			else
			{
				if(sendqdata(&r)!=SEND_OK)
				{
					khttp_puts(&r,"Failed to Process Data.Please try again");
				}
				else
				{
					if(get_page(&r)!=PAGE_OK)
					{
						khttp_puts(&r,"Failed to recv response from server.Please try again");
					}
				}
			}
		}
	}
	closeprog();
	khttp_free(&r);
	return 0;
}

