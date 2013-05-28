#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <stdint.h>

class QComboBox;
class QSettings;

#define MAX_PROFILES  64
#define CONTROLLER1   0x01
#define CONTROLLER2   0x02


struct Profile
{
	int device;					// Host device number (-1 == invalid profile)
	char mapName[32];			// Human readable map name
	int preferredController;	// CONTROLLER1 and/or CONTROLLER2
	uint32_t map[21];			// Keys/buttons/axes
};


// Function prototypes
void ReadProfiles(QSettings *);
void WriteProfiles(QSettings *);
int GetFreeProfile(void);
void DeleteProfile(int);
int FindDeviceNumberForName(const char *);
int FindMappingsForDevice(int, QComboBox *);
int FindUsableProfiles(QComboBox *);
bool ConnectProfileToController(int, int);
void AutoConnectProfiles(void);


// Exported variables
extern Profile profile[];
extern int controller1Profile;
extern int controller2Profile;
extern int gamepad1Slot;
extern int gamepad2Slot;
//extern int numberOfProfiles;

#endif	// __PROFILE_H__

