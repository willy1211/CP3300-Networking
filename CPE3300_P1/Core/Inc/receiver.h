/*
 * receiver.h
 *
 *  Created on: Feb 20, 2026
 *      Author: bodiec
 */

#ifndef INC_RECEIVER_H_
#define INC_RECEIVER_H_

void decodeMessage(uint16_t *times, uint16_t length, char*dest);
void decodeBinary(char *message, char *dest);

#endif /* INC_RECEIVER_H_ */
