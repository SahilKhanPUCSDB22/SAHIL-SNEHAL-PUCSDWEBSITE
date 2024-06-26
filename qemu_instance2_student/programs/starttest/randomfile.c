first = (char*)PQgetvalue(res,0,0); //takeout first question from list

params[1]=first; //parameterize
tparams[2]=first;

//get file question text

res1=PQexecParams(con,(char*)getfile,2,NULL,(const char* const*)params,NULL,NULL,0);

if( PQresultStatus(res1)!=PGRES_TUPLES_OK)
{
	logg(PQerrorMessage(con));
	logg("db fetch failure : fun test\n");
	return NULL;
}

wp("<div class=\"container\">\n");

wp(form); //create form

insertquestion("1",PQgetvalue(res1,0,0));

wp("<textarea rows=\"30\" cols=\"120\" disabled>"),wp(PQgetvalue(res1,0,1)),wp("</textarea><br>");

answer = getanswer(tparams,"M");

if( answer[0]=='F')
{
	logg("Failed to create page\n");
	return NULL;
}

if(answer[0]!='E')
{
	wp(label),wp("File Already Uploaded : "),wp(answer),wp(clabel),wp(br);
}

wp(label),wp("Select your File : "),wp(clabel);

wp("<input type=\"file\" id=\"sfile\" name=\"file\"><br>");

insertformbutton();
wp(cform);

wp("</div>\n");

insertbuttons();


qlist[i]=res;
nq+=PQntuples(res); //no of q
mtype[i++]="F";

wp("<br><div class=\"grid\">\n");

creategrid(res,"FILE QUESTIONS",tqno);

wp("</div>\n");

createqlist(qlist,mtype,i);
insertnextprev(nq);

