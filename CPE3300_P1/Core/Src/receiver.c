/*
 * receiver.c
 *
 *  Created on: Feb 20, 2026
 *      Author: bodiec
 */

#include <stdint.h>
#include <stdbool.h>
void decodeMessage(uint16_t *times, uint16_t length, char*dest){
	uint8_t last_bit = 0;
	bool is_at_center = true;
	// if the bit transitoin period is long
	// then that means the next bit has changed
	// if the transistion is short there was no change to hte bit
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

