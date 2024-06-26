void creategrid(PGresult *res , char* message , char* tqno)
{
	int rows = PQntuples(res),i=0;

	char type[]={message[0],'\0'};

	wp("<br><div class=\"grid\">\n");
	wp(ptag),wp(message),wp(cptag),wp(ptag);

	while(i<rows)
	{
		wp("<a href=\"#\" onclick=\"loadq("),wp(PQgetvalue(res,i,0)),wp(" , "),wp(tqno),wp(" ,'"),wp(type),wp("' ); return false;\">"),wp(tqno),wp("</a>\n");

		if(i!=(rows-1))
		{
			wp(" , ");
		}

		myitoa( (myatoi(tqno)+1) , tqno);
		i++;
	}

	wp(cptag);
	wp("</div>\n");
}

void createqlist(PGresult *res[] , char *type[] ,int n)
{ 
	wp("\n<script>\n"),wp("var qlist = [");

	int i=0,j=0,rows;
	while(i<n)
	{
		rows=PQntuples(res[i]);
		j=0;
		while(j<rows)
		{	
			wp("[");
			wp(PQgetvalue(res[i],j,0));
			wp(",'");
			wp(type[i]);
			wp("']");
			if(j!=(rows-1) || i!=(n-1))
			{
				wp(" , ");
			}
			else
			{
				wp("];\n");
			}
			j++;
		}
		i++;
	}
	wp("</script>\n");
}

void insertbuttons()
{
	wp("<br><div id=\"lrsbut\" class=\"butcontainer\">");
	wp("<button class=\"clearbutton\" onclick=\"clearanswer()\" >Clear Answer</button><br>");
	wp("</div>");
}

void insertnextprev(int i)
{
	if(i>1)
	{       wp("<script>\n"); 
		wp("var right=");

		wp("'<button onclick=\"loadnext()\" class=\"rightbutton\" id=\"nbut\">Next</button>';");

		wp("var left=");

		wp("'<button onclick=\"loadprevious()\" id=\"pbut\" class=\"leftbutton\" style=\"display :none\">Previous</button>';");

		wp("\n var butdiv = document.getElementById('lrsbut');\n var leftdiv,rightdiv;\n");

		wp("leftdiv = document.createElement('div'),rightdiv=document.createElement('div');\n");

		wp("leftdiv.innerHTML=left,rightdiv.innerHTML=right;\n");

		wp("butdiv.appendChild(leftdiv),butdiv.appendChild(rightdiv);");

		wp("\n</script>\n");

		wp("<button id=\"subbutton\"  onclick=\"submittest()\" class=\"submitbutton\" style=\"display:none\">Submit Test</button>\n");

	}
	else
	{
		wp("<button id=\"subbutton\" onclick=\"submittest()\" class=\"submitbutton\" >Submit Test</button>\n");
	}

}

char* test (char* testid , char * studid)
{
	char * filename = mycat( floc , mycat(studid,testid) ) ;

	tp = fopen (  filename , "w+");

	if(tp==NULL)
	{
		logg("Failed to open file for index\n");
		return NULL;
	}

	char *buffer , * params[2] , *tparams[3] , *answer;

	params[0]= testid,tparams[0]=studid,tparams[1]=testid;


	res=PQexecParams(con,(char*)chkrndm,1,NULL,(const char* const*)params,NULL,NULL,0);

	if( PQresultStatus(res)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		logg("db fetch failure : fun test\n");
		return NULL;
	}

	char* random = PQgetvalue(res,0,0),*first,*tqno = malloc(sizeof(char)*CAPACITY),*mtype[types];

	tqno[0]='1',tqno[1]=0;

	int i=0,t=0,nq=0;

	PGresult *res1,*qlist[types];

	if(random[0]=='Y')
	{
		res=PQexecParams(con,(char*)mcqrand,1,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun test\n");
			return NULL;
		}

		if(PQntuples(res)>0) //if mcq question present
		{
			#include"randommcq.c"
		}
		else
		{
			res=PQexecParams(con,(char*)desrand,1,NULL,(const char* const*)params,NULL,NULL,0);

			if( PQresultStatus(res)!=PGRES_TUPLES_OK)
			{
				logg(PQerrorMessage(con));
				logg("db fetch failure : fun test\n");
				return NULL;
			}

			if(PQntuples(res)>0) //if des question present
			{
				#include"randomdes.c"
			}
			else  //filefirstpage
			{
				res=PQexecParams(con,(char*)filerand,1,NULL,(const char* const*)params,NULL,NULL,0);

				if( PQresultStatus(res)!=PGRES_TUPLES_OK)
				{
					logg(PQerrorMessage(con));
					logg("db fetch failure : fun test\n");
					return NULL;
				}

				if(PQntuples(res)>0) //if des question present
				{
					#include"randomfile.c"	
				}
				else
				{
					wp("No Questions Available.Contact Admin");
				}
			}
		}
	}
	else
	{
		res=PQexecParams(con,(char*)mcq,1,NULL,(const char* const*)params,NULL,NULL,0);

		if( PQresultStatus(res)!=PGRES_TUPLES_OK)
		{
			logg(PQerrorMessage(con));
			logg("db fetch failure : fun test\n");
			return NULL;
		}

		if(PQntuples(res)>0) //if mcq question present
		{
			#include"normalmcq.c"
		}
		else
		{
			res=PQexecParams(con,(char*)des,1,NULL,(const char* const*)params,NULL,NULL,0);

			if( PQresultStatus(res)!=PGRES_TUPLES_OK)
			{
				logg(PQerrorMessage(con));
				logg("db fetch failure : fun test\n");
				return NULL;
			}
			if(PQntuples(res)>0)
			{
				#include"normaldes.c"
			}
			
			else
			{
				res=PQexecParams(con,(char*)fileques,1,NULL,(const char* const*)params,NULL,NULL,0);

				if( PQresultStatus(res)!=PGRES_TUPLES_OK)
				{
					logg(PQerrorMessage(con));
					logg("db fetch failure : fun test\n");
					return NULL;
				}
				if(PQntuples(res)>0)
				{	
					#include"normalfile.c"
				}
				else
				{
					wp("No Questions Available.Please Contact Administrator.\n");
				}
			}		
		}
	}

	buffer = malloc(sizeof(char)*total);

	if ( getpage(buffer,filename) )
	{
		return NULL;
	}

	return buffer;
}
