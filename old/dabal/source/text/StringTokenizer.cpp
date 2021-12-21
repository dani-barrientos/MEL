#include <text/StringTokenizer.h>
using text::StringTokenizer;

#include <string>
#include <string.h>
using std::string;

#include <list>
using std::list;

#include <text/StringUtil.h>

StringTokenizer::StringTokenizer(const char *src, char separator, bool allowEmptyTokens)
{
	mTokens=getTokenArray(src,separator,mTokenCount);
	if (!allowEmptyTokens) cleanTokens();
}

StringTokenizer::~StringTokenizer()
{
	for (int i=0; i<mTokenCount; i++)
	{
		delete [] mTokens[i];
	}

	delete [] mTokens;
}

void StringTokenizer::cleanTokens()
{
	list<string> tokens;
	for (int i=0; i<mTokenCount; i++)
	{
		if (strlen(mTokens[i])>0)
		{
			tokens.push_back(mTokens[i]);
		}
	}
	for (int i=0; i<mTokenCount; i++)
	{
		delete [] mTokens[i];
	}
	delete [] mTokens;

	mTokenCount=(unsigned int)tokens.size();
	mTokens=new char *[mTokenCount];
	int counter=0;
	for (list<string>::iterator it=tokens.begin(); it!=tokens.end(); it++)
	{
		mTokens[counter++]= text::StringUtil::duplicate(it->c_str());
	}
}

int StringTokenizer::getTokenCount(const char *str,char separator, int *maxTokenlength)
{
	if (maxTokenlength!=NULL) *maxTokenlength=0;
	if (str==NULL) return 0;
	int count=0;
	char ch;
	int tokenlength=0;
	for (unsigned int i=0; i<strlen(str)+1; i++)
	{
		ch=str[i];
		if (ch==separator) 
		{
			if (maxTokenlength!=NULL && *maxTokenlength<tokenlength) 
			{
				*maxTokenlength=tokenlength;
			}
			count++;
		}
		else if (ch==0) 
		{
			if (maxTokenlength!=NULL && *maxTokenlength<tokenlength) 
			{
				*maxTokenlength=tokenlength;
			}
			return ++count;
		}
		else
		{
			tokenlength++;
		}
	}

	return count;
}

void StringTokenizer::getToken(const char *src, char *dst, char separator, int token)
{
	dst[0]=0;
	int tokenCount=0;
	int tokenPos=0;
	char ch;
	unsigned int i=0;
	while (true)
	{
		ch=src[i];
		if (ch==separator)
		{
			if (separator==13) i++;
			tokenCount++;

			if (tokenCount>token)
			{
				dst[tokenPos]=0;
				break;
			}
		}
		else if (ch==0)
		{
			dst[tokenPos]=0;
			break;
		}
		else
		{
			if (tokenCount==token) dst[tokenPos++]=ch;
		}
		i++;
	}
}

char** StringTokenizer::getTokenArray(const char *src, char separator, int &tokenCount)
{
	int maxTokenlength;
	tokenCount=getTokenCount(src,separator,&maxTokenlength);
	char *temp=new char[maxTokenlength+1];
	char **result=NULL;
	result=new char *[tokenCount];
	for (int i=0; i<tokenCount; i++)
	{
		getToken(src,temp,separator,i);
		result[i]= text::StringUtil::duplicate(temp);
	}
	delete [] temp;
	return result;
}