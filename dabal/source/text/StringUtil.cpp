#include <text/StringUtil.h>
#include <inttypes.h>
#include <stdio.h>
#include <vector>
using std::vector;
using std::pair;

namespace text {


	const uint32_t StringUtil::mXmlEntityReferencesCount=6;

	//Unescaped characters: -_.!~*'()
	//Reserved characters: ; , / ? : @ & = + $
	//Score: #
	const char * StringUtil::mNotEncodeableChars = {"-_.!~*'();,/?:@&=+$#"};

	
	StringUtil::PairType StringUtil::mXmlEntityReferences[] = { PairType("<",EntityType("&lt;",4)),PairType("&",EntityType("&amp;",5)),PairType("\"",EntityType("&quot;",6)),/*PairType(" ",EntityType("&nbsp;",6)),*/PairType(">",EntityType("&gt;",4)),PairType("'",EntityType("&apos;",6)),/*Next one is the same as the prev. Inserting in this way we get &apos; as preferred simbol for "'"*/PairType("'",EntityType("&#039;",6))};


char* StringUtil::copy(char* aDestination, const char* aSource, int32_t aSourceSize)
	{
#ifdef _WINDOWS
    #ifdef _MSVC
		strcpy_s(aDestination, aSourceSize, aSource);
    #elif defined _GCC
        return strcpy(aDestination, aSource);
    #endif
		return aDestination;
#elif defined(DABAL_LINUX) || defined(DABAL_ANDROID) || defined(DABAL_IOS) || defined (DABAL_MACOSX)
		return strcpy(aDestination, aSource);
#endif
	}


char* StringUtil::copy(char* aDestination, const char* aSource)	{
	return strcpy(aDestination, aSource);
}

void StringUtil::replaceAll(string &str, const char *txt, const char *newTxt) {
	size_t pos = 0;
	uint32_t txtLng=(uint32_t)strlen(txt);
	uint32_t newTxtLng=(uint32_t)strlen(newTxt);
	while ((pos=str.find(txt,pos,txtLng))!=string::npos)
	{
		str.replace(pos,txtLng,newTxt,newTxtLng);
		pos = pos + newTxtLng;
	}
}

void StringUtil::removeAll(string& str,const char* txt) {
	size_t pos;
	uint32_t txtLng=(uint32_t)strlen(txt);
	while ((pos=str.find(txt,0,txtLng))!=string::npos)
		str.erase(pos,txtLng);
}
void StringUtil::split(const char* txt,const char* tokens,vector<string> &result,bool clear,bool includeEmptyTokens) 
{
	uint32_t lng=txt?(uint32_t)strlen(txt):0;
	const char* lastPos=txt;
	char c;
	if (clear)
		result.clear();
	while(lng--)
	{
		c = *txt;
		if(strchr(tokens,c))
		{
			if ( includeEmptyTokens || (txt-lastPos)>0 )
				//empty tokens area 
				result.push_back(string(lastPos,txt-lastPos));
			lastPos=txt+1;
		}
		else if(!lng)
		{
			result.push_back(string(lastPos,txt-lastPos+1));
		}

		txt++;
	}
}
/*
void StringUtil::split(const char* txt,const char* tokens,vector<string> &result,bool clear) 
{
	uint32_t lng=txt?(uint32_t)strlen(txt):0;
	const char* lastPos=txt;
	char c;
	if (clear)
		result.clear();
	while(lng--)
	{
		c = *txt;
		if(strchr(tokens,c))
		{
			if ((txt-lastPos)>0)
				result.push_back(string(lastPos,txt-lastPos));
			lastPos=txt+1;
		}
		else if(!lng)
		{
			result.push_back(string(lastPos,txt-lastPos+1));
		}

		txt++;
	}
}*/

void StringUtil::replaceAll(char *str, const char old, const char nw) {
	while (*str) {
		if (*str==old) *str=nw;
		str++;
	}
}

int32_t StringUtil::parseInt( const char* str )
{
	return atoi(str);
}
int64_t StringUtil::parseInt64(const char* str)
{
	int64_t result;
	sscanf(str, "%" SCNd64, &result);
	return result;
}

bool StringUtil::parseBool(const char* str)
{
	return (StringUtil::compareIgnoreCase(str, "true") == 0);
}

float StringUtil::parseFloat(const char* str)
{
	return (float)atof(str);
}

double StringUtil::parseDouble(const char* str)
{
	return atof (str);
}

string StringUtil::toString(int32_t aValue) {
	char c[16];
	return string(toString(aValue,c));
}

string StringUtil::toString(uint32_t aValue) {
	char c[16];
	return string(toString(aValue, c));
}

string StringUtil::toString(int64_t aValue) {
	char c[32];
	return string(toString(aValue,c));
}

string StringUtil::toString(uint64_t aValue) {
	char c[32];
	return string(toString(aValue, c));
}

string StringUtil::toString(float aValue) {
	char c[32];	
	return string(toString(aValue,c));
}

char* StringUtil::toString(float aValue,char* buffer) {	
	sprintf(buffer,"%f",aValue);
	return buffer;
}

string StringUtil::toString(float aValue,uint32_t numDecimals) {
	char c[32];
	return string(toString(aValue,numDecimals,c));
}

char* StringUtil::toString(float aValue,uint32_t numDecimals,char* buffer) {
	char f[8];
	sprintf(f,"%%.%df",numDecimals);	
	sprintf(buffer,f,aValue);
	return buffer;
}

char* StringUtil::toString(int32_t aValue,char* tmp) {
	sprintf(tmp,"%d",aValue);
	return tmp;
}

char* StringUtil::toString(uint32_t aValue,char* tmp) {
	sprintf(tmp,"%u",aValue);
	return tmp;
}

char* StringUtil::toString(uint64_t aValue,char* tmp) {
	sprintf(tmp,"%llu",aValue);
	return tmp;
}

char* StringUtil::toString(int64_t aValue, char* tmp) {
	sprintf(tmp, "%lld", aValue);
	return tmp;
}

const char* StringUtil::containsIgnoreCase(const char *source, const char *substr) {
	if (!substr || !source)
		return NULL;

	if ( !*substr )	{
		return source;
	}

	for ( ; *source; ++source ) {
		if ( toupper(*source) == toupper(*substr) ) {
			// Matched starting char -- loop through remaining chars.
			const char *h, *n;
			for ( h = source, n = substr; *h && *n; ++h, ++n ) {
				if ( toupper(*h) != toupper(*n) ) {
					break;
				}
			}
			if ( !*n ) { /* matched all of 'needle' to null termination */
				return source; /* return the start of the match */
			}

		}

	}
	return NULL;
}

char * StringUtil::encodeURIHelper(EncodingType type, const char * inString)
{
	std::stringstream out;
	out << std::hex;
    uint32_t i=0;
    uint32_t len=(uint32_t)strlen(inString);
    uint32_t charSize;
	unsigned char * byte = (unsigned char *)inString;

	while(i<len)
	{
		if(!(*byte & 0x80)) // 0xxxxxxx -> ASCII character
			charSize=1;
		else if ((*byte & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx -> 2 bytes
			charSize=2;		
		else if ((*byte & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx -> 3 bytes		
			charSize=3;		
		else /*if ((*byte & 0xF8) == 0xF0)*/ // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx -> 4 bytes
			charSize=4;

		encodeChar(type, byte, charSize, out);

		i+=charSize;
		byte+=charSize;
	}

	return StringUtil::duplicate(out.str().c_str());
}

char * StringUtil::decodeURIHelper(EncodingType type, const char * inString)
{
	std::stringstream out;
	uint32_t i=0, len=(uint32_t)strlen(inString);
	unsigned char * byte = (unsigned char *)inString;
	char byteStr[3];
	unsigned char utf8CharByte;

	byteStr[2]= '\0';
	while(i<len)
	{
		if (*byte!='%')
		{
			out << *byte;
			byte++;
			i++;
		}
		else
		{
			byteStr[0] = *(byte+1);
			byteStr[1] = *(byte+2);

			utf8CharByte = (unsigned char)strtoul(byteStr, NULL, 16);

			if(shouldEncode(type, utf8CharByte))
			{
				out << utf8CharByte;
			}
			else
			{
				out << *byte;
				out << byteStr;
			}
			i+=3;
			byte+=3;

		}
	}


	return StringUtil::duplicate(out.str().c_str());
}

bool StringUtil::shouldEncode(EncodingType type, char c)
{
	if (!((c>47 && c<58) ||(c>64 && c<91) || (c>96 && c<123)))
	{
		for(uint32_t i=0, len=type==ET_URI?20:9; i<len;++i)
		{
			if (mNotEncodeableChars[i] == c)
				return false;
		}

		return true;
	}

	return false;
}

void StringUtil::encodeChar(EncodingType type, unsigned char* utf8Char, uint32_t charSize, std::stringstream & out)
{
	if(charSize==1 && !shouldEncode(type,*utf8Char))
		out << *utf8Char;
	else
		for (uint32_t i=0; i<charSize; ++i)		
			out << "%" << (uint32_t)(*(utf8Char+i));
}


CharPtrProxy StringUtil::encondeXML(const char* inString)
{

	size_t originalLength=strlen(inString);
	size_t resultLength=originalLength;
	for (uint32_t i=0;i<originalLength;i++)
		for (uint32_t j=0;j<mXmlEntityReferencesCount;j++)
			if (inString[i]==mXmlEntityReferences[j].first[0])
			{
				resultLength+=mXmlEntityReferences[j].second.second;
				break;
			}
	if (resultLength==originalLength)//If same size we assume no change has been made
		return CharPtrProxy(inString,false);

	char* result=new char[resultLength+1];
	uint32_t lastReplaceAt=0;
	uint32_t resultPosition=0;
	for (uint32_t i=0;i<originalLength;i++)
		for (int32_t j=0;j<mXmlEntityReferencesCount;j++)
			if (inString[i]==mXmlEntityReferences[j].first[0])
			{
				memcpy(result+resultPosition,inString+lastReplaceAt,i-lastReplaceAt*sizeof(char));
				memcpy(result+resultPosition+i-lastReplaceAt,mXmlEntityReferences[j].second.first,mXmlEntityReferences[j].second.second*sizeof(char));
				//result.append(inString+lastReplaceAt,i-lastReplaceAt);
				//result.append(mXmlEntityReferences[j].second.first);
				resultPosition+=i-lastReplaceAt+mXmlEntityReferences[j].second.second;
				lastReplaceAt=i+1;
				break;
			}

	memcpy(result+resultPosition,inString+lastReplaceAt,originalLength-lastReplaceAt*sizeof(char));
	result[resultPosition+originalLength-lastReplaceAt]='\0';

	return CharPtrProxy(result,true);

}


CharPtrProxy StringUtil::decodeXML(const char* inString)
{
	size_t originalLength=strlen(inString);
	size_t referenceLenght;
	char* result=NULL;

	size_t lastReplaceAt=0;
	size_t resultPosition=0;
	for (uint32_t i=0;i<originalLength;i++)
		if (inString[i]=='&')
			for (uint32_t j=0;j<mXmlEntityReferencesCount;j++)
			{
				if (result==NULL)
				{
					result=new char[originalLength+1];
				}
				referenceLenght=mXmlEntityReferences[j].second.second;//quitar & de la comprobas
				if (i+referenceLenght<=originalLength && memcmp(inString+i+1,mXmlEntityReferences[j].second.first+1,sizeof(char)*(referenceLenght-1))==0)
				{
					memcpy(result+resultPosition,inString+lastReplaceAt,i-lastReplaceAt*sizeof(char));
					result[resultPosition+i-lastReplaceAt]=mXmlEntityReferences[j].first[0];//We assume we only have one char in entities struct
					
					//result.append(inString+lastReplaceAt,i-lastReplaceAt);
					//result.append(mXmlEntityReferences[j].first);
					resultPosition+=i-lastReplaceAt+1;
					lastReplaceAt=i+referenceLenght;
					break;
				}
			}
	if (result==NULL)
		return CharPtrProxy(inString,false);

	memcpy(result+resultPosition,inString+lastReplaceAt,(originalLength-lastReplaceAt)*sizeof(char));
	result[resultPosition+originalLength-lastReplaceAt]='\0';
	return CharPtrProxy(result,true);
}



/*bool StringUtil::parseVector3( const char* str, Vector3& r )
{
if ( !str || !strcmp( str , "") )
{
return false;
}else
{
sscanf(str, "%f%*[,; ]%f%*[,; ]%f", &r.x, &r.y, &r.z);
return true;
}

}

bool StringUtil::parseVector2( const char* str, Vector2& r )
{
if ( !str || !strcmp( str , "") )
{
return false;
}else
{
sscanf(str, "%f%*[,; ]%f", &r.x, &r.y);
return true;
}

}
*/



CharPtrProxy::CharPtrProxy(const CharPtrProxy& original)
{
	operator=(original);
}


CharPtrProxy::CharPtrProxy(const char * value,bool managed):mValue(value),mManaged(managed)
{
}

CharPtrProxy::~CharPtrProxy(void)
{
	checkDelete();
}


CharPtrProxy & CharPtrProxy::operator=(const CharPtrProxy &original)
{
	if ( this != &original )
	{
		checkDelete();
		mValue=original.mValue;
		mManaged = original.mManaged;
		original.mManaged=false;
	}
	return *this;
}



//end namespace
}
