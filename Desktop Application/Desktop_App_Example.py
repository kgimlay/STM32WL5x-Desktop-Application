# Author: Kevin Imlay

import SerialSession
import platform
import os
import re
import time
import random
from datetime import datetime, timedelta
from IcsEvents import loadCalendar, parseCalendarFromICS
import sys


def getPorts():
    # get a list of the ports on the machine

    # get the OS that this code is running on
    operatingSystem = platform.system()
    operatingSystemVersion = platform.version()

    # test that OS is Linux or OSX
    # I don't have Windows machien to test this code on...  so no Windows.
    # If you want Windows compatability, add implementation.
    assert operatingSystem == 'Darwin' or operatingSystem == 'Linux', \
        'Only Darwin or Linus operating systems supported.'

    # list of serial ports to return
    serialPorts = None

    # MAC OS (only tested with Ventura 13.2.1)
    if operatingSystem == 'Darwin':
        # list all device ports
        devicePorts = ['/dev/' + port for port in os.listdir('/dev/')]
        # remove all device ports that are not tty or cu ports
        serialPorts = [port for port in devicePorts if re.search(r'tty\.usb', port)]

    # Linux OS (Ubuntu) (only tested with Ubuntu Server 20.04.5 LTS 64-bit on Raspberry Pi 4)
    elif operatingSystem == 'Linux' and 'Ubuntu' in operatingSystemVersion:
        # list all device ports
        devicePorts = ['/dev/' + port for port in os.listdir('/dev/')]
        # remove all device ports that are not tty or cu ports
        serialPorts = [port for port in devicePorts if re.search(r'ttyACM', port)]

    # return list of serial ports
    return serialPorts


if __name__ == '__main__':
    # ---------------------------------------
    # ---------- Application Setup ----------
    # ---------------------------------------

    # get a list of the ports available on the machine
    testPorts = getPorts()

    # test if there are no ports
    if len(testPorts) == 0:
        print('There are no serial ports available on the machine.')
        exit(0)

    # There are available ports, try to handshake with each one until one
    # successfully handshakes.
    connectionFlag = False
    for availablePort in testPorts:
            # attempt to make connection
            Stm32Session = SerialSession.STM32SerialCom(availablePort)

            # if the connection was successful, print message
            if Stm32Session is not None:
                # report connection
                print('Connected to port {}'.format(availablePort))
                # and set flag to enter application loop
                connectionFlag = True
                # and exit loop
                break

            # if the message was unsuccessful, print message
            else:
                print('Connection could not be made with port {}'.format(availablePort))

    # If a connection was not established, report.
    if not connectionFlag:
        print('No connection could be established with MCU.')
    
    # Else, a connection was established, enter application loop.
    else:
        try:
            # --------------------------------------
            # ---------- Application Loop ----------
            # --------------------------------------
            # while True:
            #     time.sleep(1)
            #     for _ in range(10):
            #         Stm32Session._outMessageQueue.put(('ECHO','Hello World! {}'.format(random.randrange(200))))
            #     Stm32Session.update()
            #     while not Stm32Session._inMessageQueue.empty():
            #         print(Stm32Session._inMessageQueue.get()[1])

            # set the date and time on the MCU
            print('Setting MCU time...')
            Stm32Session._outMessageQueue.put(('STDT', datetime.now().strftime('%y;%m;%d;%H;%M;%S')))
            Stm32Session._outMessageQueue.put(('GTDT',''))
            while Stm32Session._inMessageQueue.empty():
                Stm32Session.update()
            mcuMessage = Stm32Session._inMessageQueue.get()
            print('The MCU\'s time is now:  ' + mcuMessage[1])
            sys.stdout.flush()

            # if user does not specify an ics file
            if len(sys.argv) == 1:
                # upload set of simple events
                now = datetime.now()
                for i in range(10, 60, 8):
                    eventStart = now + timedelta(seconds=i)
                    eventEnd = eventStart + timedelta(seconds=4)
                    messageStr = eventStart.strftime('%y;%m;%d;%H;%M;%S') + ';' + eventEnd.strftime('%y;%m;%d;%H;%M;%S')
                    Stm32Session._outMessageQueue.put(('AEVT', messageStr))
                Stm32Session._outMessageQueue.put(('SCAL', ''))
                Stm32Session.update()

            # if the user specified an ics file
            elif len(sys.argv) == 2:
                # parse some events from an ics file
                print('Uploading from test ics')
                fileData = loadCalendar(sys.argv[1])
                if fileData is not None:
                    event_list = parseCalendarFromICS(fileData)
                    uploadList = [event.export() for event in event_list]
                    for event in uploadList:
                        Stm32Session._outMessageQueue.put(('AEVT',event))
                    Stm32Session._outMessageQueue.put(('SCAL', ''))
                    Stm32Session.update()

            # flush the serial buffer before exiting
            sys.stdout.flush()

        # Handle when a keyboard interrupt occurs, to make things tidy.
        except KeyboardInterrupt as e:
            print('\n\nUser terminated program.')

    # -----------------------------------------
    # ---------- Application Cleanup ----------
    # -----------------------------------------
    
    # disconnect
    if Stm32Session is not None:
        del Stm32Session
        # and report disconnection
        print('Disconnected from port {}'.format(availablePort))
