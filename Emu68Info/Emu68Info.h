#ifndef __EMU68INFO_H__
#define __EMU68INFO_H__

#ifndef AFB_68060
#define AFB_68060 (7)
#define AFF_68060 (1<<AFB_68060)
#endif

#ifdef VERSION_STRING
#define APP_VSTRING VERSION_STRING " Philippe CARPENTIER";
#else
#define APP_VSTRING "$VER: Emu68Info 0.1k (10.2.2022) Philippe CARPENTIER"
#endif

#ifndef AFB_68080
#define AFB_68080 (10)
#define AFF_68080 (1<<AFB_68080)
#endif

#define EMU68_MANUFACTURER       (0x6d73)
#define EMU68_PRODUCT_SUPPORT    (0x01)
#define EMU68_PRODUCT_RAM        (0x10)
#define EMU68_PRODUCT_DEVICETREE (0x21)
#define EMU68_PRODUCT_SDHC       (0x22)

#endif
