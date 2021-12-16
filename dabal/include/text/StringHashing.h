#include <FoundationLibType.h>

#include <string>
using std::string;

/**
* @namespace text
* utilities on strings
*/
namespace text
{
	/**
	* Generates a hash code for the string provided
	* @param str string to be hashed
	* @return hash code
	*/
	int FOUNDATION_API HashString(const char *str);
	/**
	* Generates a hash code for the string provided
	* @param str string to be hashed
	* @return hash code
	*/
	int FOUNDATION_API HashString(const string &str);
}
