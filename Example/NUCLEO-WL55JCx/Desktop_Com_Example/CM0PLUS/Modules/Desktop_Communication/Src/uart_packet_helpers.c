/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 */


#include <uart_packet_helpers.h>
#include "string.h"


/* composePacket
 *
 * Simply acts as a wrapper for the memcpy function used to place header and payload
 * character arrays into packet buffer at correct locations.
 *
 * Copies UART_PACKET_HEADER_SIZE number of bytes to the packet_buffer, then copies
 * UART_PACKET_PAYLOAD_SIZE bytes to the packet buffer offset by UART_PACKET_HEADER_SIZE
 * number of bytes.
 */
void composePacket(uint8_t packet_buffer[UART_PACKET_SIZE], const uint8_t header[UART_PACKET_HEADER_SIZE],
		const uint8_t payload[UART_PACKET_PAYLOAD_SIZE])
{
	// Copy header into packet.
	memcpy(packet_buffer, header, UART_PACKET_HEADER_SIZE * sizeof(uint8_t));
	// Copy payload into packet.
	memcpy(packet_buffer + UART_PACKET_HEADER_SIZE, payload, UART_PACKET_PAYLOAD_SIZE * sizeof(uint8_t));
}


/* decomposePacket
 *
 * Simply acts as a wrapper for the memcpy function used to parse header and payload
 * character arrays from the packet buffer.
 *
 * Copies UART_PACKET_HEADER_SIZE number of bytes to the header_buffer and copies
 * UART_PACKET_PAYLOAD_SIZE number of bytes to the payload buffer.
 */
void decomposePacket(uint8_t header[UART_PACKET_HEADER_SIZE], uint8_t payload[UART_PACKET_PAYLOAD_SIZE],
		const uint8_t packet_buffer[UART_PACKET_SIZE])
{
	// Copy header from packet.
	memcpy(header, packet_buffer, UART_PACKET_HEADER_SIZE * sizeof(uint8_t));
	// Copy payload from packet.
	memcpy(payload, packet_buffer + UART_PACKET_HEADER_SIZE, UART_PACKET_PAYLOAD_SIZE * sizeof(uint8_t));
}
