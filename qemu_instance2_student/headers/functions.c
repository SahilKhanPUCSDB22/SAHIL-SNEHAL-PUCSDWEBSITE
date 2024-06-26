int MAX_SEG(int x)
{
	int res = (x/SEG_SIZE);

	if((x%SEG_SIZE)>0)
	{
		res++;
	}

	return res;
}


void myitoa(int a,char *out)
{
	int i=0;
	
	if(a==0)
	{
		out[0]=48;
		out[1]=0;
	}
	else
	{
		while(a>0)
		{
			out[i]='0'+(a%10);
			a/=10;
			i++;
		}

		out[i]=0;
	}

	a=(strlen(out)-1);
	i=0;
	char c;
	while(i<=a)
	{
		c=out[i];
		out[i]=out[a];
		out[a]=c;
		i++;
		a--;
	}
}

int cal(char *c , int len)
{
	long re;
	if (len==0)
	{
		re=*(c)-48;
	}
	else 
	{
		re=10 * cal(c,(len-1));
	}
	return re;
}

int gnum(char *c , int l)
{
	long re;
	if(*(c)=='\0')
	{
		re=0;
	}
	else 
	{
		re=cal(c,(l-1))+gnum(c+1 ,(l-1)) ;
	}
	return re;
}

int myatoi(char *c)
{
	long l=strlen(c);
	return gnum(c,l);	
}

int mylen(char* c)
{
	if(c==NULL)
	{
		return -1;
	}

	int i=0;
	long r=0;

	while(*(c+i)!='\0')
	{
		r=r+1;
		i=i+1;
	}
	return r;
}

char * myjoin(char *a , char* b)
{
	int size=mylen(a)+mylen(b);
	char *temp=malloc(sizeof(char)*(size+2));
	int i=0,j=0;
	int len=mylen(a);
	while(*(b+j)!='\0'  || *(a+i)!='\0')
	{
		if(*(b+j)!='\0')
		{
		 	temp[len]=*(b+j);
			len=len+1;
			j=j+1;
		}
		if(*(a+i)!='\0')
		{
			temp[i]=*(a+i);
			i=i+1;
		}
	}
	temp[size]=10;
	temp[size+1]=11;
	return temp;
}

int mycpy(char *a , char*b)
{
	if(a==NULL || b==NULL)
	{
		return -1;
	}

	int i=0;
	while(b[i]!=0)
	{
		a[i]=b[i];
		i++;
	}
	a[i]=10;
	a[i+1]=0;

	return 1;
}

int myappend(char *a , char * b)
{
	if(a==NULL || b==NULL)
	{
		return 0;
	}

	int i=mylen(a);

	int j=0;

	while(b[j]!=0)
	{
		a[i]=b[j];
		i++;
		j++;
	}
	a[i]=10;
	a[i+1]=0;

	return 1;
}

char* mycat( char* a , char *b )
{
	if(a==NULL || b==NULL)
	{
		return NULL;
	}

	int len[]={ mylen(a) , mylen(b) };
	
	char *catstr = malloc(sizeof(char)*(len[0]+len[1]+1));

	int i=0,j=0,k=len[0];

	while(a[i]!=0 || b[j]!=0)
	{
		if(a[i]!=0)
		{
			catstr[i]=a[i];
			i++;
		}

		if(b[j]!=0)
		{
			catstr[k++]=b[j++];
		}
	}

	catstr[k]=0;
	return catstr;
}

int mycmp ( char *a , char* b)
{
	if(a==NULL || b==NULL)
	{
		return 0;
	}

	if(mylen(a)!=mylen(b))
	{
		return 0;
	}

	int i=0;

	while((a[i]==b[i]) && (a[i]))
	{
		i++;
	}

	if(a[i]==0)
	{
		return 1; //same
	}
	else
	{ 
		return 0; //different
	}
}
