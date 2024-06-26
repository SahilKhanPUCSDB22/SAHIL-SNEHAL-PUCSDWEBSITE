#include"/root/website/headers/front.c"

#define portnum 1251

enum key {
	KEY_SESSIONID,
	KEY_TESTID,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX] = 
{
	{ kvalid_stringne, "sessionid" },
	{ kvalid_stringne, "ID" }
};

static enum erno send_input(struct kreq* r )
{
	if ( connect_back(portnum)!=CONNECTION_OK)
	{
		return SEND_NA;
	}

	int i=0,streamsize= SESSION_LEN + TESTID + KEY__MAX;

	char *input[KEY__MAX];
	
	while(i<KEY__MAX)
	{
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}

	char *INP_STREAM = malloc(sizeof(char)*streamsize);

	if(create_stream(input,INP_STREAM,KEY__MAX)!=STREAM_OK)
	{
		return SEND_NA;
	}

	if( send(sockfd,INP_STREAM , mylen(INP_STREAM) , 0) < 0 )
	{
		logg("Failed to send inputs to server\n");
		return SEND_NA;
	}

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
		if(open_log())
		{
			return 0;
		}
		logg("Page Request initiated\n");
		emit_header(&r);		
		khttp_body(&r);
		
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
				if(get_page(&r)!=PAGE_OK)
				{
					khttp_puts(&r,"Failed to recv response from server");
				}
			}
		}
	}
	closeprog();
	khttp_free(&r);
	return 0;
}

