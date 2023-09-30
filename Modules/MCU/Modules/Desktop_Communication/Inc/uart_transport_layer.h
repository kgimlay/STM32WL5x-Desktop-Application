/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 *
 * Purpose:
 *		Transport layer control of communication with the Desktop application.
 *	Performs transmission/reception of packets.  Makes use of the HAL for UART
 *	communication.  Structured to allow for future implementation of queuing
 *	multiple packets for transmission and multiple packets in reception (variable
 *	length messages broken into packets).
 */

#ifndef INC_UART_TRANSPORT_LAYER_H_
#define INC_UART_TRANSPORT_LAYER_H_


#include <stdbool.h>
#include <stdint.h>
#include <uart_packet_helpers.h>
#include "stm32wlxx_hal.h"


/*
 * Status returns for API calls to the UART Transport Layer.
 */
typedef enum {
	TRANSPORT_OKAY,
	TRANSPORT_TIMEOUT,
	TRANSPORT_ERROR,
	TRANSPORT_BUSY,
	TRANSPORT_TX_FULL,
	TRANSPORT_TX_EMPTY,
	TRANSPORT_RX_EMPTY,
	TRANSPORT_RX_FULL,
	TRANSPORT_NOT_INIT
} TransportStatus;

/* uartTransport_init
 *
 * Function:
 *	Initialize the transport layer before use.
 *
 * Parameters:
 *	huart - pointer to UART_HandleTypeDef (HAL) handle of the UART
 *			peripheral to be used.
 *
 * Return:
 * 	bool - returns false if the huart paramter is NULL or the UART
 * 	handle has not been initialized by HAL_UART_init.
 *
 * Note:
 * 	Will not re-inialize the layer if the layer has already been initialized.
 */
bool uartTransport_init(UART_HandleTypeDef* huart);

/* uartTransport_reset
 *
 * Function:
 * 	Resets the transport layer's state to that immediately after being initialized.
 *
 * Return:
 * 	bool - true if the layer has been initialized, false otherwise.
 */
bool uartTransport_reset(void);

/* uartTransport_deinit
 *
 * Return:
 * 	bool - true if the layer had been initialized (and is now deinitialized), false
 * 			otherwise.
 */
bool uartTransport_deinit(void);

/* uartTransport_enqueueTx
 *
 * Function:
 *	Buffers a packet for transmission.
 *
 * Parameters:
 *	header - byte array pointer to header for packet.
 *	payload - byte array pointer to payload of packet.
 *
 * Return:
 *	TransportStatus
 *		TRANSPORT_OKAY - buffering successful
 *		TRANSPORT_TX_FULL - tx queue full
 *		TRANSPORT_NOT_INIT - transport layer not initialized
 */
TransportStatus uartTransport_bufferTx(uint8_t header[UART_PACKET_HEADER_SIZE], uint8_t payload[UART_PACKET_PAYLOAD_SIZE]);

/* uartTransport_dequeueRx
 *
 * Function:
 *	Gets a packet received from reception from the buffer.
 *
 * Parameters:
 *	header - byte array pointer to array to copy header into.
 *	payload - byte array pointer to array to copy payload into.
 *
 * Return:
 *	TransportStatus
 *		TRANSPORT_OKAY - buffering successful
 *		TRANSPORT_RX_EMPTY - rx buffer empty
 *		TRANSPORT_NOT_INIT - transport layer not initialized
 *
 */
TransportStatus uartTransport_debufferRx(uint8_t header[UART_PACKET_HEADER_SIZE], uint8_t payload[UART_PACKET_PAYLOAD_SIZE]);

/* uartTransport_tx_polled
 *
 * Function:
 *	Perform transmission of buffered packet over UART.
 *
 * Parameters:
 *	timeout_ms - timeout for transmission, in milliseconds.
 *
 * Return:
 *	TransportStatus
 *		TRANSPORT_NOT_INIT - transport layer not initialized
 *		TRANSPORT_TX_EMPTY - tx buffer is empty
 *		TRANSPORT_BUSY - UART peripheral is busy and tx could
 *			not begin
 *		TRANSPORT_TIMEOUT - timeout on tx
 *		TRANSPORT_ERROR - error with message transmission,
 *			see note † in uart_transport_layer.c.
 *		TRANSPORT_OKAY - transmission successful.
 *
 * Note:
 *	If transmission is delayed or takes longer than the timeout, the timeout
 *	will stop transmission before transmission is complete.
 */
TransportStatus uartTransport_tx_polled(uint32_t timeout_ms);

/* uartTransport_rx_polled
 *
 * Function:
 *	Perform reception of packet(s) over UART and buffers them.
 *
 * Parameters:
 *	timeout_ms - timeout for transmission, in milliseconds.
 *
 * Return:
 *	TransportStatus
 *		TRANSPORT_NOT_INIT - transport layer not initialized
 *		TRANSPORT_RX_FULL - rx buffer is full
 *		TRANSPORT_BUSY - UART peripheral is busy and rx could
 *			not begin
 *		TRANSPORT_TIMEOUT - timeout on rx
 *		TRANSPORT_ERROR - error with message transmission,
 *			see note † in uart_transport_layer.c.
 *		TRANSPORT_OKAY - reception successful.
 *
 * Note:
 *	If reception is delayed or takes longer than the timeout, the timeout will
 *	stop reception before reception is complete.
 */
TransportStatus uartTransport_rx_polled(uint32_t timeout_ms);


#endif /* INC_UART_TRANSPORT_LAYER_H_ */
