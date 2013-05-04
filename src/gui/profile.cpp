//
// profile.cpp - Global profile storage/definition/manipulation
//
// by James Hammons
// (C) 2013 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  05/01/2013  Created this file
//


#include "profile.h"
#include <QtGui>
#include "gamepad.h"
#include "settings.h"


#define MAX_DEVICES  64


Profile profile[MAX_PROFILES];
int controller1Profile;
int controller2Profile;
int gamepad1Slot;
int gamepad2Slot;
int numberOfProfiles;
int numberOfDevices;
char deviceNames[MAX_DEVICES][128];

// This is so that new devices have something reasonable to show for default
uint32_t defaultMap[21] = {
	'S', 'X', 'Z', 'C', '-','7', '4', '1', '0', '8', '5', '2', '=', '9', '6',
	'3', 'L', 'K', 'J', 'O', 'P'
};


void ReadProfiles(QSettings * set)
{
	// Assume no profiles, until we read them
	numberOfProfiles = 0;

	// There is always at least one device present, and it's the keyboard
	// (hey, we're PC centric here ;-)
	numberOfDevices = 1;
	strcpy(deviceNames[0], "Keyboard");

	// Read the rest of the devices (if any)
	numberOfDevices += set->beginReadArray("devices");

	for(int i=1; i<numberOfDevices; i++)
	{
		set->setArrayIndex(i - 1);
		strcpy(deviceNames[i], set->value("deviceName").toString().toAscii().data());
//printf("Read device name: %s\n", deviceNames[i]);
	}

	set->endArray();
	numberOfProfiles = set->beginReadArray("profiles");
//printf("Number of profiles: %u\n", numberOfProfiles);

	for(int i=0; i<numberOfProfiles; i++)
	{
		set->setArrayIndex(i);
		profile[i].device = set->value("deviceNum").toInt();
		strcpy(profile[i].mapName, set->value("mapName").toString().toAscii().data());
		profile[i].preferredController = set->value("preferredController").toInt();

		for(int j=0; j<21; j++)
		{
			QString string = QString("map%1").arg(j);
			profile[i].map[j] = set->value(string).toInt();
		}
//printf("Profile #%u: device=%u (%s)\n", i, profile[i].device, deviceNames[profile[i].device]);
	}

	set->endArray();

//printf("Number of profiles found: %u\n", numberOfProfiles);
	// Set up a reasonable default if no profiles were found
	if (numberOfProfiles == 0)
	{
//printf("Setting up default profile...\n");
		numberOfProfiles++;
		profile[0].device = 0;	// Keyboard is always device #0
		strcpy(profile[0].mapName, "Default");
		profile[0].preferredController = CONTROLLER1;

		for(int i=0; i<21; i++)
			profile[0].map[i] = defaultMap[i];
	}
}


void WriteProfiles(QSettings * set)
{
#if 0
	// Don't write anything for now...
	return;
#endif
	// NB: Should only do this if something changed; otherwise, no need to do
	//     this.
	set->beginWriteArray("devices");

	for(int i=1; i<numberOfDevices; i++)
	{
		set->setArrayIndex(i - 1);
		set->setValue("deviceName", deviceNames[i]);
	}

	set->endArray();
	set->beginWriteArray("profiles");

	for(int i=0; i<numberOfProfiles; i++)
	{
		set->setArrayIndex(i);
		set->setValue("deviceNum", profile[i].device);
		set->setValue("mapName", profile[i].mapName);
		set->setValue("preferredController", profile[i].preferredController);

		for(int j=0; j<21; j++)
		{
			QString string = QString("map%1").arg(j);
			set->setValue(string, profile[i].map[j]);
		}
	}

	set->endArray();
}


int FindDeviceNumberForName(const char * name)
{
	for(int i=0; i<numberOfDevices; i++)
	{
		if (strcmp(deviceNames[i], name) == 0)
			return i;
	}

	if (numberOfDevices == MAX_DEVICES)
		return -1;

	// If the device wasn't found, it must be new; so add it to the list.
	int deviceNum = numberOfDevices;
	deviceNames[deviceNum][127] = 0;
	strncpy(deviceNames[deviceNum], name, 127);
	numberOfDevices++;

	return deviceNum;
}


int FindMappingsForDevice(int deviceNum, QComboBox * combo)
{
	int found = 0;

	for(int i=0; i<numberOfProfiles; i++)
	{
		if (profile[i].device == -1)
			continue;

		if (profile[i].device == deviceNum)
		{
			combo->addItem(profile[i].mapName, i);
			found++;
		}
	}

	// If no mappings were found, create a default one for it
	if (found == 0)
	{
		profile[numberOfProfiles].device = deviceNum;
		strcpy(profile[numberOfProfiles].mapName, "Default");
		profile[numberOfProfiles].preferredController = CONTROLLER1;

		for(int i=0; i<21; i++)
			profile[numberOfProfiles].map[i] = defaultMap[i];

		combo->addItem(profile[numberOfProfiles].mapName, numberOfProfiles);
		numberOfProfiles++;
		found++;
	}

	return found;
}


bool ConnectProfileToController(int profileNum, int controllerNum)
{
	if (profile[profileNum].device == -1)
		return false;

	if (controllerNum < 0 || controllerNum > 2)
		return false;

	uint32_t * dest = (controllerNum == 0 ? &vjs.p1KeyBindings[0] : &vjs.p2KeyBindings[0]);

	for(int i=0; i<21; i++)
		dest[i] = profile[profileNum].map[i];

printf("Successfully mapped device '%s' (%s) to controller #%u...\n", deviceNames[profile[profileNum].device], profile[profileNum].mapName, controllerNum);
	return true;
}


//
// This is a pretty crappy way of doing autodetection. What it does is scan for
// keyboard profiles first, then look for plugged in gamepads next. If more
// than one plugged in gamepad matches a preferred controller slot, the last
// one found is chosen.
//
// There has to be a better way to do this, I just can't think of what it
// should be ATM... :-P
//
/*
Also, there is a problem with this approach and having multiple devices
that are the same. Currently, if two of the same device are plugged in
and the profile is set to both controllers, it will broadcast buttons
pressed from either gamepad, no matter who is pressing them. This is
BAD(tm). [Not true, but there's a different problem described under 'How to
solve?', so GOOD(tm).]

Also, the gamepad logic doesn't distinguish inputs by controller, it just
grabs them all regardless. This is also BAD(tm). [Actually, it doesn't. It
properly segregates the inputs. So this is GOOD(tm).]

How to solve?

Seems there's yet ANOTHER dimension to all this: The physical gamepads
plugged into their ports. Now the device # can map these fine if they're
different, but we still run into problems with the handling in the MainWin
because it's hardwired to take pad 0 in slot 0 and pad 1 in slot 1. If you have
them configured other than this, you won't get anything. So we need to also
map the physical devices to their respective slots.
*/
void AutoConnectProfiles(void)
{
	int controller1Profile = -1;
	int controller2Profile = -1;

	// Nothing plugged in, we fall back to the default keyboard device profiles
//	if (Gamepad::numJoysticks == 0)
	{
// Check for Keyboard device first, if anything else is plugged in it will
// default to it instead
		for(int i=0; i<numberOfProfiles; i++)
		{
			// Skip profile if it's not Keyboard device
			if (profile[i].device != 0)
				continue;

			if (profile[i].preferredController & CONTROLLER1)
				controller1Profile = i;

			if (profile[i].preferredController & CONTROLLER2)
				controller2Profile = i;
		}
	}
//	else
	{
//printf("Number of gamepads found: %u\n", Gamepad::numJoysticks);
		for(int i=0; i<Gamepad::numJoysticks; i++)
		{
			int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));
//printf("Attempting to find valid gamepad profile. Device=%u\n", deviceNum);

			for(int j=0; j<numberOfProfiles; j++)
			{
				// Skip profile if it's not discovered device
				if (profile[j].device != deviceNum)
					continue;

				if (profile[j].preferredController & CONTROLLER1)
					controller1Profile = j;

				if (profile[j].preferredController & CONTROLLER2)
					controller2Profile = j;
			}
		}
	}

	if (controller1Profile != -1)
		ConnectProfileToController(controller1Profile, 0);

	if (controller2Profile != -1)
		ConnectProfileToController(controller2Profile, 1);
}

