#pragma once

#include <DabalLibType.h>
#include <string>
using std::string;

namespace text
{
	// This function will attempt to decode a UTF-8 encoded character in the buffer.
	// If the encoding is invalid, the function returns -1.
	int DABAL_API decodeUTF8(const char *encodedBuffer, unsigned int *outCharLength);
	int DABAL_API encodeUTF8(unsigned int value, char *outEncodedBuffer, unsigned int *outLength);
	int DABAL_API encodeUTF8(const char *text,char *outBuffer);
	// Checks the previous 'safe' character index in an UTF8 string 
	int DABAL_API getPreviousUTF8Index(const char *text,unsigned int curIndex,unsigned int *outLength);
	/**
	* URL encode the given text by appending the result into the supplied string
	* @param txt the source text to be url - encoded
	* @param txtOut the destination string to store the url - encoded text
	*/
	void DABAL_API encodeURL(const char* txt,string& txtOut);
	/**
	* URL encode the given text by appending the result into the supplied string
	* @param txt the source text to be url-encoded
	* @param the number of chars to be processed
	* @param txtOut the destination string to store the url-encoded text
	*/
	void DABAL_API encodeURL(const char* txt, size_t charCount, string& txtOut);
	/**
	* URL decode the given text
	* @param txt the source text to be url-decoded
	* @param txtOut the destination string to store the url-decoded text
	*/
	void DABAL_API decodeURL(const char* txt, string& txtOut);
	/**
	* URL decode the given text
	* @param txt the source text to be url-decoded
	* @param charCount the number of chars to be decoded
	* @param txtOut the destination string to store the url-decoded text
	*/
	void DABAL_API decodeURL(const char* txt, size_t charCount, string& txtOut);

	/**
	* Check if the given text is url-encoded
	* @param txt the text to be checked
	* @return `true` if the text contains url-encoded chars; `false` otherwise
	*/
	bool DABAL_API isURLEncoded(const char* txt);
	/**
	* Check if the given text is url-encoded
	* @param txt the text to be checked
	* @param charCount the number chars to checked in the input string
	* @return `true` if the text contains url-encoded chars; `false` otherwise
	*/
	bool DABAL_API isURLEncoded(const char* txt, size_t charCount);

	// Get the character count of an UTF8 string
	unsigned int DABAL_API getUTF8Length(const char *text);
	/*
	* Get the actual index of the byte corresponding to the nth character
	* @param text NULL terminated string with the text to search into
	* @param n the character number to get the byte index for
	* @return the byte index for the given nth character, that should be
	* at text+<index>. If text contains invalid UTF8 characters, then the
	* return value will correspond to the last well-formed code point index.
	*/
	unsigned int DABAL_API getUTF8Index(const char* text,unsigned int n);
	/**
	* Get the index of the byte corresponding to the next character
	* @param text NULL terminated string with the text to search into
	* @param lng optional pointer to receive the length of the next UTF8 char
	* @return the index from where the next _UTF8_ char is; if text contains an invalid _UTF8_ character, the returned
	* index will be 1
	*/
	unsigned int DABAL_API getNextUTF8Index(const char* text,unsigned int* lng=NULL);

	unsigned int DABAL_API toUTF8(const wchar_t* buffer,std::string& out,bool append=false);
	unsigned int DABAL_API toUTF8(const wchar_t* buffer,unsigned int lng,std::string& out,bool append=false);
	unsigned int DABAL_API toUTF8(const wchar_t* buffer,unsigned int lng,char* out,unsigned int maxLng);

	unsigned int DABAL_API toUnicode(const char* buffer,std::wstring& out,bool append=false);
	unsigned int DABAL_API toUnicode(const char* buffer,unsigned int lng,std::wstring& out,bool append=false);
	unsigned int DABAL_API toUnicode(const char* buffer,unsigned int lng,wchar_t* out,unsigned int maxLng);

#ifdef _WINDOWSRT
	unsigned int DABAL_API toUTF8(const Platform::String^ str,std::string& out,bool append=false);
	Platform::String^ DABAL_API toUnicode(const char* str);
	Platform::String^ DABAL_API toUnicode(const string& str);
	std::string& DABAL_API toUTF8String(const wchar_t* str,std::string& cstr);
	std::string& DABAL_API toUTF8String(const Platform::String^ str,std::string& cstr);
#endif
}