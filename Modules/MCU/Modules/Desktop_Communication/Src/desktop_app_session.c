/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 */


#include <desktop_app_session.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>


/*
 * Private helper function prototypes for session manager.
 */
DesktopComSessionStatus _handshake(unsigned int timeout_ms);
DesktopComSessionStatus _session_update(void);
DesktopComSessionStatus _listen(void);
DesktopComSessionStatus _tell(void);


/*
 * File-scope static variables for session manager functionality across
 * function calls.  (Manager Operational Variables)
 */
static bool _sessionOpen = false;						// Flag to signal if a session is open
static bool _sessionInit = false;						// Flag to signal if the manager is initialized
static char _messageCommand[UART_PACKET_HEADER_SIZE];	// Rx buffer for header (used for processing in manager)
static char _messageData[UART_PACKET_PAYLOAD_SIZE];		// Rx buffer for body (used for processing in manager)
static bool _messageReady = false;						// Flag to signal if a message is in the Rx buffer


/* desktopAppSession_init
 *
 * Initializes the UART transport layer and resets operational variables for the manager.
 * Only will initialize if the manager has not been initialized already.
 */
bool desktopAppSession_init(UART_HandleTypeDef* huart)
{
	// initialize transport layer
	if (!_sessionInit && uartTransport_init(huart))
	{
		// reset operational variables
		_sessionOpen = false;
		_sessionInit = true;
		_messageReady = false;
		memset(_messageCommand, 0, UART_PACKET_HEADER_SIZE * sizeof(char));
		memset(_messageData, 0, UART_PACKET_PAYLOAD_SIZE * sizeof(char));

		return true;
	}

	// an uninitialized uart handle was passed.
	else
	{
		return false;
	}
}


/* sessionOpen
 *
 * Return if the session is initialized and open.
 */
bool sessionOpen(void)
{
	return _sessionInit && _sessionOpen;
}


/* desktopAppSession_start
 *
 * Attempts to handshake with the desktop application.  Wrapper for the handshake function.
 * Will not attempt if the manager has not been initialized and will not attempt if a
 * session is already open.
 */
DesktopComSessionStatus desktopAppSession_start(void)
{
	DesktopComSessionStatus handshakeStatus;

	// if the module has been initialized
	if (_sessionInit)
	{
		// only attempt to handshake if a session is not already open
		if (!_sessionOpen)
		{
			// perform handshake and return result
			handshakeStatus = _handshake(SESSION_START_TIMEOUT_MS);
			if (handshakeStatus == SESSION_OKAY)
				_sessionOpen = true;
			return handshakeStatus;
		}

		// if session is open
		else
		{
			return SESSION_OKAY;
		}
	}

	// module not initialized
	else
	{
		return SESSION_NOT_INIT;
	}
}


/* desktopAppSession_stop
 *
 * Force the end of a session by sending the end session confirmation code
 * to the desktop application.  Only ends a session if the session manager
 * has been initialized and a session is open.
 *
 * Note:  needs to be implemented.
 */
DesktopComSessionStatus desktopAppSession_stop(void)
{
	return SESSION_OKAY;
}


/* desktopAppSession_update
 *
 * Update the state of the session manager.  Wraps the _session_cycle() function,
 * which performs the actual update, with checks for a session to be opened.
 */
DesktopComSessionStatus desktopAppSession_update(void)
{
	// if the module has been initialized
	if (_sessionInit)
	{
		// only run _update() if a session is opened
		if (_sessionOpen)
		{
			return _session_update();
		}

		// a session has not been opened
		else
		{
			return SESSION_NOT_OPEN;
		}
	}

	// the module has not been initialized
	else
	{
		return SESSION_NOT_INIT;
	}
}


/* desktopAppSession_enqueueMessage
 *
 * Buffers a single message into the transport layer tx buffer.
 *
 * todo: Need to add a queue in the session manager for this.
 */
DesktopComSessionStatus desktopAppSession_enqueueMessage(char header[UART_PACKET_HEADER_SIZE],
		char body[UART_PACKET_PAYLOAD_SIZE])
{
	// if the module has been initialized
	if (_sessionInit)
	{
		// try to enqueue message and return if successful
		if (uartTransport_bufferTx((uint8_t*)header, (uint8_t*)body) != TRANSPORT_OKAY)
		{
			return SESSION_BUFFER_FULL;
		}
		else
		{
			return SESSION_OKAY;
		}
	}

	// module has not been initialized
	else
	{
		return SESSION_NOT_INIT;
	}
}


/* desktopAppSession_dequeueMessage
 *
 * Debuffers from the session manager's header and body buffer.  See note of this buffer
 * above.
 *
 * todo: Need to add a queue in the session manager for this.
 */
DesktopComSessionStatus desktopAppSession_dequeueMessage(char header[UART_PACKET_HEADER_SIZE], char body[UART_PACKET_PAYLOAD_SIZE])
{
	// if the module has been initialized
	if (_sessionInit)
	{
		// if a message is present in the received queue, copy to output
		if (_messageReady)
		{
			memcpy(header, _messageCommand, UART_PACKET_HEADER_SIZE*sizeof(char));
			memcpy(body, _messageData, UART_PACKET_PAYLOAD_SIZE*sizeof(char));
			_messageReady = false;

			return SESSION_OKAY;
		}

		// no message is ready
		else
		{
			return SESSION_BUFFER_EMPTY;
		}
	}

	// the module has not been initialized
	else
	{
		return SESSION_NOT_INIT;
	}
}


/* _handshake
 *
 * Performs handshake with desktop application.  Listens for incomming request to
 * open a session with the SESSION_START_TIMEOUT_MS value.  If a message is received
 * with the HANDSHAKE_HEADER_SYNC header command, then handshaking begins.  A message
 * is sent with the HANDSHAKE_HEADER_ACKN header command is sent and listening begins
 * again with the RECEIVE_TIMEOUT_MS timeout value.  If the HANDSHAKE_HEADER_SYNACK
 * header command is received, then a session is opened.
 *
 * This series of steps for the handshake confirms that timeout values on the MCU are
 * not too short (as long as the Desktop is sufficiently fast enough at responding
 * messages from the MCU).  Timeout values may need to be tweaked if handshaking
 * consistently fails.
 *
 * A state machine approach is used for confirming each step in the handshaking process.
 * These states are as follows:
 * 	0)	Initial state/reset state.  Waiting for a message from the desktop application.
 * 	1)	Dequeue the message received.
 * 	2)	Check if the message is synchronization message.
 * 	3)	Queue acknowledge message.
 * 	4)	Transmit message to desktop application.
 * 	5)	Wait for incoming message from the desktop application.
 * 	6)	Dequeue the message received.
 * 	7)	Check if the message is a synchronization acknowledge message.
 * 	8)	Implicit.  Handshaking successful.
 *
 * 	The state machine is used simply as a list of steps that must be checked off in order.
 * 	If any one step fails, handshaking fails.
 *
 * Note:  no software flow control is used for the first message.  Listening for the
 * first message from the desktop may timeout and cause synchronization issues while
 * attempting to handshake.
 */
DesktopComSessionStatus _handshake(unsigned int timeout_ms)
{
	unsigned int state = 0;
	bool error = false;
	bool success = false;
	TransportStatus transportStatus;
	char messageHeader[UART_PACKET_HEADER_SIZE] = {0};
	char messageBody[UART_PACKET_PAYLOAD_SIZE] = {0};

	// while the handshake follows proper steps and UART communication does not error
	while (!success && !error)
	{
		// state 0:  receive message
		if (state == 0)
		{
			transportStatus = uartTransport_rx_polled(timeout_ms); // handshake timeout until start of handshake
		}
		// state 1: message received, dequeue
		else if (state == 1)
		{
			transportStatus = uartTransport_debufferRx((uint8_t*)messageHeader, (uint8_t*)messageBody);
		}
		// state 2: check if sync
		else if (state == 2)
		{
			if (strncmp(messageHeader, HANDSHAKE_HEADER_SYNC, UART_PACKET_HEADER_SIZE))
			{
				error = true;
			}
		}
		// state 3: sync received, queue ack
		else if (state == 3)
		{
			memset(messageBody,0,UART_PACKET_PAYLOAD_SIZE);
			transportStatus = uartTransport_bufferTx((uint8_t*)HANDSHAKE_HEADER_ACKN, (uint8_t*)messageBody);
		}
		// state 4: send ack
		else if (state == 4)
		{
			transportStatus = uartTransport_tx_polled(SEND_TIMEOUT_MS);
		}
		// state 5: ack sent, receive message
		else if (state == 5)
		{
			transportStatus = uartTransport_rx_polled(RECEIVE_TIMEOUT_MS);
		}
		// state 6: dequeue message
		else if (state == 6)
		{
			transportStatus = uartTransport_debufferRx((uint8_t*)messageHeader, (uint8_t*)messageBody);
		}
		// state 7: message received, check if syn ack
		else // if (state == 7)
		{
			if (strncmp(messageHeader, HANDSHAKE_HEADER_SYNACK, UART_PACKET_HEADER_SIZE))
			{
				error = true;
			}
		}

		// catch status codes and move through state machine
		if (transportStatus == TRANSPORT_OKAY)
		{
			if (state == 0)
				state = 1;
			else if (state == 1)
				state = 2;
			else if (state == 2)
				state = 3;
			else if (state == 3)
				state = 4;
			else if (state == 4)
				state = 5;
			else if (state == 5)
				state = 6;
			else if (state == 6)
				state = 7;
			else // if (state == 7)
				success = true;
		}
		else
		{
			error = true;
		}
	}


	// report status of handshake
	if (success && !error)
	{
		return SESSION_OKAY;
	}
	else
	{
		if (transportStatus == TRANSPORT_TIMEOUT)
		{
			return SESSION_TIMEOUT;
		}
		else
		{
			return SESSION_ERROR;
		}
	}
}


/* _session_update
 *
 * Performs update of session manager.  First transmits a queued message, then receives
 * a message.  Checks if the message received is for the session manager (close session,
 * echo).  If it is, then the session handles appropriately.
 *
 * Note:  If a response to the desktop is necessary, this response won't be sent until
 * the next time the session is updated.
 */
DesktopComSessionStatus _session_update(void)
{
	char messageHeader[UART_PACKET_HEADER_SIZE] = {0};
	char messageBody[UART_PACKET_PAYLOAD_SIZE] = {0};
	DesktopComSessionStatus status;

	// Perform Tx message phase of session cycle.
	status = _tell();

	// Perform Rx message phase of session cycle.
	status = _listen();
	if (status == SESSION_ERROR)
	{
		return SESSION_ERROR;
	}

	// If a message was received while listening.
	else if (status == SESSION_OKAY)
	{
		// dequeue received message
		uartTransport_debufferRx((uint8_t*)messageHeader, (uint8_t*)messageBody);

		// Check if disconnection handshake message was received.
		// If so, set session open flag to false.
		if (!strncmp(messageHeader, HANDSHAKE_HEADER_DISC, UART_PACKET_HEADER_SIZE))
		{
			desktopAppSession_enqueueMessage(HANDSHAKE_HEADER_DISC, "\0");
			_tell();
			_sessionOpen = false;
			status = SESSION_CLOSED;
		}

		// Check if echo command.
		else if (!strncmp(messageHeader, ECHO_HEADER, UART_PACKET_HEADER_SIZE))
		{
			desktopAppSession_enqueueMessage(messageHeader, messageBody);
			status = _tell();
		}

		// Else, buffer for processing by the application
		else
		{
			memcpy(_messageCommand, messageHeader, UART_PACKET_HEADER_SIZE*sizeof(char));
			memcpy(_messageData, messageBody, UART_PACKET_PAYLOAD_SIZE*sizeof(char));
			_messageReady = true;
		}
	}

	return status;
}


/* _listen
 *
 * Wraps calls to the UART transmission layer.
 * Listens for a message from the desktop application.  Performs software flow control.
 *
 * Listening is divided into two windows:  CTS and Message.  The CTS window acts as
 * software flow control to let the desktop application that it is ready to receive a
 * message.  A CTS message is transmitted.  The Message window listens for a message
 * from the desktop application with the RECEIVE_TIMEOUT_MS value.  Error codes from
 * the transport layer are aliased to session error codes.
 */
DesktopComSessionStatus _listen(void)
{
	TransportStatus transportStatus;
	char messageBody[UART_PACKET_PAYLOAD_SIZE] = {0};

	// CTS Window
	// Tx the CTS message to signal to desktop that the MCU is about to be ready to
	// receive a message.
	memset(messageBody,0,UART_PACKET_PAYLOAD_SIZE);
	snprintf(messageBody, UART_PACKET_PAYLOAD_SIZE, "Clear to send!\n");
	transportStatus = uartTransport_bufferTx((uint8_t*)CTS_HEADER,(uint8_t*) messageBody);

	if (transportStatus != TRANSPORT_OKAY)
	{
		return SESSION_ERROR;
	}

	transportStatus = uartTransport_tx_polled(SEND_TIMEOUT_MS);

	if (transportStatus == TRANSPORT_TIMEOUT)
	{
		return SESSION_TIMEOUT;
	}
	else if (transportStatus != TRANSPORT_OKAY)
	{
		return SESSION_ERROR;
	}

	// Message Window
	// Rx to receive a packet from the desktop.
	transportStatus = uartTransport_rx_polled(RECEIVE_TIMEOUT_MS);

	if (transportStatus == TRANSPORT_TIMEOUT)
	{
		return SESSION_TIMEOUT;
	}
	else if (transportStatus != TRANSPORT_OKAY)
	{
		return SESSION_ERROR;
	}

	return SESSION_OKAY;
}


/* _tell
 *
 * Wraps UART transmission layer calls.
 * Transmits a buffered message to the desktop application.
 * Aliases transport layer error codes to session error codes.
 */
DesktopComSessionStatus _tell(void)
{
	TransportStatus transportStatus;

	// attempt to transmit packet
	transportStatus = uartTransport_tx_polled(SEND_TIMEOUT_MS);

	// report status of transmission
	if (transportStatus == TRANSPORT_OKAY)
	{
		return SESSION_OKAY;
	}
	else if (transportStatus == TRANSPORT_TIMEOUT)
	{
		return SESSION_TIMEOUT;
	}
	else // if (transportStatus == TRANSPORT_ERROR || transportStatus == TRANSPORT_BUSY)
	{
		return SESSION_ERROR;
	}
}
