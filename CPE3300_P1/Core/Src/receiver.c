/*
 * receiver.c
 *
 *  Created on: Feb 20, 2026
 *      Author: bodiec
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void decodeMessage(char *message, char *dest){
	uint8_t length = 0;
	uint8_t bit_index = 0;
	for(int i = 8; i < 16;i++){
		if(message[i] == '1'){
			length |= (1 << (8 - bit_index++));
		}
	}

	bit_index = 0;

	// loop through each message
	// convert it to a int and then to a character
		dest[bit_index] =

//	printf("%d\n", length);
}


