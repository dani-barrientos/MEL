#pragma once
#include <FoundationLibType.h>

namespace text
{
	/**
	* intented for use in base64.cpp for as helper
	*/
	template <class StringType>
	uint32_t base64_decodehelper(const char* const encoded_string, StringType ret)
	{

		size_t in_len = strlen( encoded_string );
		int i = 0;
		int j = 0;
		int in_ = 0;
		unsigned char char_array_4[4], char_array_3[3];
		int pos=0;
		//	3*in_len/4==MAX_ENCODED_DATA_SIZE
		//unsigned char *ret=new unsigned char[3*in_len/4];
		
		while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
		{
			char_array_4[i++] = encoded_string[in_]; in_++;
			if (i ==4) 
			{
				for (i = 0; i <4; i++)
				{
					char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);
				}

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
				{
					ret[pos++]= char_array_3[i];
				}
				i = 0;
			}
		}

		if (i) 
		{
			for (j = i; j <4; j++)
			{
				char_array_4[j] = 0;
			}

			for (j = 0; j <4; j++)
			{
				char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) 
			{
				ret[pos++]= char_array_3[j];
			}
		}

		return pos;
	}
}