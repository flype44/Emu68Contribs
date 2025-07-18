#ifndef __WAITUNTILCONNECTED_H__
#define __WAITUNTILCONNECTED_H__

/******************************************************************************
 * DEFINES FOR APP
 ******************************************************************************/

#define APP_NAME     "WaitUntilConnected"
#define APP_DESCR    "Wait until the SANA2 device is CONNECTED."
#define APP_AUTHOR   "Philippe CARPENTIER"
#define APP_VERSION  "0"
#define APP_REVISION "3"
#define APP_DATE     "21.4.2024"
#define APP_TITLE    APP_NAME " by " APP_AUTHOR
#ifdef VERSION_STRING
#define APP_VSTRING VERSION_STRING " "APP_AUTHOR;
#else
#define APP_VSTRING  "$VER: " APP_NAME " " APP_VERSION "." APP_REVISION " (" APP_DATE ") [SAS/C 6.59] (C) " APP_AUTHOR
#endif

/******************************************************************************
 * DEFINES FOR ARGUMENTS
 ******************************************************************************/

#define DEFAULT_DEVICE_NAME "DEVS:Networks/wifipi.device"
#define DEFAULT_UNIT_NUMBER 0
#define DEFAULT_DELAY 25

/******************************************************************************
 * END OF FILE
 ******************************************************************************/

#endif // __WAITUNTILCONNECTED_H__
