import sys
import re
from datetime import datetime, timedelta, timezone
from enum import Enum


class EVENT_TYPE(Enum):
    STANDBY_MODE = 0
    BEACON_MODE = 1


def loadCalendar(filePath):
    # Read the file specified at the file fath provided.
    # returns a string of the file data read, or None if error.

    # check if file name provided is an ics file. Must be an ics file.
    # if not, return None and report.
    if filePath[-4:] != '.ics':
        # validate that user passed in an .ics file
        print('Non-ICS file passed as parameter!')
        return None

    # open file for reading
    with open(filePath, 'r') as file:
        # open file and read
        fileData = file.read()

    # if file could not be opened or is empty, return None and report.
    if fileData == None:
        # file was not there
        print('File was not available.')
        return None
    elif len(fileData) == 0:
        # if file was empty
        print('File was empty.')
        return None

    # file opened, return string of file data read.
    else:
        # if file was read
        # return file data
        return fileData


def parseCalendarFromICS(fileStr):
    # Parse string read from ics file.
    # Returns a Calendar object generated from the ics file string or None if error parsing.
    #
    # Arguments: fileStr - String

    # check that a string is passed in.
    # if not, report and return None.
    if not isinstance(fileStr, str):
        print(":::parseCalendarFromICS - fileStr is not a string")
        return None

    # string provided, proceed to parse string into Calendar object.
    # return result from parsing process.
    # could return Calendar object or None if parsing failed.
    else:
        return _parseICS(fileStr)


def _parseBeginEnd(str, tag):
    # extract the string segment that matches between the BEGIN and END tags specified, including the tags.
    # example:  _parseBeginEnd(string, 'VCALENDAR') -> BEGIN:VCALENDAR [...] END:VCALENDAR
    # returns: a match object with three groups: start, body, and end. Body contains the body of the section
    #       parsed and is of most insterest to the programmer
    #
    # Arguments: str - String
    #            tag - String
    #
    # Note: Non-greedy matching used. When searching for multiple begin-end blocks, only the first will be
    #       matched.
    return re.search(rf"(?P<start>BEGIN:{tag})(?P<body>[\s\S]*?(?=END:{tag}))(?P<end>END:{tag})", str)


def _parseRule(str, tag, sep=':', term='\n'):
    # extract the string segment that matches the rule line specified.
    # example: _parseRule(string, 'VERSION') -> VERSION:2.0
    # returns: a match object with two groups: tag and rule. Rule contains the rule information and is of most
    #       interest to the programmer
    #
    # Arguments: str - String
    #            tag - String
    #            sep - String
    return re.search(rf"(?P<tag>{tag}{sep})(?P<rule>.+?(?={term}))", str)


def _parseDateTime(dt, stdTime, daylightTime):
    #
    #
    timezoneHourOffset = int(_parseRule(stdTime, 'TZOFFSETTO').group('rule')[0:3])
    timezoneMinuteOffset = int(_parseRule(stdTime, 'TZOFFSETTO').group('rule')[3:])
    calTimezone = timezone(offset=timedelta(hours=timezoneHourOffset, minutes=timezoneMinuteOffset))

    date = re.search(r"(?P<year>[0-9]{4})(?P<month>[0-9]{2})(?P<day>[0-9]{2})", dt)
    date_fullyear = int(date.group('year'))
    date_month = int(date.group('month'))
    date_mday = int(date.group('day'))
    time = re.search(r"T(?P<hour>[0-9]{2})(?P<minute>[0-9]{2})(?P<second>[0-9]{2})(?P<utc>Z)?", dt)
    time_hour = int(time.group('hour') if time is not None else '0')
    time_minute = int(time.group('minute') if time is not None else '0')
    time_second = int(time.group('second') if time is not None else '0')
    time_utc = (True if time.group('utc') is not None else False) if time is not None else None
    # return datetime(date_fullyear, date_month, date_mday, time_hour, time_minute, time_second, tzinfo=calTimezone)
    return datetime(date_fullyear, date_month, date_mday, time_hour, time_minute, time_second, tzinfo=timezone(offset=timedelta(hours=0, minutes=0)))


def _parseICS(fileStr):
    # Parse the ics file string provided into a Calendar object.
    # at top most level, an ics calendar is defined as a body between begin and end tags:
    #
    # Arguments: fileStr - String

    # get calendar body
    groupedFile = _parseBeginEnd(fileStr, 'VCALENDAR')
    icalbody = groupedFile.group('body')

    # get name for calendar
    icalNameRule = _parseRule(icalbody, 'X-WR-CALNAME')
    icalName = icalNameRule.group('rule') if icalNameRule is not None else 'Calendar' # handle in case X-WR-CALNAME is not standard for all ics

    # get timezone information for standardizing to UTC
    icalDaylightTime = _parseBeginEnd(icalbody, 'DAYLIGHT').group('body')
    icalStandardTime = _parseBeginEnd(icalbody, 'STANDARD').group('body')

    # parse events
    eventcBlock_list = list()
    endIdx = 0
    while (True):
        # loop and get all VEVENT blocks
        eventcBlock = _parseBeginEnd(icalbody[endIdx:], 'VEVENT')
        if eventcBlock is not None:
            eventcBlock_list.append(eventcBlock.group('body'))
            endIdx = eventcBlock.end() + endIdx
        else:
            break

    # parse each event
    events = list()
    for block in eventcBlock_list:
        # name
        name = _parseRule(block, 'SUMMARY').group('rule')

        # set event type
        temp_event = 'event'

        # start time of event
        dtstart = _parseRule(block, 'DTSTART', sep='').group('rule')
        temp_startDateTime = _parseDateTime(dtstart, icalStandardTime, icalDaylightTime).astimezone(timezone(offset=timedelta()))

        # end time of event
        dtend = _parseRule(block, 'DTEND', sep='').group('rule')
        temp_endDateTime = _parseDateTime(dtend, icalStandardTime, icalDaylightTime).astimezone(timezone(offset=timedelta()))

        # repeat rule
        # this is left in for future development with repeat rule
        rrule = _parseRule(block, 'RRULE')
        if rrule is not None:
            # if there is a repeat rule
            # parse into recurrence object

            # parse rule string
            rule = rrule.group('rule')
            freqRule = _parseRule(rule, 'FREQ', sep='=', term=';')
            untilRule = _parseRule(rule, 'UNTIL', sep='=', term=';')
            countRule = _parseRule(rule, 'COUNT', sep='=', term=';')
            intervalRule = _parseRule(rule, 'INTERVAL', sep='=', term=';')
            bysecondRule = _parseRule(rule, 'BYSECOND', sep='=', term=';')
            byminuteRule = _parseRule(rule, 'BYMINUTE', sep='=', term=';')
            byhourRule = _parseRule(rule, 'BYHOUR', sep='=', term=';')
            bydayRule = _parseRule(rule, 'BYDAY', sep='=', term=';')
            bymonthdayRule = _parseRule(rule, 'BYMONTHDAY', sep='=', term=';')
            byyearRule = _parseRule(rule, 'BYYEARDAY', sep='=', term=';')
            byweeknoRule = _parseRule(rule, 'BYWEEKNO', sep='=', term=';')
            bymonthRule = _parseRule(rule, 'BYMONTH', sep='=', term=';')
            bysetposRule = _parseRule(rule, 'BYSETPOS', sep='=', term=';')
            wkstRule = _parseRule(rule, 'WKST', sep='=', term=';')

            # store into variables for passing to recurrence object
            freq = freqRule.group('rule') if freqRule is not None else None
            until = _parseDateTime(untilRule.group('rule'), icalStandardTime, icalDaylightTime).astimezone(timezone(offset=timedelta())) if untilRule is not None else None
            count = countRule.group('rule') if countRule is not None else None
            interval = intervalRule.group('rule') if intervalRule is not None else None
            bysecond = bysecondRule.group('rule') if bysecondRule is not None else None
            byminute = byminuteRule.group('rule') if byminuteRule is not None else None
            byhour = byhourRule.group('rule') if byhourRule is not None else None
            byday = bydayRule.group('rule') if bydayRule is not None else None
            bymonthday = bymonthdayRule.group('rule') if bymonthdayRule is not None else None
            byyear =  byyearRule.group('rule') if byyearRule is not None else None
            byweekno = byweeknoRule.group('rule') if byweeknoRule is not None else None
            bymonth = bymonthRule.group('rule') if bymonthRule is not None else None
            bysetpos = bysetposRule.group('rule') if bysetposRule is not None else None
            wkst = wkstRule.group('rule') if wkstRule is not None else None

            # build repeat rule here
            pass

        else:
            # no recurrence rule
            # build empty repeat rule here
            pass

        # add event to calendar
        events.append(AirAlarmEvent(event=temp_event, name=name, startDateTime=temp_startDateTime, endDateTime=temp_endDateTime))

    # return calendar object
    return events


class AirAlarmEvent:
    # AirAlarmCalendar represents the information needed from an ics for programming alarms on the STM32WL55JC
    # MCU. This is not representative of all of the features provided by the ics file format, but only those
    # needed for the aformentioned application of this code.
    #
    # A few simplifying restrictions are made for this application:
    #   1. All time is standardized to UTC to prevent dealing with tricky timezone and day light savings on
    #      the AIR Tag.
    #   2. No repeats in events (for now). The AIR Tag only accepts single events.
    #
    # Information that is necessary for the programming of alarms in this application are:
    #   1. Event start date and time
    #   2. Event end date and time
    #   3.

    def __init__(self, event, name, startDateTime, endDateTime):
        self.event = event
        self.start = startDateTime
        self.end = endDateTime
        self.name = name


    def export(self):
        return (str(self.start.strftime("%y;%m;%d;%H;%M;%S")) + ';' + str(self.end.strftime("%y;%m;%d;%H;%M;%S")))


    def __str__(self):
        return ('Event: ' + self.event + '\n\tStart: ' + self.start.strftime("%d/%m/%Y %H:%M:%S") + '\n\t  End: ' + self.end.strftime("%d/%m/%Y %H:%M:%S"))


if __name__ == '__main__':
    # parse ics file into calendar
    icsFilePath = sys.argv[1]
    fileData = loadCalendar(icsFilePath)
    if fileData is not None:
        event_list = parseCalendarFromICS(fileData)
        print([exported.export() for exported in event_list])

    exit(0)
