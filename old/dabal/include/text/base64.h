#include <DabalLibType.h>

#include <string>
using std::string;

namespace text {
	/**
	* Encode the given buffer in base-64
	* @param[in] bytes_to_encode the data chunk to be encoded
	* @param[in] in_len the lneght of the dat to be encoded
	* @param[out] result the string where the data will be appended (you might consider clearing it first if you plan
	* to reuse it)
	*/
	DABAL_API void base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len, string& result);

	/**
	* Decode a base-64 string into a string
	* @param[in] s the encoded string
	* @param[out] result the string to store the result into
	* @return the actual size of the decoded data. It will be 0 if the buffer size is not large enough
	*/
	DABAL_API uint32_t base64_decode(const char* s, string& result);
	/**
	* Decode a base-64 string into an preallocaded buffer
	* @param[in] s encoded string
	* @param[out] result preallocated buffer to store the result into
	* @param[in] size the size of the result buffer, that must be at least strlen(s)*3/4
	* @return the actual size of the decoded data. It will be 0 if the buffer size is not large enough
	*/
	DABAL_API uint32_t base64_decode(const char* const s, char* const result, size_t size );

	/**
	* Decode a base-64 string
	* @param[in] s encoded string
	* @param[out] length the variable receiving the actual size of the decoded data
	* @return the decoded buffer, whose onwership is passed to the caller
	*/
	DABAL_API unsigned char* base64_decode(const char* s, int &length);
}