import IcsEvents
import sys


def checkOverlaps(eventsList):
	# assumes that eventsList is in monotonic ordering by start date/time

	# list of events that are overlapping
	overlapEvents = list()

	# check that eventsList is a list of AirAlarmEvents
	if not isinstance(eventsList, list): raise TypeError
	for event in event_list:
		if not isinstance(event, IcsEvents.AirAlarmEvent): raise TypeError

	# loop over elements in the list and validate
	for index, event in enumerate(eventsList):
		# if not the last event in the list
		if index != len(eventsList) - 1:
			# terminate if the end time is not before or ar the start time
			# of the next event
			if event.end > eventsList[index + 1].start:
				overlapEvents.append(event)
				overlapEvents.append(eventsList[index + 1])

	# return overlapped events
	return overlapEvents


if __name__ == '__main__':
	# test checkOverlap
	icsFilePath = sys.argv[1]
	fileData = IcsEvents.loadCalendar(icsFilePath)
	if fileData is not None:
		event_list = IcsEvents.parseCalendarFromICS(fileData)
		print([exported.export() for exported in event_list])
		overlapList = checkOverlaps(event_list)
		if len(overlapList) > 0:
			print('Overlaps found!  ' + str([event.name for event in overlapList]))
	else:
		print('file not parsed!')