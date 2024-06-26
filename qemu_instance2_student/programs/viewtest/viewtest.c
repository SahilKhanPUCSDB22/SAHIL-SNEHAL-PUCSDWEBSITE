#include"/root/website/headers/front.c"
#define portnum 1248

enum key 
{
	KEY_SESSION,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX] = 
{
	{ kvalid_stringne , "sessionid" } /* KEY_STRING */
};


enum erno send_input(struct kreq *r)
{
	int l;	
	
	if(connect_back(portnum)!=CONNECTION_OK)
	{
		return SEND_NA;
	}

	char *input[KEY__MAX];
	
	input[0]=(char*)r->fieldmap[KEY_SESSION]->parsed.s;	

	int size=SESSION_LEN+1;
	
	char INP_STREAM[size];

	if(create_stream(input,INP_STREAM,KEY__MAX)!=STREAM_OK)
	{
		return SEND_NA;
	}

	if( (l=send(sockfd,INP_STREAM,strlen(INP_STREAM),0))<0)
	{
		err="send fail\n";
		logg(err);
		return SEND_NA;
	}

	err="send ok\n";
	logg(err);

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

		logg("Viewtest\n");

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

