# **WaitUntilConnected** version 0.3

## **Download**

[Download lastest version](https://www.dropbox.com/scl/fo/7mfi4w9akwzi7xg2zx6bc/h/Programs/WaitUntilConnected_0.3b.lha?rlkey=gbp1msizar64607yjj12q2232&e=2&dl=0)

## **Description**

`WaitUntilConnected` is a simple **SANA2** tool.

It waits until the **SANA2** device is **CONNECTED** and is intended to be used with the **RoadShow TCP/IP stack**, but not exclusive.
 
Written by `Philippe CARPENTIER`, 2024.

Compiled with SAS/C 6.59 for AmigaOS/M68K.

Freely distributed for non-commercial purposes.

## **Arguments**

```
DEVICE: The SANA2 device name (full path).
UNIT:   The SANA2 device unit number.
DELAY:  Extra delay (in ticks, 50 per second).
```

## **Defaults**

```
DEVICE="DEVS:Networks/wifipi.device"
UNIT=0
DELAY=25
```
 
## **Syntax**

`C:WaitUntilConnected ?`

`C:WaitUntilConnected DEVICE="DEVS:Networks/wifipi.device" UNIT=0`

`C:WaitUntilConnected DEVICE="DEVS:Networks/wifipi.device" UNIT=0 DELAY=50`

`C:Version FULL C:WaitUntilConnected`
 
## **Example**

```
; Network-startup script for the PiStorm/Emu68 WiFiPi.device and RoadShow

FailAt 30

Run <>NIL: C:WirelessManager DEVICE="wifipi.device" UNIT=0 CONFIG="ENVARC:Sys/Wireless.prefs"

C:WaitUntilConnected DEVICE="DEVS:Networks/wifipi.device" UNIT=0 DELAY=100

IF NOT WARN
    Run >NIL: NetLogViewer CX_POPUP=NO
    C:AddNetInterface DEVS:NetInterfaces/WiFiPi
ENDIF
```

## **Remarks**

The `DELAY` option is not mandatory, but can eventually be useful in case the WiFi key negociation takes a bit longer than expected. This is because the **SANA2** `S2EVENT_CONNECT` occurs **BEFORE** the WiFi key negociation is done. It would have been nice that the **SANA2** protocol offered a new event to inform the key negociation is completed.

The command will exit itself after a **TIMEOUT** of ~ 1 minute.

In case of **TIMEOUT** or **CTRL+C** the DOS **WARN** Return Code ($RC) is returned.
