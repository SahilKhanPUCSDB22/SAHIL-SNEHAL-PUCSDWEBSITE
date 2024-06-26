#include"/root/website/headers/front.c"

#define portno 1236

#define ID 1024
#define name 1024
#define stime 12
#define etime 12
#define sdate 12
#define edate 12
#define type 2

enum key{
	KEY_STRING1,
	KEY_STRING2,
	KEY_STRING3,
	KEY_STRING4,
	KEY_STRING5,
	KEY_STRING6,
	KEY_STRING7,
	KEY_STRING8,
	KEY__MAX

};

static const struct kvalid keys[KEY__MAX]={
	{kvalid_stringne,"sessionid"},
	{kvalid_stringne,"ID"},
	{kvalid_stringne,"name"},
	{kvalid_stringne,"sdate"},
	{kvalid_stringne,"stime"},
	{kvalid_stringne,"edate"},
	{kvalid_stringne,"etime"},
	{kvalid_stringne,"type"},
};

static enum erno send_input(struct kreq *r){
	
	if( (connect_back(portno) ) != CONNECTION_OK)
	{
		logg("Connection Failure\n");
		return SEND_NA;
	}

	char i=0;
	
	char *input[KEY__MAX];
	
	while(i < (KEY__MAX))
	{	
		input[i]=(char*)r->fieldmap[i]->parsed.s;
		i++;
	}
	
	int size = SESSION_LEN + ID + name + sdate + stime  + edate + etime + type + KEY__MAX;
	
	char INPUT_ST[size];
	
	if((create_stream(input,INPUT_ST,KEY__MAX) != STREAM_OK)){
		logg("Stream Creation Failed\n");
		return SEND_NA;
	}

	logg(INPUT_ST);

	if( (send(sockfd,INPUT_ST,mylen(INPUT_ST),0)) < 0){
			logg("Failed To Send Inputs\n");
			return SEND_NA;
		}
	else{
		logg("Input send\n");
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
			return EXIT_FAILURE;
		}

		logg("------Create Test-----\n");

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
