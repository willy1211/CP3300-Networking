/*
 * receiver.c
 *
 *  Created on: Feb 20, 2026
 *      Author: bodiec
 */

#include <stdint.h>
#include <stdbool.h>

void decodeBinary(char *message, char *dest){
	uint8_t length = 0;

	// Length byte (bits 8-15)
	for(int i = 8; i < 16; i++){
		length <<= 1;           // shift left operator
		if(message[i]=='1'){
			length |= 1;        // OR operator
		}
	}

	// Extracting bytes
	int bit_position = 16;
	for(int char_index = 0; char_index < length; char_index++){
		uint8_t value = 0;
		for(int b = 0; b < 8; b++){
			value <<=1;
			if(message[bit_position] == '1'){
				value |= 1;
			}
			bit_position++;
		}
		dest[char_index] = (char)value;

	}
	dest[length] = '\0';  // null-terminate
}
