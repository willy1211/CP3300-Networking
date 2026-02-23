/*
* Filename transmitter.c
* Brief: contians code for the transmitter
*/


#include "transmitter.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

char *stringToBinary(const char *input){
	if(!input)
		return NULL;

	size_t len = strlen(input);

	//Each character = 8 bits
	char *binary = malloc(len*8 + 1); //allocate memory at runtime 8bit per character + 1 for Null character
	if(!binary)
		return NULL;
	char *out = binary;
	// Convert each character to its binary representation
	for (size_t i = 0; i < len; i++)
	{
		unsigned char c = input[i];
		for (int bit = 7; bit >= 0; bit--)
		{
			*out++ = ((c >> bit) & 1) ? '1' : '0';
		}
	}
	*out = '\0';
	return binary;
};

// Function to convert binary string to Manchester encoding 0->"01", 1->"10"
char *binaryToManchester(const char *binary){
	if (!binary)
		return NULL;

	size_t len = strlen(binary);

	// Each binary bit becomes 2 Manchester bits
	char *manchester = malloc(len * 2 + 1);
	if (!manchester)
		return NULL;

	char *m_out = manchester;

	for (size_t i = 0; i < len; i++)
	{
		if (binary[i] == '0')
		{
			*m_out++ = '1';
			*m_out++ = '0';
		}
		else if (binary[i] == '1')
		{
			*m_out++ = '0';
			*m_out++ = '1';
		}
	}

	*m_out = '\0';
	return manchester;

};


void lengthToString(uint16_t message_length, char *res){
    int idx = 0;

    for (int i = 7; i >= 0; i--) {
        if ((message_length & (1 << i)) >= 1) {
            // 1 → 01
            res[idx++] = '0';
            res[idx++] = '1';
        } else {
            // 0 → 10
            res[idx++] = '1';
            res[idx++] = '0';
        }
    }
    res[idx] = '\0';
};
