/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 */


#include <uart_transport_layer.h>
#include "string.h"


/*
 * Macro to check if a HAL uart handle has been initialized.
 *
 * Paremeters:
 * 	hal_uart_handle - pointer to a UART_HandleTypeDef.
 *
 * Return:
 * 	bool - true if initialized, false if not.
 */
#define IS_UART_HANDLE_INIT(hal_uart_handle) (hal_uart_handle != NULL && hal_uart_handle->Instance != NULL)


/*
 * Private helper function prototypes for transport layer.
 */
void _transportLayer_reset(void);


/*
 * File-scope static variables for transport layer functionality across
 * function calls.  (Layer Operational Variables)
 */
static UART_HandleTypeDef* _uartHandle = NULL;		// pointer to HAL uart handle, for HAL calls
static uint8_t _txBuffer[UART_PACKET_SIZE] = {0};	// transmission buffer (to be replaced by queue)
static uint8_t _rxBuffer[UART_PACKET_SIZE] = {0};	// reception buffer (to be replaced by queue)
static bool _txBuffer_full = false;					// transmission buffer full flag
static bool _rxBuffer_full = false;					// reception buffer full flag


/* uartTransport_init
 *
 * Stores pointer to HAL UART handle and resets the other transport layer
 * operational variables.
 *
 * Note:  will not re-initalize until the layer has been de-initalized.
 */
bool uartTransport_init(UART_HandleTypeDef* huart)
{
	// if module not already initialized and the uart handle passed is initialized
	if (!IS_UART_HANDLE_INIT(_uartHandle) && IS_UART_HANDLE_INIT(huart))
	{
		_uartHandle = huart;		// store handle pointer
		_transportLayer_reset();	// reset the module's operational variables
		return true;				// return success
	}

	// module already initializes or handle passed is not initialized
	else
	{
		return false;
	}
}


/* uartTransport_reset
 *
 * Resets the transport layer operational variables, except the HAL UART handle
 * pointer, to their original state.
 *
 * Note:  will not reset if the layer has not been initialized.
 */
bool uartTransport_reset(void)
{
	// if module initialized
	if (IS_UART_HANDLE_INIT(_uartHandle))
	{
		_transportLayer_reset();	// reset operational variables
		return true;				// return success
	}

	// if module not initialized
	else
	{
		return false;
	}
}


/* uartTransport_deinit
 *
 * Sets the HAL UART handle pointer to NULL.  To be used before the UART is being
 * deinitialized by the HAL.
 */
bool uartTransport_deinit(void)
{
	// if module initialized
	if (IS_UART_HANDLE_INIT(_uartHandle))
	{
		_uartHandle = NULL;		// clear pointer to uart handle
		return true;			// return success
	}

	// if module not initialized
	else
	{
		return false;
	}
}


/* uartTransport_enqueueTx
 *
 * Enqueues a packet for transmission.  Only successful if the layer has been
 * initialized.  Reports if queuing could or could not be performed due to the
 * tx buffer being full.
 */
TransportStatus uartTransport_bufferTx(uint8_t header[UART_PACKET_HEADER_SIZE], uint8_t body[UART_PACKET_PAYLOAD_SIZE])
{
	// if module initialized
	if (IS_UART_HANDLE_INIT(_uartHandle))
	{
		// if the transmit buffer is in use (program has queued a packet but
		// has not yet sent it)
		if (_txBuffer_full)
		{
			return TRANSPORT_TX_FULL;
		}

		// the buffer is empty and ready to receive a new packet
		else
		{
			// Compose header and body into one message
			composePacket(_txBuffer, header, body);
			_txBuffer_full = true;

			return TRANSPORT_OKAY;
		}
	}

	// the module has not been initialized
	else
	{
		return TRANSPORT_NOT_INIT;
	}
}


/* uartTransport_dequeueRx
 *
 * Dequeues a packet from those that have been received.  Only successful if
 * the layer has been initialized.  Reportes of dequeuing could or could not be
 * performed due to the rx buffer being empty.
 */
TransportStatus uartTransport_debufferRx(uint8_t header[UART_PACKET_HEADER_SIZE], uint8_t body[UART_PACKET_PAYLOAD_SIZE])
{
	// if the module has been initialized
	if (IS_UART_HANDLE_INIT(_uartHandle))
	{
		// if no packet has been received
		if (!_rxBuffer_full)
		{
			return TRANSPORT_RX_EMPTY;
		}

		// packet received and ready
		else
		{
			// retrieve message from buffer
			// decompose header and body from message
			decomposePacket(header, body, _rxBuffer);
			_rxBuffer_full = false;

			return TRANSPORT_OKAY;
		}
	}

	// the module has not been initialized
	else
	{
		return TRANSPORT_NOT_INIT;
	}
}


/* uartTransport_tx_polled
 *
 * Transmits all packets in tx queue.  Reports if the tx queue is empty
 * (to start) or the state of the transmissions (success or failure).
 * Uses HAL calls.
 */
TransportStatus uartTransport_tx_polled(uint32_t timeout_ms)
{
	HAL_StatusTypeDef hal_status;

	// if the module has been initalized
	if (IS_UART_HANDLE_INIT(_uartHandle))
	{
		// only transmit if a message has been queued
		if (!_txBuffer_full)
		{
			return TRANSPORT_TX_EMPTY;
		}

		// transmit the message
		hal_status = HAL_UART_Transmit(_uartHandle, (uint8_t*)_txBuffer, UART_PACKET_SIZE, timeout_ms);

		// alias the has status with transport layer status
		if (hal_status == HAL_ERROR)
		{
			/*
			 * Note †: this error occurs if pData passed into HAL_UART_Transmit() is NULL
			 * or Size passed in is not greater than 0.
			 */
			return TRANSPORT_ERROR;
		}
		else if (hal_status == HAL_TIMEOUT)
		{
			return TRANSPORT_TIMEOUT;
		}
		else if (hal_status == HAL_BUSY)
		{
			return TRANSPORT_BUSY;
		}
		else
		{
			// transmission successful
			_txBuffer_full = false;
			return TRANSPORT_OKAY;
		}
	}

	// the module has not been initialized
	else
	{
		return TRANSPORT_NOT_INIT;
	}
}


/* uartTransport_rx_polled
 *
 * Receives packets and enqueues them to the rx queue.  Reports of the
 * rx queue was full (to start) or the state of the receptions (success
 * or failure).  Uses HAL calls.
 */
TransportStatus uartTransport_rx_polled(uint32_t timeout_ms)
{
	HAL_StatusTypeDef hal_status;

	// if the module has been initialized
	if (IS_UART_HANDLE_INIT(_uartHandle))
	{
		// only receive if the buffer is empty
		if (_rxBuffer_full)
		{
			return TRANSPORT_RX_FULL;
		}

		// receive a message
		hal_status = HAL_UART_Receive(_uartHandle, (uint8_t*)_rxBuffer, UART_PACKET_SIZE, timeout_ms);

		// alias the has status with transport layer status
		if (hal_status == HAL_ERROR)
		{
			/*
			 * Note †: this error occurs if pData passed into HAL_UART_Transmit() is NULL
			 * or Size passed in is not greater than 0.
			 */
			return TRANSPORT_ERROR;
		}
		else if (hal_status == HAL_TIMEOUT)
		{
			return TRANSPORT_TIMEOUT;
		}
		else if (hal_status == HAL_BUSY)
		{
			return TRANSPORT_BUSY;
		}
		else
		{
			// reception was successful and a packet was received
			_rxBuffer_full = true;
			return TRANSPORT_OKAY;
		}
	}

	// the module is not initialized
	else
	{
		return TRANSPORT_NOT_INIT;
	}
}


/* _transportLayer_reset
 *
 * Resets operational variables other than the HAL UART handle pointer.
 */
void _transportLayer_reset(void)
{
	// clear buffers and flags
	memset(_txBuffer, 0, UART_PACKET_SIZE * sizeof(uint8_t));
	memset(_rxBuffer, 0, UART_PACKET_SIZE * sizeof(uint8_t));
	_txBuffer_full = false;
	_rxBuffer_full = false;
}

