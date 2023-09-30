# Author: Kevin Imlay

import SerialProtocol
import queue

# Define session parameters.
NUM_HANDSHAKE_ATTEMTPS = 3

class STM32SerialCom:
	# STM32 Serial Communication maps actions on the application level to
	# messages passed between the MCU and the desktop application.

	# class fields
	_connection = None
	_inMessageQueue = queue.Queue(maxsize = 0)
	_outMessageQueue = queue.Queue(maxsize = 0)


	def __new__(cls, port):
		# Attempt to open connection on port.
		tempStm32McuConnection = None
		for attempt_num in range(1, NUM_HANDSHAKE_ATTEMTPS + 1):
			tempStm32McuConnection = SerialProtocol.SerialProtocol(port)
			if tempStm32McuConnection is not None:
				break

		# Check if connection was opened.
		if tempStm32McuConnection is not None:
			instance = super().__new__(cls)
			instance.__init__(port)
			instance._connection = tempStm32McuConnection
			return instance
		else:
			return None


	def __init__(self, port):
		# All initialization was performed in __new__().
		pass


	def __del__(self):
		# Deleting connection object will perform disconnection handshake
		# and close the connection.
		del self._connection

	def update(self):
		# Empty any received messages into the inMessageQueue to process.
		# This will disreguard any CTS messages sent while the desktop
		# application was not in a state to send anything, will store non-CTS
		# messages for later processing, and free the read buffer to wait for
		# the next CTS message if any messages need to be sent.
		while self._connection._connection._connection.in_waiting > 0:
			tempInMessage = self._connection.receive()
			if tempInMessage[0] != 'CTS\0':
				self._inMessageQueue.put(tempInMessage)

		# While there are messages to be sent to the MCU, wait for a CTS
		# message and send one message.  If any non-CTS messages are received
		# while sending, they will be queued for later processing.
		while not self._outMessageQueue.empty():
			while True:
				tempInMessage = self._connection.receive()
				if tempInMessage[0] != 'CTS\0':
					self._inMessageQueue.put(tempInMessage)
				else:
					break
			tempOutMessage = self._outMessageQueue.get()
			print('  ::SENDING::  ' + tempOutMessage[0] + tempOutMessage[1])
			self._connection.send(tempOutMessage[0], tempOutMessage[1])

	def setMcuTime():
		pass