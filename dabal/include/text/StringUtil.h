#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <DabalLibType.h>
#include <algorithm>
#include <ctype.h>
#include <stdint.h>

namespace text {
	using std::string;
	using std::vector;
	using std::pair;

	/**
	* Char* proxy class. 
	If the managed parameter in the constructor is true the class itself takes care of delete the char* parameter.
	If the managed parameter in the constructor is false the class will never delete the char* parameter.
	*/
	class DABAL_API CharPtrProxy
	{
	public:
		CharPtrProxy(const CharPtrProxy& original);
		CharPtrProxy(const char * value,bool managed);
		~CharPtrProxy(void);
		inline const char* getValue()const  {return mValue;}
		CharPtrProxy & operator=(const CharPtrProxy &original);		
	private:
		const char* mValue;
		mutable bool mManaged;

		inline void checkDelete(){ if (mManaged) delete[] mValue;}
	};

	/**
	 * Utility class with string manipulation methods.
	 */
	class DABAL_API StringUtil {
		public:
			/**
			* Copy n character from a string into another
			* @param[in] aDestination Destination string
			* @param[in] aSource Source string
			* @param[in] aSourceSize Size of source (0 if size is unknown or irrlevant)
			* @return the destination string
			**/
			static char* copy(char* aDestination, const char* aSource, int32_t aSourceSize);

			/**
			 * Copy n character from a string into another
			 * @param[in] aDestination Destination string
			 * @param[in] aSource Source string
			 * @return the destination string
			 **/
			static char* copy(char* aDestination, const char* aSource);

			/**
			 * Replace substrings in a string.
			 * Searches for a substring and replaces all its occurrences with a new substring.
			 * @param str the string to apply the replacement
			 * @param txt the substring to search for
			 * @param newTxt the new substring to replace the text for
			 */
			static void replaceAll(string& str,const char* txt,const char* newTxt);
			/**
			 * Replace substrings in a string.
			 * Searches for a substring and replaces all its occurrences with a new substring.
			 * @param str the string to apply the replacement
			 * @param txt the substring to search for
			 * @param newTxt the new substring to replace the text for
			 */
			inline static void replaceAll(string& str,const string& txt,const string& newTxt);
			/**
			 * Replace characters in a string.
			 * Searches for a character and replaces all its occurrences with a new one.
			 * @param str the string to apply the replacement
			 * @param old the character to search for
			 * @param nw the new character to replace it for
			 */
			static void replaceAll(char* str,const char old,const char nw);
			/**
			 * Remove substrings in a string.
			 * Searches for a substring and removes all its occurrences
			 * @param str the string to remove occurrences from
			 * @param txt the substring to remove
			 */
			static void removeAll(string& str,const char* txt);

			/**
			 * Converts all characters in a string to upper-case
			 * @param str the string to convert to uppercase
			 */
			inline static void toUpper(string &str);
			
			/**
			* Converts all characters in a string to lower-case
			* @param str the string to convert to lowercase
			*/
			inline static void toLower(string &str);

			/**
			 * Remove substrings in a string.
			 * Searches for a substring and removes all its occurrences
			 * @param str the string to remove occurrences from
			 * @param txt the substring to remove
			 */
			inline static void removeAll(string& str,const string& txt);
			/**
			 * Split a string into pieces.
			 * Scans the given strings and fills in the given vector with substrings delimited
			 * by the given delimiter characters.
			 * @param str the string to split
			 * @param tokens a NULL terminated array containing delimiter characters
			 * @param result the vector to insert the split parts into
			 * @param clear flag indicating if the vector should be cleared before split parts are added
			 * @param includeEmptyTokens If true, then empty tokens bewteen separatos are included. Example:
				with const char* cadena = "||hola|asdas||asd||", returns:"","","hola","asds","","asd","" 
			 */
			inline static void split(const string& str,const char* tokens,vector<string>& result,bool clear=true, bool includeEmptyTokens = false);
			/**
			* Split a string into pieces.
			* Scans the given strings and fills in the given vector with substrings delimited
			* by the given delimiter characters.
			* @param str the string to split
			* @param tokens a NULL terminated array containing delimiter characters
			* @param result the vector to insert the split parts into
			* @param clear flag indicating if the vector should be cleared before split parts are added
			* @param includeEmptyTokens if `true`, then empty tokens bewteen separatos are included. Example:
			* with const this flag enabled, a | separator and an input string like "||hola|asdas||asd||", the result vector will
			* contain ["","","hola","asds","","asd",""]
			*/
			static void split(const char* str,const char* tokens,vector<string>& result,bool clear=true, bool includeEmptyTokens = false);

			/**
			 * Compares two strings using case sEnsItiVe matching.
			 * It may seem stupid but the plain string comparison function may differ from platform to platform :P
			 * @param str1 the string to compare
			 * @param str2 the string to compare to
			 * @return 0 if strings are equal. <0 if str1 is less than str2. >0 if str1 is greater than str2
			 */
			inline static int32_t compare(const char* str1,const char* str2);

			/**
			* Compares two strings using case insensitive matching.
			* It may seem stupid but the plain string comparison function may differ from platform to platform :P
			* @param str1 the string to compare
			* @param str2 the string to compare to
			* @return 0 if strings are equal. <0 if str1 is less than str2. >0 if str1 is greater than str2
			*/
			inline static int32_t compareIgnoreCase(const char* str1,const char* str2);

			/**
			* Compares n characters two strings using case insensitive matching.
			* It may seem stupid but the plain string comparison function may differ from platform to platform :P
			* @param str1 the string to compare
			* @param str2 the string to compare to
			* @param[in] aN Number of characters to compare
			* @return 0 if strings are equal. <0 if str1 is less than str2. >0 if str1 is greater than str2
			*/
			inline static int32_t compareIgnoreCaseN(const char* str1, const char* str2, int32_t aN);

			/**
			* duplicate string
			* @param[in] str string to copy
			* @return new string copy of str
			*/
			inline static char* duplicate( const char* str );

			/**
			* parse string and convert to int
			*/
			static int32_t parseInt( const char* str );
			static int64_t parseInt64(const char* str);

			/**
			* parse string and convert to bool
			* @param[in] str string to convert
			* @return bool converted value.
			*/
			static bool parseBool(const char * str);

			/**
			* parse string and convert to float
			* @param[in] str string to convert
			* @return float converted value.
			*/
			static float parseFloat(const char* str);

			/**
			* parse string and convert to double
			* @param[in] str string to convert
			* @return float converted value.
			*/
			static double parseDouble(const char* str);

			/**
			* Converts an int to a string
			*/
			static string toString(int32_t aInt);
			static char* toString(int32_t aInt, char* buffer);

			/**
			* Converts an unsigned int to a string
			*/
			static string toString(uint32_t aInt);
			static char* toString(uint32_t aInt,char* buffer);

			/**
			* Converts a double to a string
			*/
			inline static string toString(double aDouble);
			inline static string toString(double aDouble,uint32_t numDecimals);
			inline static char* toString(double aDouble,char* buffer);
			inline static char* toString(double aDouble,uint32_t numDecimals,char* buffer);

			/**
			* Converts an float to a string
			*/
			static string toString(float aFloat);
			static string toString(float aFloat,uint32_t numDecimals);
			static char* toString(float aFloat,char* buffer);
			static char* toString(float aFloat,uint32_t numDecimals,char* buffer);

			/**
			* Converts an int64 to a string
			*/
			static string toString(uint64_t aQWord);
			static string toString(int64_t aQWord);
			static char* toString(uint64_t aInt,char* buffer);
			static char* toString(int64_t aInt, char* buffer);
		
			/**
			 * Search for substring in a string
			 * @param source the string to search into
			 * @param substr the substring to search for
			 * @return a pointer to the first occurrence of substr in source, or NULL if none found.
			 */
			inline static const char* contains(const char* source,const char* substr);
			/**
			 * Search for substring in a string ignoring case
			 * @param source the string to search into
			 * @param substr the substring to search for
			 * @return a pointer to the first occurrence of substr in source, or NULL if none found.
			 */
			 static const char* containsIgnoreCase(const char* source,const char* substr);
			/**
			* Encodes a string as XML friendly (with special characters encoded in a XML way)
			*/
			 static CharPtrProxy encondeXML(const char* inString);
			/**
			* Decodes a XML string (replaces special XML entities with the standard character)
			*/
			 static CharPtrProxy decodeXML(const char* inString);

			/**
			* Encodes a string, replacing certain characters with a hexadecimal escape sequence.
			* @param inString the string to be encoded
			* @return the encoded string.
			* @remarks This function encodes special characters. In addition, it encodes the following characters: , / ? : @ & = + $ #
			*/
			inline static char* encodeURIComponent(const char * inString);

			/**
			* Decodes a Uniform Resource Identifier (URI) component previously created by encodeURIComponent or by a similar routine.
			* @param inString the string to be decoded
			* @return the decoded string.
			*/
			inline static char* decodeURIComponent(const char * inString);

			/**
			 * Encodes a string, replacing certain characters with a hexadecimal escape sequence.
			 * @param inString the string to be encoded
			 * @return the encoded string.
			 * @remarks This function encodes special characters, except: , / ? : @ & = + $ #
			 */
			inline static char* encodeURI(const char * inString);

			/**
			 * Decodes a Uniform Resource Identifier (URI) previously created by encodeURI or by a similar routine.
			 * @param inString the string to be decoded
			 * @return the decoded string.
			 */
			inline static char* decodeURI(const char * inString);

		 private:

			 enum EncodingType
			 {
				 ET_URI,
				 ET_URI_COMPONENT
			 };

			static const char * mNotEncodeableChars;
			static char * encodeURIHelper(EncodingType type, const char * inString);
			static char * decodeURIHelper(EncodingType type, const char * inString);
			static void encodeChar(EncodingType type, unsigned char * utf8Char, uint32_t charSize, std::stringstream & out);
			static bool shouldEncode(EncodingType type, char c);


			 //pair: first is the entity, second the length of the entity
			 typedef const pair<const char*,uint32_t> EntityType;
			  //pair: first is the character, second the XML entity of the character
			 typedef const pair<const char*,EntityType> PairType;
			 static  PairType mXmlEntityReferences[];
			 /**
			 * XML entities
			 */
			 static const uint32_t mXmlEntityReferencesCount;
	};
	//inline
	void StringUtil::replaceAll(string& str,const string& txt,const string& newTxt) {
		replaceAll(str,txt.c_str(),newTxt.c_str());
	}
	void StringUtil::removeAll(string& str,const string& txt) {
		removeAll(str,txt.c_str());
	}
	void StringUtil::toUpper(string &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	}
	void StringUtil::toLower(string &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	}
	void StringUtil::split(const string& str,const char* tokens,vector<string>& result,bool clear,bool includeEmptyTokens) {
		split(str.c_str(),tokens,result,clear,includeEmptyTokens);
	}
	int32_t StringUtil::compare(const char *str1, const char *str2) {
		return strcmp(str1,str2);
	}
	inline char* StringUtil::duplicate( const char* str )
	{
		//strdup no es standard, aunque en MAC funciona
		return str?strdup( str ):NULL;
	}
	int32_t StringUtil::compareIgnoreCase(const char *str1, const char *str2) {
#ifdef _WINDOWS
		return _stricmp(str1,str2);
#elif defined(DABAL_POSIX)
		return strcasecmp(str1,str2);
#else
		return strcmp(str1,str2);
#endif
	}

	int32_t StringUtil::compareIgnoreCaseN(const char *str1, const char *str2, int32_t aN)
	{
#ifdef _WINDOWS
		return _strnicmp(str1, str2, aN);
#elif defined(_MACOSX) || defined(_IOS) || defined(_ANDROID)
		return strncasecmp(str1,str2,aN);
#else
		return strncmp(str1,str2,aN);
#endif
	}
	const char* StringUtil::contains(const char* source,const char* substr) {
		return strstr(source,substr);
	}

	string StringUtil::toString(double aDouble)
	{
		char c[32];	
		return string(toString(aDouble,c));
	}

	string StringUtil::toString(double aDouble,uint32_t numDecimals)
	{
		char c[32];
		return string(toString(aDouble,numDecimals,c));
	}

	char* StringUtil::toString(double aDouble,char* buffer)
	{
		sprintf(buffer,"%f",aDouble);
		return buffer;
	}

	char* StringUtil::toString(double aDouble,uint32_t numDecimals,char* buffer)
	{
		char f[8];
		sprintf(f,"%%.%df",numDecimals);	
		sprintf(buffer,f,aDouble);
		return buffer;
	}

	char* StringUtil::encodeURI(const char * inString)
	{
		return encodeURIHelper(StringUtil::ET_URI, inString);
	}

	char * StringUtil::encodeURIComponent(const char * inString)
	{
		return encodeURIHelper(StringUtil::ET_URI_COMPONENT, inString);
	}

	char * StringUtil::decodeURI(const char * inString)
	{
		return decodeURIHelper(StringUtil::ET_URI, inString);
	}

	char * StringUtil::decodeURIComponent(const char * inString)
	{
		return decodeURIHelper(StringUtil::ET_URI_COMPONENT, inString);
	}
}


/*

static bool parseVector3( const char* str, Vector3& result );

static bool parseVector2( const char* str, Vector2& result );
*/