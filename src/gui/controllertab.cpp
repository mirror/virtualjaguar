//
// controllertab.cpp: "Controller" tab on the config dialog
//
// Part of the Virtual Jaguar Project
// (C) 2011 Underground Software
// See the README and GPLv3 files for licensing and warranty information
//
// JLH = James Hammons <jlhamm@acm.org>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JLH  06/23/2011  Created this file
// JLH  07/20/2011  Fixed a bunch of stuff
//

#include "controllertab.h"

#include "controllerwidget.h"
#include "gamepad.h"
#include "joystick.h"
#include "keygrabber.h"
#include "profile.h"


ControllerTab::ControllerTab(QWidget * parent/*= 0*/): QWidget(parent),
	label1(new QLabel(tr("Host Device:"))),
	label2(new QLabel(tr("Map Name:"))),
	label3(new QLabel(tr("Maps to:"))),
	deviceList(new QComboBox(this)),
	mapNameList(new QComboBox(this)),
	controller1(new QCheckBox(tr("Jaguar Controller #1"))),
	controller2(new QCheckBox(tr("Jaguar Controller #2"))),
	addMapName(new QPushButton(tr("+"))),
	deleteMapName(new QPushButton(tr("-"))),
	redefineAll(new QPushButton(tr("Define All Inputs"))),
	controllerWidget(new ControllerWidget(this))
{
//	mapNameList->setEditable(true);

	QVBoxLayout * layout = new QVBoxLayout;
	QHBoxLayout * top = new QHBoxLayout;
	QVBoxLayout * left = new QVBoxLayout;
	QVBoxLayout * right = new QVBoxLayout;
	QHBoxLayout * middle = new QHBoxLayout;
	top->addLayout(left, 0);
	top->addLayout(right, 1);
	layout->addLayout(top);
	left->addWidget(label1, 0, Qt::AlignRight);
	left->addWidget(label2, 0, Qt::AlignRight);
	left->addWidget(label3, 0, Qt::AlignRight);
	left->addWidget(new QLabel);
	right->addWidget(deviceList);

	right->addLayout(middle);
	middle->addWidget(mapNameList, 1);
	middle->addWidget(addMapName, 0);
	middle->addWidget(deleteMapName, 0);
//	right->addWidget(mapNameList);

	right->addWidget(controller1);
	right->addWidget(controller2);
	layout->addWidget(controllerWidget);
	layout->addWidget(redefineAll, 0, Qt::AlignHCenter);
	setLayout(layout);
	// At least by doing this, it keeps the QComboBox from resizing itself too
	// large and breaking the layout. :-P
	setFixedWidth(sizeHint().width());

	connect(redefineAll, SIGNAL(clicked()), this, SLOT(DefineAllKeys()));
	connect(deviceList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeDevice(int)));
	connect(mapNameList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeMapName(int)));
	connect(addMapName, SIGNAL(clicked()), this, SLOT(AddMapName()));
	connect(deleteMapName, SIGNAL(clicked()), this, SLOT(DeleteMapName()));
	connect(controllerWidget, SIGNAL(KeyDefined(int, uint32_t)), this, SLOT(UpdateProfileKeys(int, uint32_t)));
	connect(controller1, SIGNAL(clicked()), this, SLOT(UpdateProfileConnections()));
	connect(controller2, SIGNAL(clicked()), this, SLOT(UpdateProfileConnections()));

	// Set up the device combobox (Keyboard is the default, and always
	// present)
	deviceList->addItem(tr("Keyboard"), 0);
	// Set up map name combobox (Default is default, and always present)
//	mapNameList->addItem(tr("Default"));

	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));
		deviceList->addItem(Gamepad::GetJoystickName(i), deviceNum);
	}
}
/*
So now we come to implementation. When changing devices, could have a helper function
in profile.cpp that fills the mapNameList combobox with the appropriate names/profile
numbers.

There needs to be some way of getting data from the ControllerWidget and the current
profile.

Gamepad will have to have some way of knowing which profile is mapped to which
Jaguar controllers and filtering out everything else.

Will have to have some intelligent handling of profiles when first run, to see first
what is connected and second, to assign profiles to Jaguar controllers. In this
case, keyboard is the lowest priority--if a controller is plugged in and assigned to
the same Jaguar controller as a keyboard, the controller is used. Not sure what to
do in the case of multiple controllers plugged in and assigned to the same Jaguar
controller.

Also, need a way to load/save profiles.

Meaning of checkboxes: None checked == profile not used.
1 checked == prefer connection to Jaguar controller X.
2 checked == no preference, use any available.

Single mapping cannot be deleted ("-" will be disabled). Can always add, up to the max
limit of profiles (MAX_PROFILES).

------------------------------

Now the main window passes in/removes the last edited profile #. From here, when starting
up, we need to pull that number from the profile store and populate all our boxes.
*/


ControllerTab::~ControllerTab()
{
}


void ControllerTab::SetupLastUsedProfile(void)
{
	int deviceNum = deviceList->findData(profile[profileNum].device);
	int mapNum = mapNameList->findText(profile[profileNum].mapName);

	if (deviceNum == -1 || mapNum == -1)
		return;

	ChangeDevice(deviceNum);
	ChangeMapName(mapNum);
}


void ControllerTab::DefineAllKeys(void)
{
//	char jagButtonName[21][10] = { "Up", "Down", "Left", "Right",
//		"*", "7", "4", "1", "0", "8", "5", "2", "#", "9", "6", "3",
//		"A", "B", "C", "Option", "Pause" };
	int orderToDefine[21] = { 0, 1, 2, 3, 18, 17, 16, 20, 19, 7, 11, 15, 6, 10, 14, 5, 9, 13, 8, 4, 12 };
	KeyGrabber keyGrab(this);

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		keyGrab.SetKeyText(orderToDefine[i]);
		keyGrab.exec();
		int key = keyGrab.key;

		if (key == Qt::Key_Escape)
			break;

		// Otherwise, populate the appropriate spot in the settings & update screen...
		controllerWidget->keys[orderToDefine[i]] = key;
		controllerWidget->update();
		profile[profileNum].map[orderToDefine[i]] = key;
	}
}


void ControllerTab::UpdateProfileKeys(int mapPosition, uint32_t key)
{
	profile[profileNum].map[mapPosition] = key;
}


void ControllerTab::UpdateProfileConnections(void)
{
	profile[profileNum].preferredController = (controller1->isChecked() ? CONTROLLER1 : 0) | (controller2->isChecked() ? CONTROLLER2 : 0);
}


void ControllerTab::ChangeDevice(int selection)
{
	int deviceNum = deviceList->itemData(selection).toInt();
	mapNameList->clear();
	int numberOfMappings = FindMappingsForDevice(deviceNum, mapNameList);
	deleteMapName->setDisabled(numberOfMappings == 1 ? true : false);
//printf("Found %i mappings for device #%u...\n", numberOfMappings, deviceNum);
}


void ControllerTab::ChangeMapName(int selection)
{
	profileNum = mapNameList->itemData(selection).toInt();
//printf("You selected mapping: %s (profile #%u)\n", (mapNameList->itemText(selection)).toAscii().data(), profileNum);

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
		controllerWidget->keys[i] = profile[profileNum].map[i];

	controllerWidget->update();
	controller1->setChecked(profile[profileNum].preferredController & CONTROLLER1);
	controller2->setChecked(profile[profileNum].preferredController & CONTROLLER2);
}


void ControllerTab::AddMapName(void)
{
printf("Add new mapping (TODO)...\n");
}


void ControllerTab::DeleteMapName(void)
{
printf("Delete current mapping (TODO)...\n");
}


/*
The profiles need the following:

 - The name of the controller
 - A unique human readable ID
 - The key definitions for that controller (keyboard keys can be mixed in)

So there can be more than one profile for each unique controller; the
relationship is many-to-one. So basically, how it works it like this: SDL
reports all connected controllers. If there are none connected, the default
controller is the keyboard (which can have multiple profiles). The UI only
presents those profiles which are usuable with the controllers that are plugged
in, all else is ignored. The user can pick the profile for the controller and
configure the keys for it; the UI automagically saves everything.

How to handle the case of identical controllers being plugged in? How does the
UI know which is which? Each controller will have a mapping to a default
Jaguar controller (#1 or #2). Still doesn't prevent confusion though. Actually,
it can: The profile can have a field that maps it to a preferred Jaguar
controller, which can also be both (#1 AND #2--in this case we can set it to
zero which means no preference). If the UI detects two of the same controller
and each can be mapped to the same profile, it assigns them in order since it
doesn't matter, the profiles are identical.

The default profile is always available and is the keyboard (hey, we're PC
centric here). The default profile is usually #0.

Can there be more than one keyboard profile? Why not? You will need separate
ones for controller #1 and controller #2.

A profile might look like this:

Field 1: Nostomo N45 Analog
Field 2: Dad's #1
Field 3: Jaguar controller #1
Field 4: The button/stick mapping

Profile # would be implicit in the order that they are stored in the internal
data structure.

When a new controller is plugged in with no profiles attached, it defaults to
a set keyboard layout which the user can change. So every new controller will
always have at least one profile.

Data structures:
The Gamepad class has the name of the controller (except for Keyboard)
The profile list is just a list
The controller name index + profile index makes a unique key
Probably the best way to deal with it is to stuff the name/profile indices
into the key definition structure.

#define CONTROLLER1 0x01
#define CONTROLLER2 0x02

struct Profile
{
	int device;					// Host device number
	char mapName[32];			// Human readable map name
	int preferredController;	// CONTROLLER1 and/or CONTROLLER2
	int map[21];				// Keys/buttons/axes
};

NOTE that device is an int, and the list is maintained elsewhere. It is
*not* the same as what you see in GetJoystickName(); the device names have
to be able to persist even when not available.

Where to store the master profile list? It has to be accessible to this class.
vjs.profile[x] would be good, but it's not really a concern for the Jaguar core.
So it shouldn't go there. There should be a separate global setting place for
GUI stuff...
*/

