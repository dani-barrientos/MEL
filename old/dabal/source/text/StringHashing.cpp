#include <text/StringHashing.h>

#include <string.h>

int text::HashString(const char *str)
{
	int hash=0;
	const char* caracter = str;
	while ( *caracter != 0 )
	{
		hash=(hash<<5)-hash+*caracter++;		
	}
	return hash;
}

int text::HashString(const string &str)
{
	int hash=0;
	for (unsigned int i=0; i <str.length(); i++) 
	{
		hash=(hash<<5)-hash+str[i];
	}
	return hash;
}