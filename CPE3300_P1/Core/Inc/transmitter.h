/**
* File name: transmitter.h
* Brief: contains function calls related to the transmitter
*/

#ifndef __TRANSMITTER_H__
#define __TRANSMITTER_H__

#include <stdlib.h>
#include <stdint.h>

/**
* @brief converts input string to binary string
* Note: the returned string is malloced
*/
char *stringToBinary(const char *input);

/**
* @brief converts given binary string to be a manchester encoded string
* note: returned string is malloced
*/
char *binaryToManchester(const char *binary);

/**
* @brief converts decimal message length to mancester encoded string
* @param message_length the length of the message to send
* @param res buffer location for mancester string
*/
void lengthToString(uint16_t message_length, char *res);


#endif
