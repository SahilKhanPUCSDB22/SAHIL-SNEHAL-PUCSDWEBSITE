#include"/root/website/headers/front.c"
#define portnum 1241

enum key 
{
	KEY_SESSION,
	KEY_TEST,
	KEY_QNO,
	KEY_QLABEL,
	KEY_TYPE,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX] = 
{
	{ kvalid_stringne ,"sessionid" } /* KEY_STRING */,
	{ kvalid_stringne, "testid" } ,
	{ kvalid_stringne, "qno" },
	{ kvalid_stringne, "qlabel" },
	{ kvalid_stringne, "qtype" }
};

enum erno send_input(struct kreq *r)
{
	int l,i=0;	
	
	if(connect_back(portnum)!=CONNECTION_OK)
	{
		return SEND_NA;
	}

	char *input[KEY__MAX];
	
	while(i<KEY__MAX)
	{
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}

	int size = SESSION_LEN + TESTID +QNO+QTYPE+QLABEL+PREV+NEXT+ KEY__MAX;
	
	char *INP_STREAM=malloc(sizeof(char)*size);

	if(create_stream(input,INP_STREAM,KEY__MAX)!=STREAM_OK)
	{
		return SEND_NA;
	}

	if( (l=send(sockfd,INP_STREAM,strlen(INP_STREAM),0))<0)
	{
		err="send failed\n";
		logg(err);
		return SEND_NA;
	}

	free(INP_STREAM);
	
	logg("send ok\n");

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
			khttp_puts(&r,"Failed to Load Page\n");
			return EXIT_FAILURE;
		}

		logg("********loadquestion*********\n");

		if( (re=check_input(&r,KEY__MAX))==INPUT_OK)
		{
			if( send_input(&r) == SEND_OK)
			{
				if( (re=get_page(&r)) != PAGE_OK)
				{
					khttp_puts(&r,"<p>Failure(Page)</p>");
				}
			}
			else
			{
				khttp_puts(&r,"Failure");
			}
		}
		else
		{
			khttp_puts(&r,"Couldnt get page , try again");
		}
	}
	closeprog();

	khttp_free(&r);
	return 0;
}

