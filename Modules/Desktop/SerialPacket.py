# Author: Kevin Imlay

# Defines the character to postfix a packet's body segment with.
EMPTY_CHAR = '\0'


# Messages for exceptions.
MISCOUTED_PARAMETERS_MSG = '''An invalid number of parameters 
were passed into the creation of a SerialPacket object.  The number of 
parameters must be 3 or 4, but received {}.'''


class MiscountedParameters(Exception):
    # Exception for when an invalid number of parameters are passed for the
    # creation of a SerialPacket object.

    def __init__(self, errMessage):
        # Create exception with error message
        super().__init__(self, errMessage)


class SerialPacket:
    # A UART Packet encapsulates the necessary parameters for building a
    # packet to be sent to the MCU over UART communication running the
    # Desktop Application UART Communicator module.
    #
    # A packet is formatted simply as a header of N characters contatenated
    # with a body of M characters.  The total packet length is O, and the
    # MCU expects packets of a fixed size so there is the additional
    # constraint that N+M = O, where O is the expected packet length.  To
    # achieve this, the body must not exceed O-N characters, and the body
    # is postfixed with null characters if it does not consume the full
    # O-N length.

    # Packet parameters.
    # Expected length of the packet
    _packetLength = None

    # Packet contents.
    # Text of the header segment
    _headerText = None
    # Text of the body segment
    _bodyText = None


    def __new__(cls, packetLength, headerLength, *args):
        # Testing of parameters before creation of object.  This serves to
        # a) Ensure parameters are valid, and
        # b) decide if the object is being created from separate header and
        #    body string or one string containing both.
        #
        # Parameters must start with, in order:
        # 1) packet length,
        # 2) header length,
        # and if parsing from a message string:
        # 3) packet string,
        # or if creating from separate header and body strings:
        # 3) header string, and
        # 4) body string.

        # Ensure packetLength and headerLength parameters are valid.
        if not isinstance(headerLength, int): raise TypeError
        if not isinstance(packetLength, int): raise TypeError
        if packetLength < 1: raise ValueError
        if headerLength > packetLength: raise ValueError

        # Switch on the number of args.  If on, then parsing from message 
        # string.  If two, then making from separate header and body 
        # parameters.  If neither, throw MiscountedParameters exception.
        
        # Parsing from message string.
        if len(args) == 1:
            # Alias parameters.
            packetString = args[0]

            # Check parameters for parsing from one message string.
            if not isinstance(packetString, str): raise TypeError
            if len(packetString) != packetLength: raise ValueError

            # Create SerialPacket object.
            instance = super().__new__(cls)
            instance.__init__(None)

            # Parse message string and populate object fields.
            instance._packetLength = packetLength
            instance._headerText = packetString[0:headerLength]
            instance._bodyText = packetString[headerLength:].split(EMPTY_CHAR)[0]

        # Creation from parameters.
        elif len(args) == 2:
            # Alias parameters.
            headerText = args[0]
            bodyText = args[1]

            # Check parameters for creation from separate header and body
            # strings.
            if not isinstance(headerText, str): raise TypeError
            if not isinstance(bodyText, str): raise TypeError
            if len(headerText) != headerLength: raise ValueError
            if len(bodyText) > packetLength - headerLength: raise ValueError

            # Create SerialPacket object.
            instance = super().__new__(cls)
            instance.__init__(None)

            # Populate object fields
            instance._packetLength = packetLength
            instance._headerText = headerText
            instance._bodyText = bodyText

        # Incorrect number of parameters.
        else:
            raise MiscountedParameters(
                MISCOUTED_PARAMETERS_MSG.format(len(args) + 2))

        # Finally, return object created.
        return instance


    def __init__(self, *args):
        # Initialize a new packet object.  
        # All creation and population of fields is handled in __new__()
        pass


    def format(self):
        # Formats the packet into one string from the header and body texts.
        # Padds the body segment until the packet length is reached.

        # add header and body
        formatStr = self._headerText + self._bodyText

        # postfix null characters if needed
        remainingCount = self._packetLength - len(formatStr)
        for _ in range(remainingCount):
            formatStr += EMPTY_CHAR

        # return formatted string
        return formatStr


    def __str__(self):
        # Helpful definition of how to print this object.

        # Form string for fields of packet.
        packetNameString = str(id(self))
        packetLengthString = "Packet Length: " + str(self._packetLength)
        HeaderLengthString = "Header Length: " + str(len(self._headerText))
        BodyLengthString = "Body Length: " + str(len(self._bodyText))
        headerTextString = "Header Text: " + self._headerText
        bodyTextString = "Body Text: " + self._bodyText

        # Format into one string for printing.
        formattedString = "Packet " + packetNameString + '\n' \
        + '\t' + packetLengthString + '\n' \
        + '\t' + HeaderLengthString + '\n' \
        + '\t' + BodyLengthString + '\n' \
        + '\t' + headerTextString + '\n' \
        + '\t' + bodyTextString

        # return formatted string of object fields.
        return formattedString


    def __eq__(self, other):
        # Helpful definition for comparing two packets.
        # Compares each field between two packets. If all fields are
        # equivalent, then the objects are equivalent.

        # make comparisons and return result.
        return self._packetLength == other._packetLength \
        and self._headerText == other._headerText \
            and self._bodyText == other._bodyText
