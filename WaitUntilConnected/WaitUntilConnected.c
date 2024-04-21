/******************************************************************************
 * 
 * WaitUntilConnected version 0.2
 * 
 * Short:
 * WaitUntilConnected is a simple SANA2 tool.
 * It waits until the SANA2 device is CONNECTED and is intended
 * to be used with the RoadShow TCP/IP stack, but not exclusive.
 * 
 * Written by Philippe CARPENTIER, 2024.
 * Compiled with SAS/C 6.58 for AmigaOS/M68K.
 * Freely distributed for non-commercial purposes.
 * 
 * Arguments:
 * DEVICE: The SANA2 device name (full path).
 * UNIT:   The SANA2 device unit number.
 * DELAY:  Extra delay (in ticks, 50 per second).
 * 
 * Defaults:
 * DEVICE="DEVS:Networks/wifipi.device"
 * UNIT=0
 * DELAY=25
 * 
 * Syntax:
 * C:WaitUntilConnected ?
 * C:WaitUntilConnected DEVICE="DEVS:Networks/wifipi.device" UNIT=0
 * C:WaitUntilConnected DEVICE="DEVS:Networks/wifipi.device" UNIT=0 DELAY=50
 * C:Version FULL C:WaitUntilConnected
 * 
 * Example:
 * Run <>NIL: C:WirelessManager DEVICE="wifipi.device" UNIT=0 CONFIG="ENVARC:Sys/Wireless.prefs"
 * C:WaitUntilConnected DEVICE="DEVS:Networks/wifipi.device" UNIT=0 DELAY=100
 * Run >NIL: NetLogViewer CX_POPUP=NO
 * C:AddNetInterface DEVS:NetInterfaces/WiFiPi
 * 
 ******************************************************************************/

#include <string.h>
#include <devices/sana2.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include "WaitUntilConnected.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

#define TEMPLATE "DEVICE,UNIT/N,DELAY/N"

enum OPT_ARGS
{
	OPT_DEVICE,
	OPT_UNIT,
	OPT_DELAY,
	OPT_COUNT
};

/******************************************************************************
 * Globals
 ******************************************************************************/

static STRPTR VERSTAG = APP_VSTRING;

extern struct ExecBase   * SysBase;
extern struct DosLibrary * DOSBase;

struct RDArgs     * rdargs   = NULL;
struct MsgPort    * SanaPort = NULL;
struct IOSana2Req * SanaReq  = NULL;
struct Device     * SanaDev  = NULL;

/******************************************************************************
 * S2Listen()
 ******************************************************************************/

void S2Listen(void)
{
	BOOL bContinue = TRUE;
	
	// SANA2 ONEVENT -> S2EVENT_CONNECT
	
	SanaReq->ios2_Req.io_Command = S2_ONEVENT;
	SanaReq->ios2_Req.io_Error   = 0;
	SanaReq->ios2_Req.io_Flags   = 0;
	SanaReq->ios2_StatData       = 0;
	SanaReq->ios2_WireError      = S2EVENT_CONNECT;
	
	SendIO((struct IORequest *)SanaReq);
	
	// Wait until the device is CONNECTED
	
	while (bContinue)
	{
		struct Message * msg;
		
		if (msg = GetMsg(SanaPort))
		{
			struct IOSana2Req * ioreq = (struct IOSana2Req *)msg;
			
			if (CheckIO((struct IORequest *)ioreq))
			{
				WaitIO((struct IORequest *)ioreq);
			}
			
			bContinue = FALSE;
		}
		
		if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
		{
			bContinue = FALSE;
		}
		
		Delay(DEFAULT_DELAY);
	}
	
	return;
}

/******************************************************************************
 * main()
 ******************************************************************************/

ULONG main(ULONG argc, UBYTE *argv[])
{
	ULONG RC = RETURN_ERROR;
	
	if (argc >= 1)
	{
		LONG opts[OPT_COUNT];
		struct RDArgs *rdargs;
		memset((STRPTR)opts, NULL, sizeof(opts));
		
		// Read DOS arguments
		
		if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
		{
			STRPTR deviceName       = DEFAULT_DEVICE_NAME;
			LONG   deviceUnitNumber = DEFAULT_UNIT_NUMBER;
			LONG   extraDelay       = DEFAULT_DELAY;
			
			// Read DEVICE argument
			
			if (opts[OPT_DEVICE] != NULL)
			{
				deviceName = (STRPTR)opts[OPT_DEVICE];
			}
			
			// Read UNIT argument
			
			if (opts[OPT_UNIT] != NULL)
			{
				deviceUnitNumber = *((LONG *)opts[OPT_UNIT]);
			}
			
			// Read DELAY argument
			
			if (opts[OPT_DELAY] != NULL)
			{
				extraDelay = *((LONG *)opts[OPT_DELAY]);
			}
			
			if (deviceName != NULL)
			{
				// Create MSGPORT
				
				if (SanaPort = CreateMsgPort())
				{
					// Create IOREQUEST
					
					if (SanaReq = (struct IOSana2Req *)CreateIORequest(SanaPort, sizeof(struct IOSana2Req)))
					{
						// Open DEVICE
						
						if (OpenDevice(deviceName, deviceUnitNumber, (struct IORequest *)SanaReq, 0) == 0)
						{
							SanaDev = SanaReq->ios2_Req.io_Device;
							
							if (SanaDev != NULL)
							{
								// Wait for the S2EVENT_CONNECT event.
								S2Listen();
								
								// Add an extra delay, because S2EVENT_CONNECT
								// occurs BEFORE the WiFi key negociation is done.
								Delay(extraDelay);
								
								// Ending...
								CloseDevice((struct IORequest *)SanaReq);
								RC = RETURN_OK;
							}
						}
						else
						{
							Printf("OpenDevice() failed !\n");
						}
						
						DeleteIORequest(SanaReq);
					}
					else
					{
						Printf("CreateIORequest() failed !\n");
					}
					
					DeleteMsgPort(SanaPort);
				}
				else
				{
					Printf("CreateMsgPort() failed !\n");
				}
			}
			
			FreeArgs(rdargs);
		}
		else
		{
			Printf("Invalid arguments.\n");
		}
	}
	else
	{
		Printf("Wrong number of arguments.\n");
	}
	
	return (RC);
}

/******************************************************************************
 * End of file
 ******************************************************************************/
