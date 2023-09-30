/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 *
 * Purpose:
 * 		This file provides size parameters for a packet sent/received over UART as well as
 * 	helper functions to parse and format these packets.  A packet is simply a fixed-length
 * 	array of bytes (or characters) composed of header segment followed by a payload segment.
 * 		Variable-length strings for the are not supported.  Character arrays passed into the
 * 	packet composition function need to be the same length as the header and payload segments,
 * 	as this function does not null-terminate.
 */

#ifndef INC_UART_PACKET_HELPERS_H_
#define INC_UART_PACKET_HELPERS_H_


#include <stdint.h>


/*
 * Size parameters for packets.
 */
#define UART_PACKET_SIZE 64
#define UART_PACKET_HEADER_SIZE 4
#define UART_PACKET_PAYLOAD_SIZE (UART_PACKET_SIZE - UART_PACKET_HEADER_SIZE)

/*
 * A SerialMessage is made up of a header and a body. The header represents
 * a type for the message, that is, the command type or response type, and
 * the body contains the information that is necessary for the message type.
 */
typedef struct {
	uint8_t header[UART_PACKET_HEADER_SIZE];
	uint8_t body[UART_PACKET_PAYLOAD_SIZE];
} SerialMessage;

/* composePacket
 *
 * Function:
 * 	formats separate header and payload character arrays into one character array
 * 	of correct format for transmission over UART.
 *
 * Parameters:
 * 	packet_buffer - byte buffer pointer to store output.
 * 	header - byte buffer pointer to copy the header segment from.
 * 	payload - byte buffer pointer to copy the payload segment from.
 *
 * Return:  (by parameter)
 * 	packet_buffer - formatted packet byte array.
 *
 * Note:
 * 	does not null terminate on input.
 */
void composePacket(uint8_t packet_buffer[UART_PACKET_SIZE], const uint8_t header_buffer[UART_PACKET_HEADER_SIZE],
		const uint8_t payload_buffer[UART_PACKET_PAYLOAD_SIZE]);

/* Function:
 * 	parses header and payload segments of packet into their own separate character
 * 	arrays.
 *
 * Parameters:
 * 	header - byte buffer pointer to copy the header segment to.
 * 	payload - byte buffer pointer to copy the payload segment to.
 * 	packet_buffer - byte buffer pointer to copy from.
 *
 * Return:  (by parameter)
 * 	header - header segment byte buffer.
 * 	payload - payload segment byte buffer.
 *
 * Note:
 * 	does not null terminate on input.
 */
void decomposePacket(uint8_t header_buffer[UART_PACKET_HEADER_SIZE], uint8_t payload_buffer[UART_PACKET_PAYLOAD_SIZE],
		const uint8_t packet_buffer[UART_PACKET_SIZE]);


#endif /* INC_UART_PACKET_HELPERS_H_ */
