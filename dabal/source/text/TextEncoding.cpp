#include <text/TextEncoding.h>
#include <assert.h>
#include <string.h>
using namespace text;

int text::decodeUTF8(const char *encodedBuffer, unsigned int *outCharLength)
{
	const unsigned char *buf = (const unsigned char*)encodedBuffer;

	int value = 0;
	int length = -1;
	unsigned char byte = buf[0];
	if( (byte & 0x80) == 0 )
	{
		// This is the only byte
		if( outCharLength ) *outCharLength = 1;
		return byte;
	}
	else if( (byte & 0xE0) == 0xC0 )
	{
		// There is one more byte
		value = int(byte & 0x1F);
		length = 2;

		// The value at this moment must not be less than 2, because 
		// that should have been encoded with one byte only.
		if( value < 2 )
			length = -1;
	}
	else if( (byte & 0xF0) == 0xE0 )
	{
		// There are two more bytes
		value = int(byte & 0x0F);
		length = 3;
	}
	else if( (byte & 0xF8) == 0xF0 )
	{
		// There are three more bytes
		value = int(byte & 0x07);
		length = 4;
	}else
	{
		//arreglo DANI. No estoy 100% seguro de que sea valido..
		if( outCharLength ) *outCharLength = 1;
		return -1;
	}

	int n = 1;
	for( ; n < length; n++ )
	{
		byte = buf[n];
		if( (byte & 0xC0) == 0x80 )
			value = (value << 6) + int(byte & 0x3F);
		else 
			break;
	}

	if( n == length ) 
	{
		if( outCharLength) *outCharLength = (unsigned)length;
		return value;
	}

	// The byte sequence isn't a valid UTF-8 byte sequence.
	return -1;
}

int text::encodeUTF8(unsigned int value, char *outEncodedBuffer, unsigned int *outLength)
{
	unsigned char *buf = (unsigned char*)outEncodedBuffer;

	int length = -1;

	if( value <= 0x7F )
	{
		buf[0] = value;
		if( outLength ) *outLength = 1;
		return 1;
	}
	else if( value >= 0x80 && value <= 0x7FF )
	{
		// Encode it with 2 characters
		buf[0] = 0xC0 + (value >> 6);
		length = 2;
	}
	else if( (value >= 0x800 && value <= 0xD7FF) || (value >= 0xE000 && value <= 0xFFFF) )
	{
		// Note: Values 0xD800 to 0xDFFF are not valid unicode characters
		buf[0] = 0xE0 + (value >> 12);
		length = 3;
	}
	else if( value >= 0x10000 && value <= 0x10FFFF )
	{
		buf[0] = 0xF0 + (value >> 18);
		length = 4;
	}

	int n = length-1;
	for( ; n > 0; n-- )
	{
		buf[n] = 0x80 + (value & 0x3F);
		value >>= 6;
	}

	if( outLength ) *outLength = length;
	return length;
}

int text::encodeUTF8(const char *text,char *outBuffer)
{
	int i =0;
	int i2 =0;
	int lng = (unsigned int)strlen(text);
	while(i<lng)
	{
		unsigned int l;
		if (encodeUTF8((unsigned char)text[i],&outBuffer[i2],&l)==-1)
			return -1;
		i2+=l;
		i++;
	}
	outBuffer[i2]=0;
	return i2;
}

unsigned int text::getUTF8Length(const char *text)
{
	size_t len = strlen(text);
	int numChars = 0;
	for (size_t i=0;i<len;)
	{
		unsigned int cLength;
		decodeUTF8(&text[i],&cLength);
		numChars++;
		i+= cLength;
	}
	return numChars;
}

int text::getPreviousUTF8Index(const char *text,unsigned int curIndex,unsigned int *outLength)
{
	unsigned int maxSize = (curIndex>=4)?4:curIndex;
	for (unsigned int i=maxSize;i>=1;--i)
	{
		unsigned int outCLength;
		int value = decodeUTF8(&text[curIndex-i],&outCLength);
		if ((value!=-1)&&(outCLength==i))
		{
			*outLength = i;
			return curIndex-i;
		}
	}
	outLength = 0;
	return -1;
}

/*
string urlencode(const string &c)
{
string escaped="";
int max = c.length();
for(int i=0; i<max; i++)
{
if ( (48 <= c[i] && c[i] <= 57) ||//0-9
(65 <= c[i] && c[i] <= 90) ||//abc...xyz
(97 <= c[i] && c[i] <= 122) || //ABC...XYZ
(c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
)
{
escaped.append( &c[i], 1);
}
else
{
escaped.append("%");
escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
}
}
return escaped;
}

string char2hex( char dec )
{
char dig1 = (dec&0xF0)>>4;
char dig2 = (dec&0x0F);
if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
if ( 0<= dig2 && dig2<= 9) dig2+=48;
if (10<= dig2 && dig2<=15) dig2+=97-10;

string r;
r.append( &dig1, 1);
r.append( &dig2, 1);
return r;
}

*/
char* char2HexURL( char dec, char* txtOut ) {
	char dig1 = (dec&0xF0)>>4;
	char dig2 = (dec&0x0F);
	if ( 0<= dig1 && dig1<= 9) dig1 += 48;    //0->48 (0x30) in ASCII
	if (10<= dig1 && dig1<=15) dig1 += 65-10; //A->65 (0x41) in ASCII
	if ( 0<= dig2 && dig2<= 9) dig2 += 48;
	if (10<= dig2 && dig2<=15) dig2 += 65-10;

	txtOut[0]='%';
	txtOut[1]=dig1;
	txtOut[2]=dig2;
	txtOut[3]=0;
	return txtOut;
}

void text::encodeURL(const char* txt,string& txtOut) {	
	encodeURL(txt, 0xFFFFFFFF, txtOut);
}

void text::encodeURL(const char* txt, size_t charCount, string& txtOut) {
	char c;
	char cth[4];
	while (charCount && (c = *(txt++))) {
		if ((48 <= c && c <= 57) ||  //0-9
			(65 <= c && c <= 90) ||  //abc...xyz
			(97 <= c && c <= 122) || //ABC...XYZ
			(c=='-' || c=='_' || c=='.' || c == '~' || c=='/' || c == '!' || c == '*' || c == '(' || c == ')' || c == '\'') || c=='=') {
			txtOut.append(&c, 1);
		}
		else {
			txtOut.append(c == ' ' ? "%20" : char2HexURL(c, cth));
		}
		--charCount;
	}
}

void text::decodeURL(const char* txt, string& txtOut) {
	decodeURL(txt, 0xFFFFFFFF, txtOut);
}

static char fromHex(char* hex) {
	uint8_t c0 = hex[0];
	uint8_t c1 = hex[1];
	if (c0 >= 'A') c0 -= 'A' - 10;
	else if (c0 >= 'a') c0 -= 'a' - 10;
	else c0 -= '0';

	if (c1 >= 'A') c1 -= 'A' - 10;
	else if (c1 >= 'a') c1 -= 'a' - 10;
	else c1 -= '0';

	uint8_t c = (c0 << 4) | c1;
	return c;
}

void text::decodeURL(const char* txt, size_t charCount, string& txtOut) {
	char c;
	char hex[2];
	while (charCount && (c = *(txt++))) {
		if (c != '%') {
			txtOut += c;
		}
		else {
			hex[0] = c = *txt++;
			if (c == '%') {
				txtOut += c;
			}
			else if (c) {
				hex[1] = *txt++;
				txtOut += fromHex(hex);
			}
		}
		--charCount;
	}
}

bool text::isURLEncoded(const char* txt, size_t charCount) {
	char c;
	while ((c = *(txt++)) && (charCount--)) {
		if (c == '%') {
			return true;
		}
	}
	return false;
}

bool text::isURLEncoded(const char* txt) {
	char c;
	while ((c = *(txt++))) {
		if (c == '%') {
			return true;
		}
	}
	return false;
}

unsigned int text::getUTF8Index(const char* text,unsigned int n) {
	unsigned int byteCount=0;
	for (unsigned int charCount=0;charCount<n && *text;++charCount) {
		unsigned int lng;
		int utf = decodeUTF8(text,&lng);		
		if ((utf==-1)||(lng>4))
			break;
		byteCount+=lng;
		text+=lng;
	}
	return byteCount;
}

unsigned int text::getNextUTF8Index(const char* text,unsigned int* outLng) {
	unsigned int lng;
	int utf=decodeUTF8(text,&lng);
	if ((utf<0)||(lng>4)) lng=1;
	if (outLng)
		*outLng=lng;
	return lng;
}

unsigned int text::toUTF8(const wchar_t* src,std::string& out,bool append) {	
	int srcSize=(int)wcslen(src);
	return toUTF8(src,srcSize,out,append);
}

static unsigned int computeUTF8Length(const wchar_t* src,unsigned int lng) {
#if defined(_WINDOWS) || defined(_WINDOWSRT)
	unsigned int utf8Length = (unsigned int)WideCharToMultiByte(
		CP_UTF8,
		0,//WC_ERR_INVALID_CHARS
		src,
		static_cast<int>(lng),
		nullptr,
		0,
		nullptr,
		nullptr
	);
	return utf8Length;
#else
	//!@todo other platforms
	assert(false && "Not yet implemented for non-WindowsRT platforms");
	return 0;
#endif
}

unsigned int text::toUTF8(const wchar_t* src,unsigned int lng,std::string& out,bool append) {
	if (!append)
		out.clear();

	size_t oldLng=out.size(); 
	unsigned int utf8Lng=computeUTF8Length(src,lng);
	out.append(utf8Lng,'#');
	char* outBuffer=(char*)out.data();
	int c=toUTF8(src,lng,outBuffer+oldLng,utf8Lng);	
	return c;
}

unsigned int text::toUTF8(const wchar_t* src,unsigned int lng,char* out,unsigned int maxLng) {
#if defined (_WINDOWS) || defined(_WINDOWSRT)
	if (lng>maxLng) 
		lng=maxLng;

	unsigned int requiredBufferSize = computeUTF8Length(src,lng);
	if ((requiredBufferSize == 0) || (requiredBufferSize>maxLng))
		return 0;	
		
	int numBytesWritten = WideCharToMultiByte(
		CP_UTF8,
		0,//WC_ERR_INVALID_CHARS
		src,
		static_cast<int>(lng),
		const_cast<char *>(out),
		requiredBufferSize,
		nullptr,
		nullptr
		);

	return numBytesWritten;
#else
	//!@todo other platforms
	assert(false && "Not yet implemented for non-WindowsRT platforms");
	return 0;
#endif
}

unsigned int text::toUnicode(const char* src,std::wstring& out,bool append) {
	if (!append) out.clear();
	int srcSize=(int)strlen(src);
	return toUnicode(src,srcSize,out);
}

static unsigned int computeUnicodeLength(const char* src,unsigned int lng) {
#if defined(_WINDOWS) || defined(_WINDOWSRT)
	unsigned int unicodeLng = (unsigned int)MultiByteToWideChar(
		CP_UTF8,
		0,
		src,
		static_cast<int>(lng),
		nullptr,
		0	
	);
	return unicodeLng;
#else
	//!@todo other platforms
	assert(false && "Not yet implemented for non-WindowsRT platforms");
	return 0;
#endif
}

unsigned int text::toUnicode(const char* src,unsigned int lng,std::wstring& out,bool append) {
	if (!append)
		out.clear();
	size_t oldLng=out.size();
	unsigned int unicodeLng=computeUnicodeLength(src,lng);
	out.append(unicodeLng,'#');
	wchar_t* outBuffer=(wchar_t*)out.data();
	int c=toUnicode(src,lng,outBuffer+oldLng,unicodeLng);
	return c;
}

unsigned int text::toUnicode(const char* src,unsigned int lng,wchar_t* out,unsigned int maxLng) {
#if defined(_WINDOWS) || defined(_WINDOWSRT)
	unsigned int requiredBufferSize = computeUnicodeLength(src,lng);
	if ((requiredBufferSize == 0) || (requiredBufferSize>maxLng))
		return 0;	
		
	int numBytesWritten = MultiByteToWideChar(
		CP_UTF8,
		0,
		src,
		static_cast<int>(lng),
		out,
		requiredBufferSize
		);

	return numBytesWritten;
#else
	//!@todo
	assert(false && "Not yet implemented for non-windows platforms");
	return 0;
#endif
}

#ifdef _WINDOWSRT
unsigned int text::toUTF8(const Platform::String^ str,std::string& out,bool append) {
	return toUTF8(((Platform::String^)str)->Data(),out,append);
}

Platform::String^ text::toUnicode(const char* str) {
	if (!str)
		return nullptr;

	std::wstring tmp;
	return toUnicode(str,tmp)?
		ref new Platform::String(tmp.c_str()):
		nullptr;
}
Platform::String^ text::toUnicode(const string& str) {
	return toUnicode(str.c_str());
}

std::string& text::toUTF8String(const wchar_t* str,std::string& cstr) {
	toUTF8(str,cstr);
	return cstr;
}

std::string& text::toUTF8String(const Platform::String^ str,std::string& cstr) {
	toUTF8(((Platform::String^)str)->Data(),cstr);
	return cstr;
}
#endif
