#pragma once

#include <DabalLibType.h>
#ifdef _ANDROID
//NULL is not defined at this point on Android
#include <stddef.h>
#endif

namespace text
{
	/**
	*Splits a string into tokens given a separator
	*/
	class DABAL_API StringTokenizer
	{
	public:
		/**
		*Constructs a new StringTokenizer
		*@param src source string
		*@param separator the separator character
		*@param allowEmptyTokens tokens `true` if empty tokens ("") are to be extracted; `false` otherwise
		*/
		StringTokenizer(const char *src,char separator, bool allowEmptyTokens=false);
		~StringTokenizer();
		/**
		*Gets the token at index
		*@param i token index
		*/
		inline const char *getToken(int i);
		/**
		*Gets token count
		*@return token count
		*/
		inline int getTokenCount();
	private:
		char **mTokens;
		int mTokenCount;
		void cleanTokens();
		static int getTokenCount(const char *str,char separator, int *maxtokenlength);
		static void getToken(const char *src, char *dst, char separator, int token);
		static char** getTokenArray(const char *src, char separator, int &tokencount);
	};

	const char *StringTokenizer::getToken(int i)
	{
		if (i<0 || i>=mTokenCount) return 0;
		return mTokens[i];
	}

	int StringTokenizer::getTokenCount()
	{
		return mTokenCount;
	}
}
