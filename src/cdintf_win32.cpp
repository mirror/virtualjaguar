//
// OS specific CDROM interface (Win32)
//
// by James L. Hammons
//

// OS dependent CDROM stuffola
#include <windows.h>
#include <stdio.h>
#include "wnaspi32.h"
#include "scsidefs.h"
// End OS dependent

#include "log.h"

using namespace std;

// Local variables

DWORD remain = 0, sector = 0;
BYTE cdBuf[2532 * 10];

// Private function prototypes


HINSTANCE WNASPI32_handle = NULL;  //Handle to ASPI for Win32 (WNASPI.DLL)
//WNASPI.DLL functions
DWORD (*ASPI_GetASPI32SupportInfo)(VOID);
DWORD (*ASPI_SendASPI32Command)(LPSRB);
//BOOL  (*ASPI_GetASPI32Buffer)(PASPI32BUFF);
//BOOL  (*ASPI_FreeASPI32Buffer)(PASPI32BUFF);
//BOOL  (*ASPI_TranslateASPI32Address)(PDWORD, PDWORD);

byte DataBuf[2352];  //Buffer for holding data to/from drive
//************************************************************
//End of globals

//
// Load the WNASPI32.DLL and import the required functions, then initialise
// Win ASPI 32.
//
BOOL InitASPI()
{
	DWORD dwSupportInfo;
	SRB_GetSetTimeouts srbTimeouts;

	WNASPI32_handle = LoadLibrary("WNASPI32"); //WNASPI32.DLL

	if (!WNASPI32_handle)
	{
		WriteLog("CDINTF: Could not load WNASPI32.DLL\n");
		return FALSE;
	};

    /*
    ** Get the ASPI entry points.  Note that only two functions are mandatory:
    ** GetASPI32SupportInfo and SendASPI32Command.  The code will run if the
    ** others are not present.
    */
    ASPI_GetASPI32SupportInfo = (DWORD (*)(void))GetProcAddress(WNASPI32_handle, "GetASPI32SupportInfo");
    ASPI_SendASPI32Command = (DWORD (*)(LPSRB))GetProcAddress(WNASPI32_handle, "SendASPI32Command");
    //ASPI_GetASPI32Buffer = (BOOL (*)(PASPI32BUFF))GetProcAddress(WNASPI32_handle, "GetASPI32Buffer");
    //ASPI_FreeASPI32Buffer = (BOOL (*)(PASPI32BUFF))GetProcAddress(WNASPI32_handle, "FreeASPI32Buffer");
    //ASPI_TranslateASPI32Address = (BOOL (*)(PDWORD, PDWORD))GetProcAddress(WNASPI32_handle, "TranslateASPI32Address");

    //Check if the 2 functions were imported.
    if(!ASPI_GetASPI32SupportInfo || !ASPI_SendASPI32Command)
    {
        WriteLog("Could not import GetASPI32SupportInfo & SendASPI32Command functions from WNASPI32.DLL\n");
        return FALSE;
    };

    //Initialise Win ASPI 32 by calling ASPI_GetASPI32SupportInfo().
    dwSupportInfo = ASPI_GetASPI32SupportInfo();
    if (HIBYTE(LOWORD(dwSupportInfo)) != SS_COMP && HIBYTE(LOWORD(dwSupportInfo)) != SS_NO_ADAPTERS)
    {
        WriteLog("Could not initialise using GetASPI32SupportInfo function or no adapters\n");
        return FALSE;
    };

    /*
    ** Set timeouts for ALL devices to 15 seconds.  Nothing we deal with should
    ** take that long to do ANYTHING.  We are just doing inquiries to most
    ** devices, and then simple reads to CDs, disks, etc. so 10 seconds (even
    ** if they have to spin up) should be plenty. 
    */
    memset(&srbTimeouts, 0, sizeof(SRB_GetSetTimeouts));
    srbTimeouts.SRB_Cmd = SC_GETSET_TIMEOUTS;
    srbTimeouts.SRB_HaId = 0xFF;
    srbTimeouts.SRB_Flags = SRB_DIR_OUT;
    srbTimeouts.SRB_Target = 0xFF;
    srbTimeouts.SRB_Lun = 0xFF;
    srbTimeouts.SRB_Timeout = 15 * 2;
    ASPI_SendASPI32Command(&srbTimeouts);

    return TRUE;
};

/*
    Sends a SRB (SCSI Request Block) to Win ASPI32 driver for processing and
    waits for completion.

    The SRB contains the CDB (Command Descriptor Block) which has the
    raw SCSI command/info you want to send to the drive.  So effectively
    this is actually sending the SCSI command to the drive.

    Note that this uses a Windows event to wait for the drive to reply back
    which is efficient.  If an event is not available (which should never
    happen really) a loop is used to poll for reply back.

    Both methods will wait infinitely - so the drive must reply back otherwise
    this software will appear to hang.  There seems to be no mechanism for
    cancelling the SRB process - the Win ASPI 32 'SC_ABORT_SRB' command doesn't
    seem to work.
*/
BOOL SendASPICMD_and_wait(BYTE HA_ID, BYTE Target_ID, BYTE SRB_flags, DWORD Buffer_len,
	PBYTE Buffer, BYTE CDB_len, PBYTE CDB)
{
    DWORD           dwASPIStatus;
    HANDLE          hevent_SRB;  //A handle for a new Windows event
    SRB_ExecSCSICmd SRB;  //The SRB variable with CDB included
    BOOL b_retry = TRUE;

    //Clear & setup the SRB for this command..
    memset(&SRB, 0, sizeof(SRB_ExecSCSICmd));  //Set it to zeroes
    memcpy(SRB.CDBByte, CDB, CDB_len);  //Copy CDB into SRB

    SRB.SRB_Cmd = SC_EXEC_SCSI_CMD;
    SRB.SRB_HaId = HA_ID;
    SRB.SRB_Target = Target_ID;
    //SRB.SRB_Lun = 0;  //It's already zero
    SRB.SRB_Flags = SRB_flags;
    SRB.SRB_BufLen = Buffer_len;
    SRB.SRB_BufPointer = Buffer;
    SRB.SRB_SenseLen = SENSE_LEN;
    SRB.SRB_CDBLen = CDB_len;

    do
    {
        /*
        ** Create an event (if possible) and issue the command.  After sending
        ** the command, wait for completion.
        */
        hevent_SRB = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (hevent_SRB)
        {
            //Windows event method for waiting - efficient.

            SRB.SRB_Flags |= SRB_EVENT_NOTIFY;
            SRB.SRB_PostProc = (LPVOID)hevent_SRB;
        
            //Send the SRB for processing.
            dwASPIStatus = ASPI_SendASPI32Command((LPSRB)&SRB);
            if (dwASPIStatus == SS_PENDING)
            {
                //Wait for reply back.
                WaitForSingleObject(hevent_SRB, INFINITE);
            }
            CloseHandle(hevent_SRB);
        }
        else
        {
            //Polling method for waiting - not very efficient.

            //Send the SRB for processing.
            ASPI_SendASPI32Command((LPSRB)&SRB);
            //Wait for reply back.
            while(SRB.SRB_Status == SS_PENDING);
        };

        /*
        ** Check for errors.  We'll retry on unit attention condition.  Anything
        ** else will generate an error msg.
        */
        if (SRB.SRB_Status != SS_COMP)
        {
			if (b_retry && (SRB.SRB_TargStat != STATUS_CHKCOND
				|| (SRB.SenseArea[2] & 0x0F) != KEY_UNITATT))
                b_retry = FALSE;
            else
            {
                WriteLog("SCSI command failed.\n");
                return FALSE;
            }
        }
    }
    while (b_retry == FALSE);

    return TRUE;
};

/*
    Lists all available CDROM type drives.  This includes:
        - CDROM drives
        - CD rewriters/Combo
        - DVD drives
        - DVD rewriters
        etc

    It can only list the SCSI IDs not drive letters.  Win ASPI 32 does
    not provide a mechanism for this.  You will have to use windows API
    for this.
*/
VOID ListDevices()
{
    BYTE          HaId;
    BYTE          Target;
    BYTE          MaxHaId;
    BYTE          MaxTarget;
    BYTE          InquiryBuf[36];
    BYTE          InquiryCDB[6];
    CHAR          szVendor[9];
    CHAR          szProduct[17];
    CHAR          szRev[5];
    BOOL          bSRB_exec;
    DWORD         dwASPIStatus;
    DWORD         dwMaxTransferBytes;    
    SRB_HAInquiry srbHAInquiry;
    SRB_GDEVBlock srbGDEVBlock;
    DWORD         n_CDROM_drives=0;  //No of CDROM type drives found.

    //Use support info for host adapter count and loop over all of them.
    dwASPIStatus = ASPI_GetASPI32SupportInfo();
    if(HIBYTE(LOWORD(dwASPIStatus)) == SS_COMP)
    {
        MaxHaId = LOBYTE(LOWORD(dwASPIStatus));
        for(HaId = 0; HaId < MaxHaId; HaId++)
        {
            /*
            ** Do a host adapter inquiry to get max target count.  If the 
            ** target count isn't 8 or 16 then go with a default of 8.
            */
            memset(&srbHAInquiry, 0, sizeof(SRB_HAInquiry));
            srbHAInquiry.SRB_Cmd = SC_HA_INQUIRY;
            srbHAInquiry.SRB_HaId = HaId;

            ASPI_SendASPI32Command((LPSRB)&srbHAInquiry);
            if(srbHAInquiry.SRB_Status != SS_COMP)
            {
                continue;
            };

            MaxTarget = srbHAInquiry.HA_Unique[3];
            if(MaxTarget != 8 && MaxTarget != 16)
            {
                MaxTarget = 8;
            };

            /*
            ** Loop over all the targets on this host adapter.
            */
            for(Target = 0; Target < MaxTarget; Target++ )
            {
                /*
                ** Issue get device type call to see if there is a device we're
                ** interested in at this address.  We're interested in CDROMs.
                */
                memset(&srbGDEVBlock, 0, sizeof(SRB_GDEVBlock));
                srbGDEVBlock.SRB_Cmd = SC_GET_DEV_TYPE;
                srbGDEVBlock.SRB_HaId = HaId;
                srbGDEVBlock.SRB_Target = Target;

                ASPI_SendASPI32Command((LPSRB)&srbGDEVBlock);
                if(srbGDEVBlock.SRB_Status != SS_COMP ||
                  (srbGDEVBlock.SRB_DeviceType != DTYPE_CDROM))
                {                    
                    continue;
                };

                /*
                ** Determine the max transfer count of this target.  It will
                ** be the min of the host adapters min count and the size
                ** of our global transfer buffer.
                */
                dwMaxTransferBytes = (DWORD)&srbHAInquiry.HA_Unique[4];

                /*
                ** Issue an INQUIRY.
                */
                memset(InquiryCDB, 0, 6);
                InquiryCDB[0] = SCSI_INQUIRY;
                InquiryCDB[4] = 36;  //Size in bytes of inquiry buffer.

                //Send SCSI device inquiry command and wait for completion.
                bSRB_exec = SendASPICMD_and_wait
                (
                    HaId,
                    Target,
                    SRB_DIR_IN,
                    36,
                    InquiryBuf,
                    6,
                    InquiryCDB
                );

                /*
                ** Make sure the inquiry worked.  If it failed, or if the
                ** inquiry data returns a different device type than we got
                ** before (guards against certain device drivers and against
                ** vendor unique devices).
                */
                if(!bSRB_exec || (InquiryBuf[0] != DTYPE_CDROM))
                    continue;

                /*
                ** Add this target to the screen.
                */
                WriteLog("Host adapter ID: %ld\n", HaId);
                WriteLog("Target ID      : %ld\n", Target);
                WriteLog("LUN ID         : 0\n");
                WriteLog("Max buffer size: %ld bytes\n", dwMaxTransferBytes);

                memcpy(szVendor, InquiryBuf + 8, 8);                
                szVendor[8] = '\0';  //Terminate the string

                memcpy(szProduct, InquiryBuf + 16, 16);
                szProduct[16] = '\0';  //Terminate the string

                memcpy(szRev, InquiryBuf + 32, 4);
                szRev[4] = '\0';  //Terminate the string

                WriteLog("Vendor         : %s\n", szVendor);
                WriteLog("Product        : %s\n", szProduct);
                WriteLog("Revision       : %s\n\n", szRev);

                n_CDROM_drives++;
            }
        }

        if (n_CDROM_drives == 0)
            WriteLog("No CDROM type drives found.\n");
    }
}

//
// 1. Sets up the CDB for MMC readcd (CDB12) command.
// 2. Send the request to the drive.
// 3. If success displays the sector data as hex on the screen.
//
BOOL ReadCD(BYTE HA_ID, BYTE Target_ID, long int MMC_LBA_sector)
{
    BYTE read_CDB12[12];
    long int MMC_LBA_sector2;    

    //CDB with values for ReadCD CDB12 command.  The values were taken from MMC1 draft paper.
    read_CDB12[0] = 0xBE;  //Code for ReadCD CDB12 command
    read_CDB12[1] = 0;        

    read_CDB12[5] = byte(MMC_LBA_sector);   //Least sig byte of LBA sector no. to read from CD
    MMC_LBA_sector2 = MMC_LBA_sector >> 8;
    read_CDB12[4] = byte(MMC_LBA_sector2);  //2nd byte of:
    MMC_LBA_sector2 = MMC_LBA_sector2 >> 8;
    read_CDB12[3] = byte(MMC_LBA_sector2);  //3rd byte of:
    MMC_LBA_sector2 = MMC_LBA_sector2 >> 8;
    read_CDB12[2] = byte(MMC_LBA_sector2);  //Most significant byte        

    read_CDB12[6] = 0;  //No. of sectors to read from CD byte 2 (MSB)
    read_CDB12[7] = 0;  //No. of sectors to read from CD byte 1
    read_CDB12[8] = 1;  //No. of sectors to read from CD byte 0 (LSB)
    read_CDB12[9] = 0xF8;  //Raw read, 2352 bytes per sector
    read_CDB12[10] = 0;  //Sub-channel selection bits.
    read_CDB12[11] = 0;

	return SendASPICMD_and_wait(HA_ID, Target_ID, SRB_DIR_IN, 2352, DataBuf, 12, read_CDB12);
}

//
// Initialize the SDL sound system
//
void __CDInit(void)
{
	if (!InitASPI())
	{
		WriteLog("Sound: Failed to init ASPI layer!\n");
		exit(1);//bad!
	}
}

//
// Close down the SDL sound subsystem
//
void __CDDone(void)
{
    //Unload ASPI if it has been loaded.
	if (WNASPI32_handle)
		FreeLibrary(WNASPI32_handle);
}

/*//
// Sound card callback handler
//
void SDLSoundCallback(void * userdata, Uint8 * buffer, int length)
{
	if (remain > 0)
	{
		memcpy(buffer, DataBuf + 2352 - remain, remain);
		length -= remain;
		buffer += remain;
		remain = 0;
	}

	while (length > 0)
	{
		ReadCD(0, 1, sector++);
		memcpy(buffer, DataBuf, (length >= 2352 ? 2352 : length));
		length -= 2352;
		buffer += 2352;
	}

	if (length < 0)
		remain = -length;
}*/
