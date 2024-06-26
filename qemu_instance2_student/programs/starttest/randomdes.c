first = (char*)PQgetvalue(res,0,0); //takeout first question from list

params[1]=first; //parameterize
tparams[2]=first;

//get mcq question text
res1=PQexecParams(con,(char*)getdes,2,NULL,(const char* const*)params,NULL,NULL,0);

if( PQresultStatus(res1)!=PGRES_TUPLES_OK)
{
	logg(PQerrorMessage(con));
	logg("db fetch failure : fun test\n");
	return NULL;
}


wp("<div class=\"container\">");
wp(form); //create form

insertquestion("1",PQgetvalue(res1,0,0));

answer=getanswer(tparams,"M");
if( answer[0]=='F')
{
	logg("Failed to create page\n");
	return NULL;
}

insertanswerbox(res1,answer);

insertformbutton();

wp(cform);

wp("</div>\n");
insertbuttons();

qlist[i]=res;
nq+=PQntuples(res); //no of q
mtype[i++]="D";

wp("<br><div class=\"grid\">\n");

creategrid(res,"DESCRIPTIVE QUESTIONS",tqno);

res=PQexecParams(con,(char*)filerand,1,NULL,(const char* const*)params,NULL,NULL,0);

if( PQresultStatus(res)!=PGRES_TUPLES_OK)
{
	logg(PQerrorMessage(con));
	logg("db fetch failure : fun test\n");
	return NULL;
}

if(PQntuples(res)>0) //if file question present
{
	creategrid(res,"FILE QUESTIONS",tqno);
	qlist[i]=res;
	mtype[i++]="F";
	nq+=PQntuples(res); //no of q
}

wp("</div>\n");

createqlist(qlist,mtype,i);
insertnextprev(nq);

