# STM32WL5x Desktop Communication

The STM32WL5x Desktop Communication module provides a slightly-more-than basic communication module for the Nucleo-WL55JCx development board.  A set of python scripts for a desktop and an MCU module are provided as a pair for sending messages between a desktop and the Nucleo-WL55JCx development board.

___

## Conceptual Design

The Desktop Communication module was developed out of the need to program settings to and read settings and data from a Nucleo-WL55JCx development board.  A desktop computer acts as a master with the MCU as a slave, sending tasks to the MCU or requesting information from the MCU.  The module encapsulates this functionality and provides an API for use on both the MCU and the desktop, allowing a programmer to design their applications around it.

At the highest level, the module establishes communication sessions between a python desktop application and the MCU's main loop.  Within an session, the desktop can interact with the MCU over serial communication.  For more into how the communication works see the *Protocol* section.

___

## Installation/Usage

### NUCLEO

To use the Desktop Communication module on your Nucleo development board add the provided directory to your STM32CubeMX project and include mode_timer.h.

#### STM32CubeMX Configuration

Before you can use the module a hardware timer must be enabled an configured.  **One possible configuration is as follows:**

1. Open the STM32CubeMX configuration tool within your project and enable the USART2 on the core you would like to develop within.
2. Make sure the mode is asynchronous and RS-232 flow control is disabled.  These settings are for compatibility with the UART to VCOM chip the Nucleo development board uses.
3. Set the baud rate to 9600 Bits/s, the word length to 8 bits (including parity), the parity to None, and the number of stop bits to 2.  These settings are for compatibility with the desktop test application provided, but make sure these are identical between both the MCU and the desktop application's settings.
4. Set the overrun option to Disable.  The module does not perform any handling of an overrun.

![UART2 Config 1](./Assets/Images/uart2_config_1.png)

#### Adding to Your STM32CubeMX Project

Now you are ready to add the [module code](Modules/MCU/Modules).  Create a new source folder and add the module's header files to the compiler's include directory.

1. Right click on the sub-project for the core you want to use the module in.
2. Navigate to New > Source Folder.
3. Name the new Source Folder Modules.  Copy the [Desktop_Communication](Modules/MCU/Modules) folder into here.
4. Right click again on the same sub-project.  Navigate to Properties.
5. On the left side, navigate to C/C++ Build > Settings.
6. Within that window navigate to MCU GCC Compiler > Include Paths.
7. Add a new Include Path.
8. Enter "../Modules/Desktop_Communication/Inc".
9. Click Okay and exit the window.
10. Now include "desktop_app_session.h" in the file you want to use it within.

![Calendar Import 1](./Assets/Images/import_1.png)
![Calendar Import 2](./Assets/Images/import_2.png)
![Calendar Import 3](./Assets/Images/import_3.png)
![Calendar Import 4](./Assets/Images/import_4.png)
![Calendar Import 5](./Assets/Images/import_5.png)
![Calendar Import 6](./Assets/Images/import_6.png)
![Calendar Import 7](./Assets/Images/import_7.png)

### Desktop

todo: making a new dir for desktop app
todo: add pyserial stuff here
todo: [module code](Modules/Desktop)

### Example Usage

A simple usage [example STM32CubeMX project](Example) is provided along with a sample Desktop Application.  This example shows the session-level command "ECHO" and a custom application-level command "LED".  The ECHO command just asks the MCU to echo back the message exactly as it was received, which can be useful for testing purposes.  The LED command we will define to toggle the blue LED on the Nucleo development board.

First, lets look at the MCU.

#### MCU

For a more interesting example I have included an additional module called LED Debug.  All it does is provide some basic functionality to turn on and off the LEDs on the STM32WL55JC development board.

On most STM32 development boards the USART 2 is connected to the VCOM used to
communicate with the desktop.  After initializing the USART 2 in the HAL, the desktop application session manager can be initialized:

    // initialize the Desktop App Communication
    desktopAppSession_init(&huart2);

Within the main loop, call the session manager to try to open a session with the desktop application.  Toggle the green LED to signal while the session is open.

    // Attempt to open a session,
    // will skip attempt if a session is already open
    if (desktopAppSession_start() == SESSION_OKAY)
        // if a session was opened, turn on the green LED
        activate_led(GREEN_LED);
    else
        // if the session is closed, turn off green LED
        deactivate_led(GREEN_LED);

Make a call to update the session manager.  If the session is not open this no update is performed.

    // update the session manager
    desktopAppSession_update();

Now, check for any queued commands from the desktop application.  Again, if the session is not open this is skipped.  If a message is received that gives the command to toggle the blue LED, toggle it and report to the desktop that it has been toggled.  This command is given through a message with the header "LED" and the body "toggle blue LED".

```
// get message from desktop if there is one
if (desktopAppSession_dequeueMessage(message_command_buffer, message_payload_buffer) == SESSION_OKAY)
{
    // if the command is "LED/0" and payload is "toggle blue LED\0", toggle the blue LED
    if (!strncmp(message_command_buffer, "LED\0", UART_PACKET_HEADER_SIZE)
            && !strncmp(message_payload_buffer, "toggle blue LED\0", UART_PACKET_PAYLOAD_SIZE))
    {
        if (!blueLedOn)
        {
            // turn led on
            activate_led(BLUE_LED);
            blueLedOn = 1;

            // report it to desktop
            memset(message_payload_buffer, 0, sizeof(char) * UART_PACKET_PAYLOAD_SIZE);
            strncpy(message_payload_buffer, "blue LED is now on\0", sizeof(char) * UART_PACKET_PAYLOAD_SIZE);
            desktopAppSession_enqueueMessage("LED/0", message_payload_buffer);
            // Note that this only enqueues the message to be sent the next time
            // desktopAppSession_update() is called.
        }
        else
        {
            // turn led off
            deactivate_led(BLUE_LED);
            blueLedOn = 0;

            // report it to desktop
            memset(message_payload_buffer, 0, sizeof(char) * UART_PACKET_PAYLOAD_SIZE);
            strncpy(message_payload_buffer, "blue LED is now off\0", sizeof(char) * UART_PACKET_PAYLOAD_SIZE);
            desktopAppSession_enqueueMessage("LED/0", message_payload_buffer);
            // Note that this only enqueues the message to be sent the next time
            // desktopAppSession_update() is called.
        }
    }
}
```

#### Desktop

Now let us look at the desktop application to interface with this.  The example desktop application provides additional framework to help identify what port the MCU will be connected to based on the desktop's OS.  I will skip over this.

To open a session, call:

    # attempt to make connection
    Stm32Session = SerialSession.STM32SerialCom(availablePort)

Now send an ECHO command and an LED command, repeating for a total of 20 commands.  Also, listen for the messages sent back from the MCU at the end of each iteration.

```
    for _ in range(10):
        # sleep for 1 second for easy watching of terminal
        time.sleep(1)

        # queue an echo message with a random payload
        Stm32Session._outMessageQueue.put(('ECHO','Hello World! {}'.format(random.randrange(200))))

        # queue a command to toggle the blue LED
        Stm32Session._outMessageQueue.put(('LED\0', 'toggle blue LED'))

        # update the session (send and receive messages with MCU)
        Stm32Session.update()

        # if the MCU sent any messages, print them to the terminal
        while not Stm32Session._inMessageQueue.empty():
            print(Stm32Session._inMessageQueue.get()[1])
```

Call the desktop application by opening a terminal in the folder containing Desktop_App_Example.py and call this file with:

    python3 Desktop_App_Example.py

You will get an output like the following.  If you get three lines that say there was a malformed packet, try again at least once.  If that doesn't fix it, reset the MCU and try again.

    Connected to port /dev/tty.usbmodem143403
      ::SENDING::  ECHOHello World! 130
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  Hello World! 130
      ::SENDING::  ECHOHello World! 192
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now on
      ::RECEIVED::  Hello World! 192
      ::SENDING::  ECHOHello World! 136
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now off
      ::RECEIVED::  Hello World! 136
      ::SENDING::  ECHOHello World! 148
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now on
      ::RECEIVED::  Hello World! 148
      ::SENDING::  ECHOHello World! 0
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now off
      ::RECEIVED::  Hello World! 0
      ::SENDING::  ECHOHello World! 42
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now on
      ::RECEIVED::  Hello World! 42
      ::SENDING::  ECHOHello World! 14
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now off
      ::RECEIVED::  Hello World! 14
      ::SENDING::  ECHOHello World! 57
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now on
      ::RECEIVED::  Hello World! 57
      ::SENDING::  ECHOHello World! 116
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now off
      ::RECEIVED::  Hello World! 116
      ::SENDING::  ECHOHello World! 74
      ::SENDING::  LEDtoggle blue LED
      ::RECEIVED::  blue LED is now on
      ::RECEIVED::  Hello World! 74
      ::DISCONNECTING::  Port /dev/tty.usbmodem143403
    Disconnected from port /dev/tty.usbmodem143403

Let's dissect this output.  A connection is established and a session is opened on port "/dev/tty.usbmodem143403".  Then all the messages exchanged with the MCU.  Most notably, reports from the MCU seem like they are off in timing.  Take a look at the first six messages exchanged.  The MCU echoes the first echo command immediately but the MCU does not report turning the LED on until one iteration later.  This is because session-level commands (i.e. ECHO) are responded to immediately where the application-level messages are queued and sent from the MCU when it gets around to it, and before any new messages are received from the desktop.  In our example program this is at the start of the main loop.  This example highlights how the desktop application must not expect all messages from the MCU to arrive in a particular order.

___

## Notable Design Choices and Limitations

### SysTick

For the module to function, the SysTick timer must not be disabled.  The HAL uses the SysTick timer for timeouts with the UART peripheral.

### Protocol



#### TX and RX Behaviors

Both the MCU and the Desktop are configured to receive and transmit messages with a timeout (the MCU by how the polling TX and RX function in the HAL and the Desktop by how the pySerial package functions).  Due to this timeout behavior, both the Desktop and the MCU can experience a partial reception or transmission of packets.  This is mitigated using software flow control.

#### Handshaking and Sessions

Before messages can be sent between the MCU and the Desktop, a handshake takes place.  This is performed mainly for the Desktop side to find the serial port that the MCU is connected to but allows for the MCU to also be aware when the Desktop is connected.  This state of whether the two are connected and ready for communication is called a Session.  The Session is considered open when both sides are ready to send and receive messages and is considered closed otherwise.  Sessions can be opened with a handshake or closed with a different handshake.

The MCU and the Desktop treat open and closed sessions differently.  A closed session to the Desktop just tells the user that the MCU is not connected and prevents the Desktop application from attempting communication.  A closed session to the MCU can prevent it from spending time and power listening for messages while the Desktop is not connected, which can be costly with long timeout periods for listening.

#### Software Flow Control

The MCU and the Desktop buffer messages differently as well.  The MCU has only one buffer the size of a message prepared for receiving, and one for transmitting messages.  This is contrasted with the Desktop with a buffer managed by the OS and large enough to hold multiple messages.

To keep behavior simple and predictable on the MCU, a polling approach is applied to the serial communications.  To avoid scenarios 2 and 3 of RX, the MCU sends a clear-to-send (CTS) message to the Desktop before starting a reception period.  It is up to the Desktop to wait for this CTS message before sending only one message to the MCU.  If no message is received by the MCU, it simply moves on.  If a full message is received, then it processes that message. 

Due to the Desktop’s ability to buffer several messages gives the MCU more flexibility in sending messages.  To keep behavior simple, a non-CTS message is sent before the CTS message is.  The Desktop will ignore CTS messages received until it is ready to send a message and queues any non-CTS messages for processing.  If the Desktop has multiple messages to send, it queues them and synchronizes with the MCU with the help of CTS messages to send one at a time.

#### Message Architecture and Function with Flow Control

Messages are defined as having two parts, a header and a body or sometimes referred to as a command and data/info for that command.  They have a fixed total length and a fixed length for the header, and consequently a fixed length for the body.  The header contains a character code that signals how the body of the message is to be treated.  For example, a message [‘ECHO’, ‘Hello!’] sent to the MCU is asking the MCU to echo back the data in the body and would return to the computer a message with ‘Hello!’ in the body.

Some message header codes are reserved for controlling the state of Sessions (session-level commands).  In the opening handshake the headers ‘SYNC’ (for synchronize), ‘ACKN’ (for acknowledge), and ‘SYAC’ (for synchronize acknowledge) are used.  In the closing handshake the header ‘DISC’ (for disconnect) is used and the ‘ACNK’ header returned from the MCU.

Some message header codes are reserved for software flow control.  Currently only the ‘CTS\0’ (for clear-to-send) header is used.

#### Message Function with Application Behavior

Additional message headers can be added for actions the Desktop sends to be performed on the MCU.  Consider the 'LED\0' command in the example.  This can be expanded to cover many different possible settings of the MCU such as getting and setting the date and time.

___

## API

### Defines

1. UART_PACKET_SIZE (uart_helpers.h) - sets the number of bytes in a serial packet.  Must be the same as MESSAGE_LENGTH.
2. MESSAGE_LENGTH (SerialProtocol.py) - sets the number of bytes in a serial packet.  Must be the same as UART_PACKET_SIZE.
3. UART_PACKET_HEADER_SIZE (uart_helpers.h) - sets the number of bytes in a serial packet's header/command segment.  Must be the same as HEADER_LENGTH.
4. HEADER_LENGTH (SerialProtocol.py) - sets the number of bytes in a serial packet's header/command segment.  Must be the same as UART_PACKET_HEADER_SIZE.
5. DEFAULT_BAUD (SerialConnection.py) - serial baud rate.  Must be the same as the baud rate set in STM32CubeMX.
6. DEFAULT_BYTESIZE (SerialConnection.py) - number of bits in serial frame.  Must be the same as set in STM32CubeMX.
7. DEFAULT_PARITY (SerialConnection.py) - parity bit setting of serial frame.  Must be the same as set in STM32CubeMX.
8. DEFAULT_STOPBITS (SerialConnection.py) - number of stop bits of serial frame.  Must be the same as set in STM32CubeMX.
9. DEFAULT_READ_TIMEOUT (SerialConnection.py) - timeout for receiving from MCU.
10. DEFAULT_WRITE_TIMEOUT (SerialConnection.py) - timeout for transmitting to MCU.
11. RECEIVE_TIMEOUT_MS (desktop_app_session.h) - timeout for receiving from the desktop.
12. SEND_TIMEOUT_MS (desktop_app_session.h) - timeout for transmitting to the desktop.
13. SESSION_START_TIMEOUT_MS (desktop_app_session.h) - timeout for receiving during handshake.

### Return Codes

1. **DesktopComSessionStatus** - Status code returns from API function calls to signal if the call was successful or why the call was unsuccessful:
    - **SESSION_OKAY** - API function call was successfully completed.
    - **SESSION_NOT_INIT** - The module has not been initialized before API call.
    - **SESSION_TIMEOUT** - A timeout has occurred with a transmission or a reception.
    - **SESSION_ERROR** - An error occurred with the UART.
    - **SESSION_NOT_OPEN** - A session is not established with the desktop application.
    - **SESSION_BUSY** - The UART is busy, try again later.
    - **SESSION_CLOSED** - A session is not established with the desktop application.
    - **SESSION_BUFFER_EMPTY** - The serial manager's message buffer is empty.
    - **SESSION_BUFFER_FULL** - The serial manager's message buffer is full.

### Functions

1. **bool desktopAppSession_init(UART_HandleTypeDef* huart)** - Initialize the mode timer module.  Must be called before the module can operate.
    - Parameters:
        - **huart** - HAL UART handle pointer.
    - Return:
        - **bool** - false if NULL or uninitialized HAL UART handle passed, true otherwise.
    - Note:
        - Dependency on uartTransport_init()

2. **bool sessionOpen(void)** - Returns if a session is open.
    - Return:
        bool - true if a session is open, false otherwise.
    - Note:
        - will always return false if the session module has not been initialized.

3. **bool desktopAppSession_deinit(void)** - Deinitialize the module.

4. **DesktopComSessionStatus desktopAppSession_start(void)** - Attempts to start a session with the desktop application.  Performs start handshake with desktop computer if present, starting the session.  Waits on timeout.
    - Return:
        - SESSION_NOT_INIT - if desktopAppSession_init() has not been performed prior
        - SESSION_OKAY - if a session is already open or if successfully opened
        - SESSION_ERROR - if an error occurred during UART communication
        - SESSION_TIMEOUT - if the desktop application did not attempt to start a session.
        - SESSION_OPEN - a session is already open
    - Note:
        - Software flow control is not used while listening for first step of handshake, which can cause difficulty for the desktop application to establish a handshake successfully.  This is a point for future development.

5. **DesktopComSessionStatus desktopAppSession_stop(void)** - Force-closes a session with the desktop application if a session is open.
    - Return:
        - SESSION_NOT_INIT - if desktopAppSession_init() has not been performed prior
        - SESSION_OKAY - if a session is already closed or if successfully opened
    - Note:
        - Undefined - this function's implementation is a point for future development.

6. **DesktopComSessionStatus desktopAppSession_update(void)** - Performs an update of the state of the session manager.  Any queued messages for transmission are sent, then reception of messages from the desktop application are received.
    - Return:
        - SESSION_NOT_INIT - if desktopAppSession_init() has not been performed prior
        - SESSION_NOT_OPEN - if a session has not been opened with the desktop application
        - SESSION_ERROR - if an error occurred with the UART communication
        - SESSION_OKAY - otherwise (does not distinguish whether or not any messages were received.
    - Note:
        - Updating the session only stores received messages in a queue.  Getting received messages requires the use of the desktopAppSession_dequeueMessage() function.

7. **DesktopComSessionStatus desktopAppSession_enqueueMessage(char header[UART_PACKET_HEADER_SIZE], char body[UART_PACKET_PAYLOAD_SIZE])** - Enqueue a message for later transmission to the desktop application.
    - Parameters:
        - header - char array message header code
        - body - char array message body (or payload)
    - Return:
        - SESSION_NOT_INIT - if desktopAppSession_init() has not been performed prior
        - SESSION_BUFFER_FULL - if the queue is full
        - SESSION_OKAY - if enqueuing successful

8. **DesktopComSessionStatus desktopAppSession_dequeueMessage(char header[UART_PACKET_HEADER_SIZE], char body[UART_PACKET_PAYLOAD_SIZE])** - Dequeues a message that has been received from the desktop application.
    - Parameters:
        - header - char array pointer where the message header code is to be stored
        - body - char array pointer where the message body (or payload) is to be stored
    - Return:
        - SESSION_NOT_INIT - if desktopAppSession_init() has not been performed prior
        - SESSION_BUFFER_EMPTY - if the queue is empty
        - SESSION_OKAY - if dequeuing successful
