/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 *
 * Purpose:
 *		The desktop application communication session is intended to manage
 *	the more complex behaviors for communicating with the desktop application.
 *	It builds on top of the UART transport layer, implementing software flow
 *	control, handshaking to start and stop sessions, and receiving/sending
 *	messages with the desktop application.  The use of sessions also helps
 *	minimize resource usage by the MCU while the desktop application is not
 *	connected, conserving power.
 *		Sessions follow a design pattern that updates the session's state with
 *	the use of an update function, allowing the main application (MCU) to
 *	perform this update as often as it chooses.  Updating the state will send
 *	any queued messages and receive messages from the desktop application.
 *	Messages can be queued for transmission outside the update loop, acting as
 *	a buffer that is flushed only when explicitly commanded to by the update
 *	function.  Messages can be dequeued after reception when chosen to by the
 *	main application (MCU).
 *		Sessions are also opened by the desktop application, but can be closed
 *	by either the desktop application or the MCU.
 *		Actions (transmissions, receptions, handshakes) are performed using
 *	polling with timeouts (through the UART transmission layer) to ensure non-
 *	blocking and deterministic behavior.
 *
 *
 *	Note:  In place of a proper queue for message reception and transmission,
 *	the transport layer buffer is used.  This is a point for future development.
 */

#ifndef INC_DESKTOP_APP_SESSION_LAYER_H_
#define INC_DESKTOP_APP_SESSION_LAYER_H_


#include <stdbool.h>
#include <uart_packet_helpers.h>
#include <uart_transport_layer.h>

/*
 * Timeout values, in milliseconds, for operations performed by the session manager.
 */
#define RECEIVE_TIMEOUT_MS 100
#define SEND_TIMEOUT_MS 100
#define SESSION_START_TIMEOUT_MS 1000

/*
 * Flow control message header (command) codes.
 */
#define HANDSHAKE_HEADER_SYNC "SYNC\0"
#define HANDSHAKE_HEADER_ACKN "ACKN\0"
#define HANDSHAKE_HEADER_SYNACK "SYNA\0"
#define HANDSHAKE_HEADER_DISC "DISC\0"
#define HANDSHAKE_HEADER_DISCACK "DACK\0"
#define CTS_HEADER "CTS\0\0"
#define ECHO_HEADER "ECHO\0"

/*
 * Session Manager status codes for returns.
 */
typedef enum {
	SESSION_OKAY,
	SESSION_TIMEOUT,
	SESSION_ERROR,
	SESSION_NOT_INIT,
	SESSION_NOT_OPEN,
	SESSION_BUSY,
	SESSION_CLOSED,
	SESSION_BUFFER_EMPTY,
	SESSION_BUFFER_FULL
} DesktopComSessionStatus;


/* desktopAppSession_init
 *
 * Function:
 *	Initialize the session manager before usage.
 *
 * Parameters:
 *	huart - HAL UART handle pointer.
 *
 * Return:
 *	bool - false if NULL or uninitialized HAL UART handle passed,
 *			true otherwise.
 *
 * Note:
 * 	Dependency on uartTransport_init()
 */
bool desktopAppSession_init(UART_HandleTypeDef* huart);

/* sessionOpen
 *
 * Function:
 * 	Returns if a session is open.
 *
 * Return:
 * 	bool - true if a session is open, false otherwise.
 *
 * Note:
 * 	will always return false if the session module has not been initialized.
 */
bool sessionOpen(void);

/* desktopAppSession_deinit
 *
 * Function:
 *
 */
bool desktopAppSession_deinit(void);

/* desktopAppSession_start
 *
 * Function:
 *	Attempts to start a session with the desktop application.  Performs start
 *	handshake with desktop computer if present, starting the session.  Waits
 *	on timeout.
 *
 * Return:
 *	DesktopComSessionStatus
 *		SESSION_NOT_INIT - if desktopAppSession_init() has not been performed
 *				prior
 *		SESSION_OKAY - if a session is already open or if successfully opened
 *		SESSION_ERROR - if an error occurred during UART communication
 *		SESSION_TIMEOUT - if the desktop application did not attempt to start
 *				a session.
 * 		SESSION_OPEN - a session is already open
 *
 * Note:
 * 	Software flow control is not used while listening for first step of
 * 	handshake, which can cause difficulty for the desktop application to
 * 	establish a handshake successfully.  This is a point for future development.
 */
DesktopComSessionStatus desktopAppSession_start(void);

/* desktopAppSession_stop
 *
 * Function:
 *	Force-closes a session with the desktop application if a session is open.
 *
 * Return:
 * 	DesktopComSessionStatus
 *
 * Note:
 * 	Undefined - this function's implementation is a point for future development.
 */
DesktopComSessionStatus desktopAppSession_stop(void);

/* desktopAppSession_update
 *
 * Function:
 *	Performs an update of the state of the session manager.  Any queued messages
 *	for transmission are sent, then reception of messages from the desktop
 *	application are received.
 *
 * Return:
 *	DesktopComSessionStatus
 *		SESSION_NOT_INIT - if desktopAppSession_init() has not been performed
 *				prior
 *		SESSION_NOT_OPEN - if a session has not been opened with the desktop
 *			application
 *		SESSION_ERROR - if an error occurred with the UART communication
 *		SESSION_OKAY - otherwise (does not distinguish whether or not any
 *			messages were received.
 *
 * Note:
 * 	Updating the session only stores received messages in a queue.  Getting
 * 	received messages requires the use of the desktopAppSession_dequeueMessage()
 * 	function.
 */
DesktopComSessionStatus desktopAppSession_update(void);

/* desktopAppSession_enqueueMessage
 *
 * Function:
 *	Enqueue a message for later transmission to the desktop application.
 *
 * Parameters:
 *	header - char array message header code
 *	body - char array message body (or payload)
 *
 * Return:
 *	DesktopComSessionStatus
 *		SESSION_NOT_INIT - if desktopAppSession_init() has not been performed
 *				prior
 *		SESSION_BUFFER_FULL - if the queue is full
 *		SESSION_OKAY - if enqueuing successful
 */
DesktopComSessionStatus desktopAppSession_enqueueMessage(char header[UART_PACKET_HEADER_SIZE], char body[UART_PACKET_PAYLOAD_SIZE]);

/* desktopAppSession_dequeueMessage
 *
 * Function:
 *	Dequeues a message that has been received from the desktop application.
 *
 * Parameters:
 *	header - char array pointer where the message header code is to be stored
 *	body - char array pointer where the message body (or payload) is to be stored
 *
 * Return:
 *	DesktopComSessionStatus
 *		SESSION_NOT_INIT - if desktopAppSession_init() has not been performed
 *				prior
 *		SESSION_BUFFER_EMPTY - if the queue is empty
 *		SESSION_OKAY - if dequeuing successful
 */
DesktopComSessionStatus desktopAppSession_dequeueMessage(char header[UART_PACKET_HEADER_SIZE], char body[UART_PACKET_PAYLOAD_SIZE]);


#endif /* INC_DESKTOP_APP_SESSION_LAYER_H_ */
