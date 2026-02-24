/*
 * receiver.c
 *
 *  Created on: Feb 20, 2026
 *      Author: bodiec
 */

#include <stdint.h>
#include <stdbool.h>
void decodeMessage(uint16_t *times, uint16_t length, char*dest){
	// decodes manchester encoding to binary
	uint8_t last_bit = 0;
	bool is_at_center = true;
	// if the bit transitoin period is long
	// then that means the next bit has changed
	// if the transistion is short there was no change to the bit
	for(int i = 0; i < length; i++){
		if (times[i] > 750){
			dest[i+1] = '1';
			last_bit = 0;
		} else {
			dest[i+1] = '0';
			last_bit = 0;
		}
	}
	dest[length] = '\0';
}


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
	int bit_position = 0;
	for(int char_index = 0; char_index < length; char_index++){
		uint8_t value = 0;
		for(int b = 0; b < 8; b++){
			value <<=1;
			if(message[bit_position] == '1'){
				value |= 1;
			}
		}
		dest[char_index] = (char)value;
	}
	dest[length] = '\0';  // null-terminate
}
