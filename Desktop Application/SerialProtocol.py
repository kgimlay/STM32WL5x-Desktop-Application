# Author: Kevin Imlay

import SerialConnection
import SerialPacket
import serial


# Defines message parameters
HEADER_LENGTH = 4
MESSAGE_LENGTH = 64


class SerialProtocol:
    # 

    # connection object
    _connection = None


    def __new__(cls, port):
        # Attempts to open a connection on the port provided.  If successful,
        # a SerialProtocol object is created.  If not, an exception is thrown.

        def _connect_handshake(connection):
            # 

            # clear send and receive buffers before trying handshake
            connection._connection.reset_input_buffer()
            connection._connection.reset_output_buffer()

            # compose acknowledge message
            synMessage = SerialPacket.SerialPacket(MESSAGE_LENGTH, 
                HEADER_LENGTH, 'SYNC', '')
            sendData = synMessage.format()
            
            # send acknowledge message
            connection.send(sendData)
            # print(connection._connection.out_waiting)
            
            # listen for echo back
            receivedData = connection.receive(MESSAGE_LENGTH)
            try:
                synackMessage = SerialPacket.SerialPacket(MESSAGE_LENGTH, 
                    HEADER_LENGTH, receivedData)
            except ValueError:
                # Note: a value error can be thrown for several reasons while
                # parsing a message string into a packet object.  This case,
                # it can only happen when not enough characters were received
                # from the MCU.  It is likely that the MCU is unresponsive to
                # the SYNC message sent and did not respond with the proper
                # ACKN message.
                print('Malformed packet or no packet was received.')
                return False

            # test that received message is an acknowledge message
            ackMessage = SerialPacket.SerialPacket(MESSAGE_LENGTH, 
                HEADER_LENGTH, 'ACKN', '')
            if synackMessage == ackMessage:
                # compose synack message
                synackMessage = SerialPacket.SerialPacket(MESSAGE_LENGTH,
                    HEADER_LENGTH, 'SYNA', '')
                sendData = synackMessage.format()

                # send synack message
                connection.send(sendData)

                # return successful handshake
                return True

            else:
                # return handshake unsuccessful
                return False

        # Check port parameter.
        if not isinstance(port, str): raise TypeError

        # Create new UART Connection on port.
        # print('  ::CONNECTING::  Port ' + port)
        tempConnection = SerialConnection.SerialConnection()

        # Attempt to open port.  If opening is unsuccessful, a
        # serial.SerialException is thrown.
        tempConnection.openPort(port)

        # Attempt handshake with port.  If handshake successful, then create 
        # object.
        if _connect_handshake(tempConnection):
            instance = super().__new__(cls)
            instance.__init__(port)
            instance._connection = tempConnection
            return instance

        # If handshake unsuccessful, return None.
        else:
            return None


    def __init__(self, port):
        # All initialization was performed in __new__().
        pass


    def __del__(self):
        # 

        def _disconnect_handshake(connection):
            #

            print('  ::DISCONNECTING::  Port ' + connection._connection.port)

            # Clear send and receive buffers before trying handshake.
            connection._connection.reset_input_buffer()
            connection._connection.reset_output_buffer()

            # Wait for CTS.
            while self.receive()[0] != 'CTS\0':
                pass

            # Send disconnection command
            self.send('DISC', '')

        # close connection
        _disconnect_handshake(self._connection)
        self._connection.closePort()


    def send(self, commandStr, dataStr):
        # 

        # Test command is of valid type.
        if not isinstance(commandStr, str): raise TypeError

        # Test that data is of valid type.
        if not isinstance(dataStr, str): raise TypeError

        message = SerialPacket.SerialPacket(
            MESSAGE_LENGTH, HEADER_LENGTH, commandStr, dataStr)
        self._connection.send(message.format())
        

    def receive(self):
        # 

        # Receive message from MCU.
        tempMessage = self._connection.receive(MESSAGE_LENGTH)

        # Return message parsed into command and data segments.
        return tempMessage[:HEADER_LENGTH], tempMessage[HEADER_LENGTH:]


    def receive_raw_noNull_noWhitespace(self):
        # 

        # Receive message from MCU.
        tempMessage = self._connection.receive(MESSAGE_LENGTH)

        # Return message parsed into command and data segments.
        return tempMessage.replace('\0', '\\0').replace('\t', '\\t')\
        .replace(' ', '\\ ').replace('\n', '\\n')\
        .replace('\f', '\\f').replace('\r', '\\r')\
        .replace('\v', '\\v')
