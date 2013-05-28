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
#include "log.h"
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

// Function Prototypes
int ConnectProfileToDevice(int deviceNum);
int FindProfileForDevice(int deviceNum, int preferred, int * found);


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


int GetFreeProfile(void)
{
	// Check for too many, return -1 if so
	if (numberOfProfiles == MAX_PROFILES)
		return -1;

	int profileNum = numberOfProfiles;
	numberOfProfiles++;
	return profileNum;
}


void DeleteProfile(int profileToDelete)
{
	// Sanity check
	if (profileToDelete >= numberOfProfiles)
		return;

	// Trivial case: Profile at end of the array
	if (profileToDelete == (numberOfProfiles - 1))
	{
		numberOfProfiles--;
		return;
	}

//	memmove(dest, src, bytesToMove);
	memmove(&profile[profileToDelete], &profile[profileToDelete + 1], ((numberOfProfiles - 1) - profileToDelete) * sizeof(Profile));
	numberOfProfiles--;
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
//This should *never* be the case--all profiles in list are *good*
//		if (profile[i].device == -1)
//			continue;

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


int FindUsableProfiles(QComboBox * combo)
{
	int found = 0;

	// Check for device #0 (keyboard) profiles first
	for(int j=0; j<numberOfProfiles; j++)
	{
		// Check for device *and* usable configuration
		if ((profile[j].device == 0) && (profile[j].preferredController))
		{
			combo->addItem(QString("Keyboard::%1").arg(profile[j].mapName), j);
			found++;
		}
	}

	// Check for connected host devices next
	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));

		for(int j=0; j<numberOfProfiles; j++)
		{
			if ((profile[j].device == deviceNum) && (profile[j].preferredController))
			{
				combo->addItem(QString("%1::%2").arg(Gamepad::GetJoystickName(i)).arg(profile[j].mapName), j);
				found++;
			}
		}
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

	WriteLog("PROFILE: Successfully mapped device '%s' (%s) to controller #%u...\n", deviceNames[profile[profileNum].device], profile[profileNum].mapName, controllerNum);
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
Here's the rules: If preferred Jaguar controller is not checked, the profile is
skipped. If one or the other is checked, it's put into that slot. If *both* are
checked, it will take over any slot that isn't claimed by another gamepad. If
there are ties, present it to the user *once* and ask them which gamepad should
be #1; don't ask again unless a), they change the profiles and b), the
situation warrants it.

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


Steps:

1) Make a list of all devices attached to the system.

2) Make a list of all profiles belonging to those devices, as long as they have
   one or more Jaguar controllers that are "mapped to".

3) See if there are any conflicts. If there are, see if the user has already
   been asked to resolve and chosen a resolution; otherwise, ask the user to
   resolve.

   a) Loop through all found profiles. If they are set to a single controller,
      set it in the appropriate list (one list for controller 1, another for
      controller 2).

   b) Loop through all found profiles. If they are set to both controllers,
      ... (first stab at it:)
      Check for list #1. If nothing there, assign it to list #1.
      Else, check for List #2. If nothing there, assign to list #2.
      [But the wording of it implies that it will assign it to both.
       Does that mean we should make another combobox will all the possible
       combinations laid out? Probably. Not many people will understand that
       checking both means "assign to either one that's free".]

4) Connect profiles to controllers, and set gamepad slots (for the MainWin
   handler).

*/
void AutoConnectProfiles(void)
{
	int foundProfiles[MAX_PROFILES];
	int controller1Profile = -1;
	int controller2Profile = -1;

	// Check for Keyboard device profiles first, if anything else is plugged in
	// it will default to it instead.
#if 0
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
#else
	// Connect keyboard devices first...
	ConnectProfileToDevice(0);
#endif

	// Next, check for the "don't care" condition of both jaguar controllers
	// checked for connected host devices
	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));
		int numberProfilesFound = FindProfileForDevice(deviceNum, CONTROLLER1 | CONTROLLER2, foundProfiles);

		// We need to grab pairs here, host device # paired up with profiles
		// so we can then determine if there are any conflicts that can't be
		// resolved...
	}

	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));

#if 0
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
#else
		ConnectProfileToDevice(deviceNum);
#endif
	}

	if (controller1Profile != -1)
		ConnectProfileToController(controller1Profile, 0);

	if (controller2Profile != -1)
		ConnectProfileToController(controller2Profile, 1);
}


int ConnectProfileToDevice(int deviceNum)
{
//	bool found1 = false;
//	bool found2 = false;
	int numberFoundForController1 = 0;
	int numberFoundForController2 = 0;

	for(int i=0; i<numberOfProfiles; i++)
	{
		// Skip profile if it's not our device
		if (profile[i].device != deviceNum)
			continue;

		if (profile[i].preferredController & CONTROLLER1)
		{
			controller1Profile = i;
//			found1 = true;
			numberFoundForController1++;
		}

		if (profile[i].preferredController & CONTROLLER2)
		{
			controller2Profile = i;
//			found2 = true;
			numberFoundForController2++;
		}
	}

//	return found;
	return numberFoundForController1 + numberFoundForController2;
}

/*
int FindProfileForDevice(int deviceNum)
{
	for(int i=0; i<numberOfProfiles; i++)
	{
		// Skip profile if it's not our device
		if (profile[i].device != deviceNum)
			continue;

		return i;
	}

	return -1;
}
*/

int FindProfileForDevice(int deviceNum, int preferred, int * found)
{
	int numFound = 0;

	for(int i=0; i<numberOfProfiles; i++)
	{
		// Return the profile only if it matches the passed in device and
		// matches the passed in prefence...
		if ((profile[i].device == deviceNum) && (profile[i].preferredController == preferred))
			found[numFound++] = i;
	}

	return numFound;
}

