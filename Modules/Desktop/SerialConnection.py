# Author: Kevin Imlay

import serial


# Defines communication parameters.  Same as what has been programmed to MCU.
DEFAULT_BAUD = 9600
DEFAULT_BYTESIZE = serial.SEVENBITS
DEFAULT_PARITY = serial.PARITY_NONE
DEFAULT_STOPBITS = serial.STOPBITS_TWO
DEFAULT_READ_TIMEOUT = 0.7
DEFAULT_WRITE_TIMEOUT = 1.0
DEFAULT_SOFT_FLOW_CONTRL = False
DEFAULT_RTSCTS_FLOW_CONTRL = False
DEFAULT_DSRDTR_FLOW_CONTRL = False
DEFAULT_INTER_BYTE_TIMEOUT = None
DEFAULT_EXCLUSIVE = False


class SerialConnection:
    # Serial Connection encapsulates the most basic functions for sending and
    # receiving data over the UART to the STM32 MCU.  Communication is
    # performed using the pySerial package.

    # serial connection parameters
    _connection = None


    def __init__(self):
        # Initialize new connection object with defualt serial parameters.
        # These parameters are what are also programmed into the MCU.
        #
        # Raises a ValueError if a value is out of range

        # Create new serial object
        self._connection = serial.Serial()

        # Set parameters for serial communication.
        self._connection.baudrate = DEFAULT_BAUD
        self._connection.bytesize = DEFAULT_BYTESIZE
        self._connection.parity = DEFAULT_PARITY
        self._connection.stopbits = DEFAULT_STOPBITS
        self._connection.timeout = DEFAULT_READ_TIMEOUT
        self._connection.write_timeout = DEFAULT_WRITE_TIMEOUT
        self._connection.xonxoff = DEFAULT_SOFT_FLOW_CONTRL
        self._connection.rtscts = DEFAULT_RTSCTS_FLOW_CONTRL
        self._connection.dsrdtr = DEFAULT_DSRDTR_FLOW_CONTRL
        self._connection.inter_byte_timeout = DEFAULT_INTER_BYTE_TIMEOUT
        self._connection.exclusive = DEFAULT_EXCLUSIVE


    def openPort(self, port):
        # Alias to open a serial connection.  The port must be a string
        # representation of the directory and port to open on the OS.
        # 
        # Raises a serial.SerialException if opening on the port fails.

        # Test for valid port parameter.
        if not isinstance(port, str): raise TypeError

        # Set port field for future purposes.
        self._connection.port = port

        # Try to open connection.
        self._connection.open()


    def closePort(self):
        # Alias to close serial connection.

        # Try to close connection.
        self._connection.close()


    def send(self, message):
        # Alias to send a message over the serial connection.  The message
        # must be a string that can be encoded to ASCII.
        #
        # Raises a serial.SerialException if the connection is not open.

        # Test for valid message parameter.
        if not isinstance(message, str): raise TypeError

        # Encode message and send message.  Ensure message is sent before 
        # continuing.
        # print('  ::SENDING::  ' + message)
        self._connection.write(message.encode('ascii'))
        self._connection.flush()


    def receive(self, length):
        # Alias to receive a message from the serial connection.  The length
        # must be an integer greater than 0.
        #
        # Raises a serial.SerialException if the connection is not open.

        # Test for valid length parameter.
        if not isinstance(length, int): TypeError
        if length < 1: raise ValueError

        # Read from the serial connection, decode, and return string.
        received = self._connection.read(length).decode('ascii')
        # print('  ::RECEIVING::  ' + received)
        return received
