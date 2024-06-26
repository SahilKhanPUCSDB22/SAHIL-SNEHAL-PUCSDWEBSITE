#define floc "/root/website/files/testfiles/"

//normal
#define mcq "select qno from mquestion where testid=$1 order by qno"

#define ops "select ono,optext from options where testid=$1 and qno=$2 order by ono"

#define des "select qno from dquestion where testid=$1 order by qno"

#define fileques "select qno from fquestion where testid=$1 order by qno"

//rand
#define mcqrand "select qno from mquestion where testid=$1 order by  random()"

#define opsrand "select ono,optext from options where testid=$1 and qno=$2 order by random()"

#define desrand "select qno from dquestion where testid=$1 order by random()"

#define filerand "select qno from fquestion where testid=$1 order by random()"

//get qdata

#define getmcq "select ques from mquestion where testid=$1 and qno=$2"

#define getdes "select ques,charlimit from dquestion where testid=$1 and qno=$2"

#define getfile " select ques,fcontent from fquestion where testid=$1 and qno=$2"

//check for random

#define chkrndm "select randomize from test where testid=$1"

//answers

#define getmcqanswer " select answer from manswer where studid=$1 and testid=$2 and qno=$3 "
#define getdesanswer " select answer from danswer where studid=$1 and testid=$2 and qno=$3 "
#define getfileanswer " select fname from fname where studid=$1 and testid=$2 and qno=$3 "

//html tags

#define ptag "<p>"
#define cptag "</p>\n"
#define br "<br>\n"
#define div "<div>"
#define form "<form id=\"qform\">"
#define cform "</form>\n"
#define label "<label id=\"qlabel\">"
#define clabel "</label>\n"

FILE* tp;

long total;

void wp(char* c)
{
	total += fwrite((void*) c , sizeof(char) , mylen(c) , tp);
}

void insertoptions(PGresult * res1 , char* answer)
{
	char *opid,*tono = malloc(sizeof(char)*CAPACITY);
	
	tono[0]='1';
	tono[1]=0;

	int i=0,rows=PQntuples(res1);
	while(i<rows)
	{
		opid=PQgetvalue(res1,i,0);
		
		wp("<input type=\"radio\" name=\"qanswer\" id=\""),wp(opid),wp("\" value=\""),wp(opid),wp("\"") ;
		
		if( mycmp(opid,answer)==1 )
		{
			 wp(" checked");

		}
		
		wp(">"),wp("\n");

		wp("<label for=\""),wp(opid),wp("\">"),wp(tono),wp(" . "),wp(PQgetvalue(res1,i,1)),wp("</label>"),wp(br);	
		myitoa((i+2) , tono);
		i++;
	}
}

void insertquestion(char* qlabel , char* ques)
{
	wp(label) , wp(qlabel), wp(clabel), wp(" . ") ,wp(label), wp(ques) ,wp(clabel),wp(br),wp(br);;
}

void insertformbutton()
{
	wp("<br><button type=\"submit\" class=\"answerbutton\">Submit answer</button>\n");
}

void insertanswerbox(PGresult* res1 , char * answer)
{	
	wp("<textarea rows=\"20\" cols=\"100\" id=\"danswer\" placeholder=\"Enter your answer here\" maxlength=\""),wp(PQgetvalue(res1,0,1)), wp ("\">");
	
	if( answer[0] != 'E')
	{
		wp(answer);
	}
 	wp("</textarea><br>\n");
}

void addquestion(char *qno)
{
	wp("<input type=\"hidden\" id=\"qid\" value=\""),wp(qno),wp("\">");
}

char * getanswer ( char ** params , char *t)
{
	char * command;
	if(t[0]=='M')
	{
		command = getmcqanswer;
	}
	if(t[0]=='D')
	{
		command = getdesanswer;
	}
	if(t[0]=='F')
	{
		command= getfileanswer;
	}

	PGresult* r = PQexecParams( con , command , 3 , NULL , (const char* const*)params , NULL,NULL,0);
	
	if(PQresultStatus(r)!=PGRES_TUPLES_OK)
	{
		logg(PQerrorMessage(con));
		return "F";
	}

	if(PQntuples(r)>0)
	{
		return (char*)PQgetvalue(r,0,0);
	}
	else
	{
		return "E";
	}
}


int getpage(char * buffer , char *filename)
{
	fseek(tp,0,SEEK_SET);

	fread(buffer,sizeof(char),total,tp);

	if(ftruncate(fileno(tp),0)!=0)
	{
		logg("Failed to truncate the file\n");
		return 1;
	}

	fclose(tp);
	
	if(remove(filename)!=0)
	{
		logg("Failed to delete file\n");
		return 1;
	}
	
	return 0;
}
