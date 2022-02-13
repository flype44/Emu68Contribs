/**********************************************************
 * 
 * Project: Emu68Info
 * Version: 0.1-WIP (2022)
 * Author:  Philippe CARPENTIER
 * 
 **********************************************************/

/**********************************************************
 * 
 * TODO:
 * 
 * [X] Function PrintTitle() to abstract the DOS format codes (0033b)
 * [X] Replace CR/LF/CRLF in IdStrings (in MODULES)
 * 
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <hardware/custom.h>
#include <libraries/expansion.h>
#include <libraries/configregs.h>
#include <libraries/configvars.h>
#include <cybergraphics/cybergraphics.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/cybergraphics.h>
#include <proto/devicetree.h>

#include "AsmFuncs.h"
#include "DeviceTree.h"
#include "MailBox.h"
#include "Modules.h"
#include "Emu68Info.h"
#include "VC4Msg.h"
#include "VC4.h"

/**********************************************************
 * DEFINES
 **********************************************************/

#define TEMPLATE "\
HELP/S,\
AMIGA/S,\
BOARD/S,\
BOARDID/S,\
BOARDNAME/S,\
BOOTARGS/S,\
CHIPID/S,\
CLOCKRATE/S,\
CLOCKRATES/S,\
CMDLINE/S,\
COUNTERS/S,\
CPU/S,\
DEBUG/S,\
DENISEID/S,\
DETECT/S,\
DMA/S,\
EDID/S,\
EXPANSION/S,\
HARDRESET/S,\
IDSTRING/S,\
JIT/S,\
LED/S,\
MEMLIST/S,\
MODULES/S,\
POWERSTATE/S,\
SETATTN,\
SETBLANK/N,\
SETCLOCKRATE/N,\
SETGAMMA/N,\
SETLED/N,\
SETTURBO/N,\
TEMPERATURE/S,\
UPTIME/S,\
VC4/S,\
VOLTAGE/S\
"
typedef enum {
	OPT_HELP,
	OPT_AMIGA,
	OPT_BOARD,
	OPT_BOARDID,
	OPT_BOARDNAME,
	OPT_BOOTARGS,
	OPT_CHIPID,
	OPT_CLOCKRATE,
	OPT_CLOCKRATES,
	OPT_CMDLINE,
	OPT_COUNTERS,
	OPT_CPU,
	OPT_DEBUG,
	OPT_DENISEID,
	OPT_DETECT,
	OPT_DMA,
	OPT_EDID,
	OPT_EXPANSION,
	OPT_HARDRESET,
	OPT_IDSTRING,
	OPT_JIT,
	OPT_LED,
	OPT_MEMLIST,
	OPT_MODULES,
	OPT_POWERSTATE,
	OPT_SETATTN,
	OPT_SETBLANK,
	OPT_SETCLOCKRATE,
	OPT_SETGAMMA,
	OPT_SETLED,
	OPT_SETTURBO,
	OPT_TEMPERATURE,
	OPT_UPTIME,
	OPT_VC4,
	OPT_VOLTAGE,
	OPT_COUNT
} OPT_ARGS;

/**********************************************************
 * GLOBALS
 **********************************************************/

UBYTE * AttnName[16] = { 
	"10",     //  0 (68010)
	"20",     //  1 (68020)
	"30",     //  2 (68030)
	"40",     //  3 (68040)
	"881",    //  4 (68881)
	"882",    //  5 (68882)
	"FPU40",  //  6 (FPU40)
	"60",     //  7 (68060)
	"BIT8",   //  8 (Unused)
	"BIT9",   //  9 (Unused)
	"80",     // 10 (68080)
	"BIT11",  // 11 (Unused)
	"BIT12",  // 12 (Unused)
	"ADDR32", // 13 (CPU is 32bit)
	"BIT14",  // 14 (MMU presence)
	"PRIV"    // 15 (FPU presence)
};

APTR MailBox = NULL;
APTR DeviceTreeBase = NULL;

struct ExpansionBase * ExpansionBase;

extern struct ExecBase   * SysBase;
extern struct DosLibrary * DOSBase;
extern struct GfxBase    * GfxBase;

static STRPTR verstring = APP_VSTRING;

UBYTE sBuffer[4096];
STRPTR unknown = "Unknown";

/**********************************************************
 ** 
 ** Help()
 ** 
 **********************************************************/

ULONG Help(void)
{
	printf(verstring + 6);
	
	printf(
	"\n"
	"\n"
	"HELP           : Get this help\n"
	"AMIGA          : Get System Amiga information\n"
	"BOARD          : Get RPi    Board information\n"
	"BOARDID        : Get RPi    Board identifier ($RC)\n"
	"BOARDNAME      : Get RPi    Board Name\n"
	"BOOTARGS       : Get RPi    Boot-Args\n"
	"CHIPID         : Get Amiga  Agnus/Alice ID ($RC)\n"
	"CLOCKRATE      : Get RPi    ARM clock rate in MHz ($RC)\n"
	"CLOCKRATES     : Get RPi    Clock rates\n"
	"CMDLINE        : Get RPi    CmdLine.txt\n"
	"COUNTERS       : Get Emu68  Counters information\n"
	"DEBUG          : Get Emu68  Debug information\n"
	"DENISEID       : Get Amiga  Denise/Lisa ID ($RC)\n"
	"DETECT         : Get Emu68  Detection ($RC)\n"
	"DMA            : Get RPi    DMA information\n"
	"EDID           : Get RPi    EDID information\n"
	"EXPANSION      : Get System Expansion list\n"
	"HARDRESET      : !!! RPi    Hard Reset (Emu68 reload!)\n"
	"IDSTRING       : Get Emu68  IdString\n"
	"JIT            : Get Emu68  JIT information\n"
	"LED            : Get RPi    LED information\n"
	"MEMLIST        : Get System Memory list\n"
	"MODULES        : Get System Module information\n"
	"POWERSTATE     : Get RPi    Power information\n"
	"SETATTN        : Set System Exec AttnFlags (in Hexa)\n"
	"SETCLOCKRATE   : Set RPi    ARM clock rate in MHz\n"
	"SETLED         : Set RPi    LED state (0=Off, 1=On)\n"
	"SETTURBO       : Set RPi    Turbo-Mode (0=Off, 1=On)\n"
	"TEMPERATURE    : Get RPi    Temperature information\n"
	"UPTIME         : Get RPi    UpTime ($RC)\n"
	"VC4            : Get Emu68  VC4-drivers information ($RC)\n"
	"VOLTAGE        : Get RPi    Voltage information\n"
	"\nAmiga Rulez!\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** PrintSize()
 ** 
 **********************************************************/

void PrintSize(UBYTE * buf, ULONG size, BOOL align)
{
	CONST_STRPTR units[] = { "B", "KB", "MB", "GB", "TB" };
	
	if (buf != NULL)
	{
		ULONG i = 0;
		ULONG r = 0;
		
		STRPTR fmt = align ? "%3lu.%02lu %s" : "%lu.%02lu %s";
		
		while (size >= 1024)
		{
			r = size % 1024;
			size /= 1024;
			i++;
		}
		
		if (r >= 1024)
		{
			size++;
			r = 0;
		}
		
		r /= 100;
		
		sprintf(buf, fmt, size, r, units[i]);
	}
}

/**********************************************************
 ** 
 ** PrintFreq()
 ** 
 **********************************************************/

void PrintFreq(UBYTE * buf, ULONG value, BOOL align)
{
	ULONG i = 0;
	ULONG r = 0;
	STRPTR units[] = { "Hz", "KHz", "MHz", "GHz", "THz" };
	
	if (buf != NULL)
	{
		STRPTR fmt = align ? "%3lu.%lu %3s" : "%lu.%lu %3s";
		
		while (value >= 1000)
		{
			r = value % 1000;
			value /= 1000;
			i++;
		}
		
		r /= 100;
		
		sprintf(buf, fmt, value, r, units[i]);
	}
}

/**********************************************************
 ** 
 ** PrintTitle()
 ** 
 **********************************************************/

void PrintTitle(STRPTR s)
{
	// \033n : Normal
	// \033b : Bold
	// \033u : Underline
	
	printf("\n\033b%s\033n\n\n", s);
}

/**********************************************************
 ** 
 ** GetAttnFlags()
 ** 
 **********************************************************/

ULONG GetAttnFlags(void)
{
	ULONG i, j = 0;
	
	UBYTE buf[128];
	
	memset(buf, 0, 128);
	
	for (i = 0; i < 16; i++)
	{
		if (SysBase->AttnFlags & (1 << i))
		{
			if (j > 0)
			{
				strcat(buf, ",");
			}
			
			strcat(buf, AttnName[i]);
			j++;
		}
	}
	
	printf("AttnFlags      : 0x%04x (%s)\n",
		SysBase->AttnFlags, buf);
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetBootArgs()
 ** 
 **********************************************************/

ULONG GetBootArgs(void)
{
	ULONG result = RETURN_WARN;
	UBYTE sbuf[4096];
	
	PrintTitle("Raspberry Pi DeviceTree BootArgs:");
	
	if (GetPropStr(sbuf, "/chosen", "bootargs"))
	{
		STRPTR a = sbuf;
		STRPTR b = sbuf;
		
		sbuf[strlen(sbuf)] = ' ';
		
		while (b = strstr(b, " "))
		{
			if ((b - a) > 0)
			{
				printf("%.*s\n", b - a, a);
			}
			
			a = ++b;
		}
		
		printf("\n");
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetBootTime()
 ** 
 **********************************************************/

ULONG GetBootTime(void)
{
	APTR oldSysStack;
	
	ULONG a, b, c;
	
	if (oldSysStack = SuperState())
	{
		a = asm_cnt_val_lo();
		b = asm_cnt_val_hi();
		c = asm_cnt_frq();
		
		UserState(oldSysStack);
	}
	
	return (asm_div64(a, b, c));
}

/**********************************************************
 ** 
 ** GetBootTime2()
 ** 
 **********************************************************/

ULONG GetBootTime2(void)
{
	ULONG h, m, s, t;
	
	t = GetBootTime();
	h = 0;
	m = 0;
	s = t;
	
	if (s > 3600)
	{
		h = s / 3600;
		s = s % 3600;
	}
	
	if (s > 60)
	{
		m = s / 60;
		s = s % 60;
	}
	
	printf("UpTime         : %ldh %ldm %lds (%ld secs)\n", 
		h, m, s, t);
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetBoardId()
 ** 
 **********************************************************/

ULONG GetBoardId(void)
{
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG a, b;
		
		mbox_get_board_model(&a, &b);
		
		if (b != 0)
		{
			// board type
			return ((b >> 4) & 0xff);
		}
	}
	
	return (-1);
}

/**********************************************************
 ** 
 ** GetBoardName()
 ** 
 **********************************************************/

ULONG GetBoardName(void)
{
	ULONG result = RETURN_WARN;
	UBYTE boardName[128];
	
	if (GetPropStr(boardName, "/", "model"))
	{
		printf("%s\n", boardName);
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetCmdLine()
 ** 
 **********************************************************/

ULONG GetCmdLine(void)
{
	PrintTitle("Raspberry Pi CMDLINE content:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		STRPTR s, a, b;
		
		mbox_get_command_line(&s);
		
		a = s;
		b = s;
		
		s[strlen(s)] = ' ';
		
		while (b = strstr(b, " "))
		{
			if ((b - a) > 0)
			{
				printf("%.*s\n", b - a, a);
			}
			
			a = ++b;
		}
		
		printf("\n");
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetCPU()
 ** 
 **********************************************************/

ULONG GetCPU(void)
{
	ULONG i;
	
	STRPTR keys[4] = {
		"/cpus/cpu@0",
		"/cpus/cpu@1",
		"/cpus/cpu@2",
		"/cpus/cpu@3",
	};
	
	for (i = 0; i < 4; i++)
	{
		UBYTE buffer1[128];
		
		if (GetPropStr(buffer1, keys[i], "compatible"))
		{
			UBYTE buffer2[128];
			
			PrintFreq(buffer2, GetPropInt(keys[i], "clock-frequency"), FALSE);
			
			printf("CPU%lu: %s @ %s\n", i, buffer1, buffer2);
		}
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetDetect()
 ** 
 **********************************************************/

ULONG GetDetect(void)
{
	ULONG result = RETURN_FAIL;
	
	if (ExpansionBase = (struct ExpansionBase *)OpenLibrary(EXPANSIONNAME, 0L))
	{
		struct ConfigDev* cd;
		
		cd = FindConfigDev(
			NULL,
			EMU68_MANUFACTURER,
			EMU68_PRODUCT_DEVICETREE);
		
		result = (cd == NULL) ? RETURN_WARN : RETURN_OK;
		
		CloseLibrary((struct Library *)ExpansionBase);
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetIdString()
 ** 
 **********************************************************/

ULONG GetIdString(void)
{
	ULONG result = RETURN_WARN;
	UBYTE sbuf[128];
	
	if (GetPropStr(sbuf, "/emu68", "idstring"))
	{
		printf("IdString: %s\n", sbuf + 6);
		
		if (GetPropStr(sbuf, "/emu68", "git-hash"))
		{
			printf("Git-Hash: %s\n", sbuf);
		}
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetMemBase()
 ** 
 **********************************************************/

ULONG GetMemBase(void)
{
	UBYTE bootargs[4096];
	
	if (GetPropStr(bootargs, "/chosen", "bootargs"))
	{
		printf("Memory Base    : 0x%08lx\n", 
			GetField(bootargs, ".mem_base=", 16));
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetMemSize()
 ** 
 **********************************************************/

ULONG GetMemSize(void)
{
	UBYTE bootargs[4096];
	
	if (GetPropStr(bootargs, "/chosen", "bootargs"))
	{
		UBYTE buffer[128];
		
		PrintSize(buffer, 
			GetField(bootargs, ".mem_size=", 16), FALSE);
		
		printf("Memory Size    : %s\n", buffer);
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetSDClock()
 ** 
 **********************************************************/

ULONG GetSDClock(void)
{
	UBYTE bootargs[4096];
	
	if (GetPropStr(bootargs, "/chosen", "bootargs"))
	{
		UBYTE buffer[128];
		
		ULONG clock = GetField(bootargs, "sd.clock=", 10);
		
		if (clock == 0)
		{
			// not defined, use default.
			clock = 50;
		}
		
		PrintFreq(buffer, clock * 1000000, FALSE);
		
		printf("SD-Clock rate  : %s\n", buffer);
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetTurbo()
 ** 
 **********************************************************/

ULONG GetTurbo(void)
{
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG t;
		
		mbox_get_turbo(0, &t);
		
		printf("Turbo-Mode     : %s\n", t == 0 ? "Disabled" : "Enabled");
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetVC4Mem()
 ** 
 **********************************************************/

ULONG GetVC4Mem(void)
{
	ULONG addr = GetPropInt("/emu68", "vc4-mem");
	
	printf("VC4-Memory     : 0x%08lx\n", addr);
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetClockRate()
 ** 
 **********************************************************/

ULONG GetClockRate(void)
{
	ULONG result = 0;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		// ARM clock id
		
		mbox_get_clock_rate(3, &result, 0, 0, 0);
		
		result /= 1000000;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetClockRates()
 ** 
 **********************************************************/

ULONG GetClockRates(void)
{
	PrintTitle("Raspberry Pi clock information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG id;
		
		static STRPTR unknown = "Unknown";
		
		static STRPTR names[] = {
			"RESERVED",
			"EMMC",
			"UART", 
			"ARM",
			"CORE",
			"V3D",
			"H264",
			"ISP",
			"SDRAM",
			"PIXEL",
			"PWM",
			"HEVC",
			"EMMC2",
			"M2MC",
			"PIXEL_BVB" };
		
		printf("| %-2s | %-9s | %-5s | %-9s | %-9s | %-9s | %-9s |\n", 
			"Id",
			"Clock",
			"State",
			"Rate",
			"Measured",
			"Minimum",
			"Maximum");
		
		printf("+%s+%s+%s+%s+%s+%s+%s+\n", 
			"----",
			"-----------",
			"-------",
			"-----------",
			"-----------",
			"-----------",
			"-----------");
		
		for (id = 0x0; id < 32; id++)
		{
			ULONG state;
			
			// Clock exists ?
			
			mbox_get_clock_state(id, &state);
			
			if (((state >> 1) & 1) == 0) 
			{
				ULONG a, b, c, d;
				UBYTE str[4][128];
				
				mbox_get_clock_rate(id, &a, &b, &c, &d);
				
				PrintFreq(str[0], a, TRUE);
				PrintFreq(str[1], b, TRUE);
				PrintFreq(str[2], c, TRUE);
				PrintFreq(str[3], d, TRUE);
				
				printf("| %02ld | %-9s | %-5s | %s | %s | %s | %s |\n", 
					id,
					(id < 15) ? names[id] : unknown,
					(state & 1) ? "On" : "Off", // state
					str[0],   // rate
					str[1],   // rate measured
					str[2],   // rate minimum
					str[3]);  // rate maximum
			}
		}
		
		printf("\n");
	}
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetCounters()
 ** 
 **********************************************************/

ULONG GetCounters(void)
{
	APTR oldSysStack;
	
	ULONG cnt_frq;
	ULONG cnt_val_lo, cnt_val_hi;
	ULONG cnt_68k_lo, cnt_68k_hi;
	ULONG cnt_arm_lo, cnt_arm_hi;
	
	UBYTE buffer[128];
	
	PrintTitle("Emu68 counters information:");
	
	if (oldSysStack = SuperState())
	{
		cnt_frq    = asm_cnt_frq();
		cnt_val_lo = asm_cnt_val_lo();
		cnt_val_hi = asm_cnt_val_hi();
		cnt_68k_lo = asm_cnt_68k_lo();
		cnt_68k_hi = asm_cnt_68k_hi();
		cnt_arm_lo = asm_cnt_arm_lo();
		cnt_arm_hi = asm_cnt_arm_hi();
		
		UserState(oldSysStack);
	}
	
	PrintFreq(buffer, cnt_frq, FALSE);
	
	printf("Counter frequency        : %s\n", buffer);
	printf("Counter running value    : %04lu.%010lu ticks\n",  cnt_val_hi, cnt_val_lo);
	printf("Counter executed M68k    : %04lu.%010lu instr.\n", cnt_68k_hi, cnt_68k_lo);
	printf("Counter executed ARM     : %04lu.%010lu instr.\n", cnt_arm_hi, cnt_arm_lo);
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetDebug()
 ** 
 **********************************************************/

ULONG GetDebug(void)
{
	APTR oldSysStack;
	
	ULONG dbg_ctrl;
	ULONG dbg_adr1;
	ULONG dbg_adr2;
	
	PrintTitle("Emu68 debug information:");
	
	if (oldSysStack = SuperState())
	{
		dbg_ctrl = asm_dbg_ctrl();
		dbg_adr1 = asm_dbg_addr_lo();
		dbg_adr2 = asm_dbg_addr_hi();
		
		UserState(oldSysStack);
	}
	
	printf("Debug control            : 0x%08lx\n", (dbg_ctrl));
	printf("Debug verbosity          : %s\n",      (dbg_ctrl     ) & 3 ? "Enabled" : "Disabled");
	printf("Debug disassemble        : %s\n",      (dbg_ctrl >> 2) & 1 ? "Enabled" : "Disabled");
	printf("Debug low address        : 0x%08lx\n", (dbg_adr1));
	printf("Debug high address       : 0x%08lx\n", (dbg_adr2));
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetDMAChannels()
 ** 
 **********************************************************/

ULONG GetDMAChannels(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Raspberry Pi DMA information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG id, mask, addr;
		
		mbox_get_dma_channels(&mask);
		
		printf("DMA mask : 0x%08lx\n\n", mask);
		
		printf("| %-2s | %-9s | %-8s | %-10s |\n", 
			"Id",
			"Channel",
			"Status",
			"Address");
		
		printf("+%s+%s+%s+%s+%s\n", 
			"----",
			"-----------",
			"----------",
			"------------");
		
		// DMA00_BASE
		
		addr = 0x7E007000;
		
		for (id = 0; id < 15; id++, addr += 0x100)
		{
			printf("| %02ld | %-9s | %-8s | 0x%08lx |\n", 
				id, "Unknown", mask & (1 << id) ? "Enabled" : "Disabled", addr);
		}
		
		// DMA15_BASE
		
		addr = 0x7EE05000;
		
		for (; id < 16; id++, addr += 0x100)
		{
			printf("| %02ld | %-9s | %-8s | 0x%08lx |\n", 
				id, "Unknown", mask & (1 << id) ? "Enabled" : "Disabled", addr);
		}
		
		printf("\n");
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetEDID()
 ** 
 **********************************************************/

ULONG GetEDID(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Raspberry Pi EDID information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG  block  = 0;
		STRPTR buffer = 0;
		
		while (!mbox_get_edid_block(block++, &buffer))
		{
			ULONG i, j;
			
			for (i = 0; i < 128; i+=16)
			{
				for (j = 0; j < 16; j++)
				{
					printf("%02lx ", buffer[i+j]);
				}
				
				printf("\n");
			}
			
			printf("\n");
			
		}
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetModules()
 ** 
 **********************************************************/

#define MAXMODULES (9)

ULONG GetModules(void)
{
	ULONG i;
	
	Module Modules[] = {
		{ MODULE_DEVICETREE, "emu68/idstring"     , 0, 0, 0, 0 },
		{ MODULE_LIBRARY,    "exec.library"       , 0, 0, 0, 0 },
		{ MODULE_LIBRARY,    "workbench.library"  , 0, 0, 0, 0 },
		{ MODULE_LIBRARY,    "rtg.library"        , 0, 0, 0, 0 },
		{ MODULE_RESOURCE,   "devicetree.resource", 0, 0, 0, 0 },
		{ MODULE_DEVICE,     "brcm-sdhc.device"   , 0, 0, 0, 0 },
		{ MODULE_LIBRARY,    "68040.library"      , 0, 0, 0, 0 },
		{ MODULE_LIBRARY,    "emu68-vc4.card"     , 0, 0, 0, 0 },
		{ MODULE_MSGPORT,    "Emu68 VC4"          , 0, 0, 0, 0 },
	};
	
	PrintTitle("Emu68 modules information:");
	
	printf("| %-9s | %-20s | %-3s | %-4s | %-32s\n", 
		"Address",
		"Module",
		"Ver",
		"Rev.",
		"Description");
	
	printf("+%s+%s+%s+%s+%s\n", 
		"-----------",
		"----------------------",
		"-----",
		"------",
		"--------------------------------------");
	
	if (Modules_Load(Modules, MAXMODULES-1))
	{
		/*
		for (i = 0; i < MAXMODULES; i++)
		{
			Module m = Modules[i];
			
			if (m.addr != NULL)
			{
				printf("| $%08lx | %-20s | %3ld | %4ld | %s\n", 
					m.addr, m.name, m.ver, m.rev, m.idString);
			}
			else
			{
				printf("| $%08lx | %-20s | %3ld | %4ld | %s\n", 
					0, m.name, 0, 0, "NOT LOADED");
			}
		}
		*/
	}
	
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetExpansion()
 ** 
 **********************************************************/

ULONG GetExpansion(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("System expansion list:");
	
	if (ExpansionBase = (struct ExpansionBase *)OpenLibrary(EXPANSIONNAME, 0L))
	{
		struct ConfigDev* cd = NULL;
		
		printf("| %-9s | %-4s | %-6s | %-4s | %-16s | %-34s |\n", 
			"Address",
			"Type",
			"MID",
			"PID",
			"Manufacturer",
			"Product description");
		
		printf("+%s+%s+%s+%s+%s+%s+\n", 
			"-----------",
			"------",
			"--------",
			"------",
			"------------------",
			"------------------------------------");
		
		while (cd = FindConfigDev(cd, -1, -1))
		{
			UBYTE boardManu[256];
			UBYTE boardName[256];
			UBYTE boardSize[256];
			UBYTE boardType[256];
			
			sprintf(boardManu, "%s", "Unknown");
			sprintf(boardName, "%s", "Unknown");
			sprintf(boardSize, "%s", "Unknown");
			sprintf(boardType, "%s", "Unknown");
			
			if (cd->cd_Rom.er_Manufacturer == EMU68_MANUFACTURER)
			{
				sprintf(boardManu, "%s", "Michal Schulz");
				
				switch(cd->cd_Rom.er_Product)
				{
				case EMU68_PRODUCT_SUPPORT:
					sprintf(boardName, "%s", "Zorro III Emu68 support");
					break;
				case EMU68_PRODUCT_RAM:
					sprintf(boardName, "%s", "Zorro II RAM expansion");
					break;
				case EMU68_PRODUCT_DEVICETREE:
					sprintf(boardName, "%s", "Zorro III Device Tree");
					break;
				case EMU68_PRODUCT_SDHC:
					sprintf(boardName, "%s", "Zorro III Broadcom SDHC controller");
					break;
				}
			}
			
			if ((cd->cd_Rom.er_Type & ERT_ZORROII) == ERT_ZORROII)
			{
				sprintf(boardType, "%s", "Z2");
			}
			
			if ((cd->cd_Rom.er_Type & ERT_ZORROII) == ERT_ZORROIII)
			{
				sprintf(boardType, "%s", "Z3");
			}
			
			PrintSize(boardSize, cd->cd_BoardSize, TRUE);
			
			printf("| $%08lx | %-4s | 0x%04x | 0x%02x | %-16s | %-34s |\n",
				cd->cd_BoardAddr,
				boardType,
				cd->cd_Rom.er_Manufacturer,
				cd->cd_Rom.er_Product,
				boardManu,
				boardName);
		}
		
		CloseLibrary((struct Library *)ExpansionBase);
		
		result = RETURN_OK;
	}
	
	printf("\n");
	
	return (result);
}

/**********************************************************
 ** 
 ** GetFirmware()
 ** 
 **********************************************************/

ULONG GetFirmware(void)
{
	ULONG result = RETURN_WARN;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		LONG t;
		UBYTE sbuf[128];
		
		mbox_get_firmware_revision((ULONG*)&t);
		strftime(sbuf, 128, "%b %d, %Y %H:%M:%S", gmtime(&t));
		printf("Firmware       : %s\n", sbuf);
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetJIT()
 ** 
 **********************************************************/

ULONG GetJIT(void)
{
	APTR oldSysStack;
	
	ULONG jit_ctrl, jit_size, jit_free;
	ULONG jit_cmiss, jit_sfthresh, jit_count;
	ULONG jit_flush, jit_loop,jit_range,jit_depth;
	
	UBYTE buf1[128];
	UBYTE buf2[128];
	
	PrintTitle("Emu68 JIT information:");
	
	if (oldSysStack = SuperState())
	{
		jit_ctrl     = asm_jit_ctrl();
		jit_size     = asm_jit_size();
		jit_free     = asm_jit_free();
		jit_cmiss    = asm_jit_cmiss();
		jit_sfthresh = asm_jit_sfthresh();
		jit_count    = asm_jit_count();
		
		UserState(oldSysStack);
	}
	
	jit_flush = ((jit_ctrl >>  0) & 0x0001);
	jit_loop  = ((jit_ctrl >>  4) & 0x000f);
	jit_range = ((jit_ctrl >>  8) & 0xffff);
	jit_depth = ((jit_ctrl >> 24) & 0x00ff);
	
	if (jit_loop  == 0) jit_loop  = 16;
	if (jit_depth == 0) jit_depth = 256;
	
	PrintSize(buf1, jit_size, FALSE);
	PrintSize(buf2, jit_size - jit_free, FALSE);
	
	printf("JIT control              : 0x%08lx\n", jit_ctrl);
	printf("JIT cache size           : %s (Used: %s)\n", buf1, buf2);
	printf("JIT cache misses         : %ld\n",     jit_cmiss);
	printf("JIT instruction depth    : %ld\n",     jit_depth);
	printf("JIT inlining range       : %ld\n",     jit_range);
	printf("JIT inline loop count    : %ld\n",     jit_loop);
	printf("JIT use soft flush       : %s\n",      jit_flush ? "Enabled" : "Disabled");
	printf("JIT soft flush threshold : %ld\n",     jit_sfthresh);
	printf("JIT units                : %ld\n",     jit_count);
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetLED()
 ** 
 **********************************************************/

ULONG GetLED(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Raspberry Pi LED information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG i;
		
		static ULONG pins[]  = {
			LED_PIN_POWER,
			LED_PIN_STATUS
		};
		
		static STRPTR names[] = {
			"Power  LED",
			"Status LED"
		};
		
		printf("| %-3s | %-12s | %-7s |\n", 
			"Id",
			"Unit",
			"Enabled");
		
		printf("+%s+%s+%s+\n", 
			"-----",
			"--------------",
			"---------");
		
		for (i = 0; i < 2; i++)
		{
			ULONG status;
			
			mbox_get_led_status(pins[i], &status);
			
			if (status == 0x80000000)
			{
				printf("| %03ld | %-12s | %-7s |\n", 
					pins[i], 
					names[i], 
					"n/a");
			}
			else
			{
				printf("| %03ld | %-12s | %-7s |\n", 
					pins[i], 
					names[i], 
					status == 0 ? "Yes" : "No");
			}
		}
		
		result = RETURN_OK;
	}
	
	printf("\n");
	
	return (result);
}

/**********************************************************
 ** 
 ** GetMemList()
 ** 
 **********************************************************/

ULONG GetMemList(void)
{
	struct MemHeader * mh;
	
	PrintTitle("AmigaOS Exec memory list:");
	
	printf("| %-9s | %-16s | %-4s | %-9s | %-9s | %-5s | %-10s |\n", 
		"Address",
		"Name",
		"Pri",
		"Lower",
		"Upper",
		"Attrs",
		"Size");
	
	printf("+%s+%s+%s+%s+%s+%s+%s+\n", 
		"-----------",
		"------------------",
		"------",
		"-----------",
		"-----------",
		"-------",
		"------------");
	
	for (
		mh = (struct MemHeader *)((struct List *)(&SysBase->MemList))->lh_Head;
		((struct Node *)mh)->ln_Succ;
		mh = (struct MemHeader *)((struct Node *)mh)->ln_Succ)
	{
		UBYTE sbuf[128];
		
		PrintSize(sbuf, (ULONG)mh->mh_Upper - (ULONG)mh, TRUE);
		
		printf("| $%08lx | %-16s | %4ld | $%08lx | $%08lx | $%04lx | %10s |\n", 
			mh,
			mh->mh_Node.ln_Name,
			mh->mh_Node.ln_Pri,
			mh->mh_Lower,
			(ULONG)mh->mh_Upper,
			mh->mh_Attributes,
			sbuf);
	}
	
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetAmigaChipId()
 ** 
 **********************************************************/

ULONG GetAmigaChipId(void)
{
	return (ULONG)(((*(volatile UWORD*)0xDFF004) & 0x7f00) >> 8);
}
	
/**********************************************************
 ** 
 ** GetAmigaChip()
 ** 
 **********************************************************/

ULONG GetAmigaChip(void)
{
	STRPTR s;
	ULONG id = GetAmigaChipId();
	
	switch (id)
	{
	case 0x00:
		s = "OCS PAL Agnus 8367 or 8371 (512K)";
		break;
	case 0x10:
		s = "OCS NTSC Agnus 8361 (256K) or 8370 (512K)";
		break;
	case 0x20:
		s = "ECS PAL Agnus 8372 (1MB)";
		break;
	case 0x30:
		s = "ECS NTSC Agnus 8372 (1MB)";
		break;
	case 0x21:
		s = "ECS PAL Agnus 8372 (2MB)";
		break;
	case 0x31:
		s = "ECS NTSC Agnus 8372 (2MB)";
		break;
	case 0x22:
		s = "AGA PAL Alice 8374 (2MB)";
		break;
	case 0x32:
		s = "AGA NTSC Alice 8374 (2MB)";
		break;
	case 0x23:
		s = "AGA PAL Alice 8374 (2MB)";
		break;
	case 0x33:
		s = "AGA NTSC Alice 8374 (2MB)";
		break;
	default:
		s = "Unknown";
		break;
	}
	
	printf("Agnus/Alice    : %s\n", s);
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetAmigaDeniseId()
 ** 
 **********************************************************/

ULONG GetAmigaDeniseId(ULONG *revision)
{
	// custom.h -> Custom.deniseid
	
	UWORD id = *(volatile UWORD*)0xDFF07C;
	
	UBYTE lo = (id & 0xff);
	
	if (revision != NULL)
	{
		*revision = (15 - ((lo >> 4) & 15));
	}
	
	if ((lo & 1) == 1)
	{
		if (((lo >> 1) & 1) == 0)
		{
			return (1); // ECS
		}
		
		if (((lo >> 2) & 1) == 0)
		{
			UBYTE hi = (id >> 8) & 0xff;
			
			if (hi != 0)
			{
				return (2); // AGA (A4000)
			}
			
			return (1); // AGA (A1200)
		}
	}
	
	return (0); // OCS
}

/**********************************************************
 ** 
 ** GetAmigaDenise()
 ** 
 **********************************************************/

ULONG GetAmigaDenise(void)
{
	STRPTR s;
	ULONG r = 0;
	ULONG id = GetAmigaDeniseId(&r);
	
	switch (id)
	{
	case 0:
		s = "OCS Denise 8362";
		break;
	case 1:
		s = "ECS Denise 8373";
		break;
	case 2:
		s = "AGA Lisa 4203 A1200";
		break;
	case 3:
		s = "AGA Lisa 4203 A4000";
		break;
	default:
		s = "Unknown";
		break;
	}
	
	printf("Denise/Lisa    : %s (rev: %ld)\n", s, r);
	
	return (RETURN_OK);
}
/*
ULONG GetAmigaDeniseId(void)
{
	STRPTR s;
	UBYTE lo, hi;
	UWORD id = *(volatile UWORD*)0xDFF07C;
	
	lo = id & 0xff;
	hi = (id >> 8) & 0xff;
	
	s = "OCS Denise 8362";
	
	if ((lo & 1) == 1)
	{
		if (((lo >> 1) & 1) == 0)
		{
			s = "ECS Denise 8373";
		}
		
		if (((lo >> 2) & 1) == 0)
		{
			if (hi != 0)
			{
				s = "AGA Lisa 4203 A4000";
			}
			else
			{
				s = "AGA Lisa 4203 A1200";
			}
		}
	}
	
	printf("Denise/Lisa    : %s (rev: %ld)\n", 
		s, 15 - ((lo >> 4) & 15));
	
	return (id);
}
*/
/**********************************************************
 ** 
 ** GetAmigaDisplayFlags()
 ** 
 **********************************************************/

ULONG GetAmigaDisplayFlags(void)
{
	UBYTE s[128];
	ULONG i, j = 0;
	UWORD flags = GfxBase->DisplayFlags;
	
	STRPTR names[] = {
		"NSTC", "GENLOCK", "PAL", "TODA_SAFE", 
		"REALLY_PAL", "LPEN_SWAP_FRAMES"
	};
	
	memset(s, 0, 128);
	
	for (i = 0; i < 6; i++)
	{
		if (flags & (1 << i))
		{
			if (j > 0)
			{
				strcat(s, ", ");
			}
			
			strcat(s, names[i]);
			j++;
		}
	}
	
	printf("Display-Flags  : %s (0x%04x)\n", 
		s, flags);
	
	return ((ULONG)flags);
}

/**********************************************************
 ** 
 ** GetAmigaPaulaRev()
 ** 
 **********************************************************/

ULONG GetAmigaPaulaRev(void)
{
	UWORD id = *(volatile UWORD*)0xDFF016;
	
	ULONG rev = (id & 0xfe) > 1;
	
	STRPTR names[] = {
		"Paula 8364",
		"Arne SAGA",
		"Unknown",
	};
	
	printf("Audio Chip     : %s (rev: %ld)\n", 
		rev < 2 ? names[rev] : names[2],
		rev);
	
	return (rev);
}

/**********************************************************
 ** 
 ** GetAmigaAkiko()
 ** 
 **********************************************************/

ULONG GetAmigaAkiko(void)
{
	UWORD id = *(volatile UWORD*)0xB80002;
	
	printf("Akiko Chip     : %s\n", 
		(id == 0xCAFE) ? "Detected" : "Not detected");
	/*
	printf("Akiko Chip   : %s (0x%04x)\n", 
		(id == 0xCAFE) ? "Detected" : "Not detected", id);
	*/
	if (id == 0xCAFE)
	{
		printf("Akiko C2P      : %s (0x%08lx)\n", 
			GfxBase->ChunkyToPlanarPtr ? "Initialized" : "Uninitialized",
			GfxBase->ChunkyToPlanarPtr);
	}
	
	return (id);
}

/**********************************************************
 ** 
 ** GetAmigaClocks()
 ** 
 **********************************************************/

ULONG GetAmigaClocks(void)
{
	ULONG eclk = SysBase->ex_EClockFrequency;
	
	printf("E-Clock        : %lu.%05lu Hz (%s)\n",
		( eclk /  100000 ),
		( eclk %  100000 ),
		( eclk == 709379 ) ? "PAL" : "NTSC");
	
	printf("Vertical-Blank : %lu Hertz\n",
		SysBase->VBlankFrequency);
	
	printf("Power-Supply   : %lu Hertz\n",
		SysBase->PowerSupplyFrequency);
	
	return (eclk);
}

/**********************************************************
 ** 
 ** GetAmigaMemory()
 ** 
 **********************************************************/

ULONG GetAmigaMemory(void)
{
	UBYTE c[128];
	UBYTE f[128];
	
	ULONG tc = AvailMem(MEMF_TOTAL | MEMF_CHIP);
	ULONG tf = AvailMem(MEMF_TOTAL | MEMF_FAST);
	
	PrintSize(c, tc, FALSE);
	PrintSize(f, tf, FALSE);
	
	printf("Chip memory    : %s\n", c);
	printf("Fast memory    : %s\n", f);
	
	return (tc + tf);
}

/**********************************************************
 ** 
 ** GetAmigaCPU()
 ** 
 **********************************************************/

ULONG GetAmigaCPU(void)
{
	APTR  oldSysStack;
	ULONG attn, cpu, fpu, cacr, vbr;
	UBYTE cpuModel[16];
	UBYTE fpuModel[16];
	UBYTE cpuRate[128];
	UBYTE fpuRate[128];
	
	// Model
	
	sprintf(cpuModel, "%s", "MC68000");
	sprintf(fpuModel, "%s", "Absent");
	
	attn = SysBase->AttnFlags;
	
	if (attn & AFF_68010) sprintf(cpuModel, "%s", "MC68010");
	if (attn & AFF_68020) sprintf(cpuModel, "%s", "MC68020");
	if (attn & AFF_68030) sprintf(cpuModel, "%s", "MC68030");
	if (attn & AFF_68040) sprintf(cpuModel, "%s", "MC68040");
	if (attn & AFF_68060) sprintf(cpuModel, "%s", "MC68060");
	if (attn & AFF_68080) sprintf(cpuModel, "%s", "MC68080");
	if (attn & AFF_68881) sprintf(fpuModel, "%s", "MC68881");
	if (attn & AFF_68882) sprintf(fpuModel, "%s", "MC68882");
	if (attn & AFF_FPU40) sprintf(fpuModel, "%s", "MC68040");
	
	// Frequency
	
	Forbid();
	
	cpu = asm_cpu_rate();
	
	if ((attn & AFF_68881) || (attn & AFF_68882) || (attn & AFF_FPU40))
	{
		fpu = asm_fpu_rate();
	}
	
	Permit();
	
	// CPU special purpose registers
	
	if (oldSysStack = SuperState())
	{
		cacr = asm_cpu_cacr();
		vbr  = asm_cpu_vbr();
		
		UserState(oldSysStack);
	}
	
	// Printout
	
	PrintFreq(cpuRate, cpu, FALSE);
	PrintFreq(fpuRate, fpu, FALSE);
	
	printf("CPU model      : %s @ %s\n", cpuModel, cpuRate);
	printf("FPU model      : %s @ %s\n", fpuModel, fpuRate);
	
	printf("CPU VBR        : 0x%08lx (Located in %s ram)\n", 
		vbr, 
		vbr ? "Fast" : "Chip");
	
	printf("CPU CACR       : 0x%08lx (ICache: %s) (DCache: %s)\n", 
		cacr, 
		cacr & (1<<31) ? "On" : "Off", 
		cacr & (1<<15) ? "On" : "Off");
	
	return (cpu);
}

/**********************************************************
 ** 
 ** GetAmiga()
 ** 
 **********************************************************/

ULONG GetAmiga(void)
{
	PrintTitle("Amiga hardware information:");

	GetAmigaCPU();
	GetAttnFlags();
	GetAmigaClocks();
	GetAmigaDisplayFlags();
	GetAmigaChip();
	GetAmigaDenise();
	GetAmigaPaulaRev();
	GetAmigaAkiko();
	GetAmigaMemory();
	
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetFrameBuffer()
 ** 
 **********************************************************/

ULONG GetFrameBuffer(void)
{
	ULONG pw, ph, vw, vh, de, po, am, pi;
	
	mbox_get_frame_buffer(
		&pw,  // physical width
		&ph,  // physical height
		&vw,  // virtual width
		&vh,  // virtual height
		&de,  // depth
		&po,  // pixel order
		&am,  // alpha mode
		&pi); // pitch
	
	printf("Physical-Size  : %ld x %ld\n", pw, ph);
	printf("Virtual-Size   : %ld x %ld\n", vw, vh);
	printf("Depth          : %ld bits per pixel\n", de);
	printf("Pixel-Order    : %s\n", po == 0 ? "BGR" : "RGB");
	printf("Alpha-Mode     : %ld\n", am);
	printf("Pitch          : %ld\n", pi);
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetBoard()
 ** 
 **********************************************************/

ULONG GetBoard(void)
{
	UBYTE buffer1[128];
	UBYTE buffer2[128];
	UBYTE bootargs[4096];
	
	PrintTitle("Raspberry Pi model information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG a, b, c, d;
		ULONG overVoltage;
		ULONG otpProgram;
		ULONG otpRead;
		ULONG usuned1;
		ULONG warranty;
		ULONG unused2;
		ULONG newFlag;
		ULONG memorySize;
		ULONG manufacturer;
		ULONG processor;
		ULONG boardType;
		ULONG boardRev;
		ULONG revision;
		
		static STRPTR type[] = {
			"1 Model A",             // 0x00
			"1 Model B",             // 0x01
			"1 Model A Plus",        // 0x02
			"1 Model B Plus",        // 0x03
			"2 Model B",             // 0x04
			"Alpha",                 // 0x05
			"Compute Module 1",      // 0x06
			"2 Model A",             // 0x07
			"3 Model B",             // 0x08
			"Zero",                  // 0x09
			"Compute Module 3",      // 0x0A
			"Unknown",               // 0x0B
			"Zero W",                // 0x0C
			"3 Model B Plus",        // 0x0D
			"3 Model A Plus",        // 0x0E
			"Unknown",               // 0x0F
			"Compute Module 3 Plus", // 0x10
			"4 Model B",             // 0x11
			"Zero 2 W",              // 0x12
			"400",                   // 0x13
			"Compute Module 4",      // 0x14
			"Unknown",               // 0x15
		};
		
		static STRPTR proc[] = {
			"BCM2835",               // 0x00
			"BCM2836",               // 0x01
			"BCM2837",               // 0x02
			"BCM2711",               // 0x03
			"BCM????",               // 0x04
			"Unknown",               // 0x05
		};
		
		static STRPTR manu[] = {
			"Sony UK",               // 0x00
			"Egoman",                // 0x01
			"Embest",                // 0x02
			"Sony Japan",            // 0x03
			"Embest",                // 0x04
			"Stadium",               // 0x05
			"Unknown",               // 0x06
		};
		
		/*
		static STRPTR memo[] = {
			"256 MB",                // 0x00
			"512 MB",                // 0x01
			"1 GB",                  // 0x02
			"2 GB",                  // 0x03
			"4 GB",                  // 0x04
			"8 GB",                  // 0x05
			"Unknown",               // 0x06
		};
		*/
		
		// ------------------- -------------------
		//    0    2    9    0    2    0    e    0
		// ------------------- -------------------
		// 0000 0010 1001 0000 0010 0000 1110 0000
		// ------------------- -------------------
		// NOQu uuWu FMMM CCCC PPPP TTTT TTTT RRRR
		// ------------------- -------------------
		
		mbox_get_board_model(&a, &b);
		
		boardRev     = (b);
		overVoltage  = (b >> 31) & 0x01; // %00000001 N
		otpProgram   = (b >> 30) & 0x01; // %00000001 O
		otpRead      = (b >> 29) & 0x01; // %00000001 Q
		usuned1      = (b >> 26) & 0x07; // %00000111 uuu
		warranty     = (b >> 25) & 0x01; // %00000001 W
		unused2      = (b >> 24) & 0x01; // %00000001 u
		newFlag      = (b >> 23) & 0x01; // %00000001 F
		memorySize   = (b >> 20) & 0x07; // %00000111 MMM
		manufacturer = (b >> 16) & 0x0f; // %00001111 CCCC
		processor    = (b >> 12) & 0x0f; // %00001111 PPPP
		boardType    = (b >>  4) & 0xff; // %11111111 TTTTTTTT
		revision     = (b >>  0) & 0x0f; // %00001111 RRRR
		
		printf("Board-Revision : %lx (id: %ld)\n", 
			boardRev & 0x00ffffff, boardType);
		
		printf("Board-Model    : Raspberry Pi %s Rev 1.%ld\n", 
			boardType < 22 ? type[boardType] : type[21], revision);
		
		if (GetPropStr(buffer1, "/", "compatible"))
		{
			printf("Compatible     : %s\n", buffer1);
		}
		
		printf("Manufacturer   : %s\n", 
			manufacturer < 7 ? manu[manufacturer] : manu[6]);
		
		GetTurbo();
		
		printf("Over-Voltage   : %s\n",
			overVoltage ? "Enabled" : "Disabled");
		
		if (GetPropStr(buffer1, "/cpus/cpu@0", "compatible"))
		{
			ULONG a, b, c, d;
			mbox_get_clock_rate(3, &a, &b, &c, &d);
			PrintFreq(buffer2, a, FALSE);
			printf("Processor      : %s @ %s\n", buffer1, buffer2);
		}
		
		if (GetPropStr(bootargs, "/chosen", "bootargs"))
		{
			ULONG a, b, c, d;
			mbox_get_clock_rate(9, &a, &b, &c, &d);
			PrintFreq(buffer1, b, FALSE);
			printf("Frame-Size     : %ld x %ld @ %s\n",
				GetField(bootargs, ".fbwidth=",  10),
				GetField(bootargs, ".fbheight=", 10),
				buffer1);
		}
		
		printf("Hardware       : %s\n", 
			processor < 6 ? proc[processor] : proc[5]);
		
		GetFirmware();
		GetBootTime2();
		
		mbox_get_board_serial(&a, &b);
		printf("Serial-Number  : %08lx%08lx\n", a, b);
		
		mbox_get_board_macaddr(&a, &b);
		printf("MAC-Address    : %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", 
			(((a) & 0xff000000) >> 24),
			(((a) & 0x00ff0000) >> 16),
			(((a) & 0x0000ff00) >>  8),
			(((a) & 0x000000ff) >>  0),
			(((b) & 0xff000000) >> 24),
			(((b) & 0x00ff0000) >> 16));
		
		GetMemSize();
		
		mbox_get_memory(&a, &b, &c, &d);
		PrintSize(buffer1, b, FALSE);
		PrintSize(buffer2, d, FALSE);
		printf("ARM-Memory size: %s\n", buffer1);
		printf("VC4-Memory size: %s\n", buffer2);
		
		GetSDClock();
		
		printf("Warranty       : %s\n", warranty    ? "Voided" : "Ok");
//		printf("OTP-Program    : %s\n", otpProgram  ? "Yes"    : "No");
//		printf("OTP-Read       : %s\n", otpRead     ? "Yes"    : "No");
	}
	
	printf("\n");
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** GetPowerState()
 ** 
 **********************************************************/

ULONG GetPowerState(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Raspberry Pi power information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG id, a, b;
		
		static STRPTR device[] = {
			"SD Card",  // 0x00
			"UART0",    // 0x01
			"UART1",    // 0x02
			"USB HCD",  // 0x03
			"I2C0",     // 0x04
			"I2C1",     // 0x05
			"I2C2",     // 0x06
			"SPI",      // 0x07
			"CCP2TX",   // 0x08
			"Unknown",  // 0x09
		};
		
		printf("| %-2s | %-11s | %-7s | %-7s |\n", 
			"Id",
			"Unit",
			"Powered",
			"Timing");
		
		printf("+%s+%s+%s+%s+\n", 
			"----",
			"-------------",
			"---------",
			"---------");
		
		for (id = 0; id < 16; id++)
		{
			mbox_get_power_state(id, &a, &b);
			
			if (1 - ((a > 1) & 1)) // Exists ?
			{
				printf("| %02ld | %-11s | %-7s | %4ld ms |\n", 
					id,
					id < 9 ? device[id] : device[9],
					(a & 1) ? "Yes" : "No",
					b);
			}
		}
		
		printf("\n");
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetTemperature()
 ** 
 **********************************************************/

ULONG GetTemperature(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Raspberry Pi temperature (celcius) information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG id;
		
		static STRPTR names[] = {
			"Core",     // 0x00 Core
			"Unknown"   // 0x01 Unknown
		};
		
		printf("| %-2s | %-11s | %-8s | %-8s |\n", 
			"Id",
			"Unit",
			"Measured",
			"Max safe");
		
		printf("+%s+%s+%s+%s+\n", 
			"----",
			"-------------",
			"----------",
			"----------");
		
		for (id = 0; id < 1; id++)
		{
			ULONG a, b;
			
			mbox_get_temperature(id, &a, &b);
			
			a = (a + 50) / 100;
			b = (b + 50) / 100;
			
			printf("| %02ld | %-11s | %4ld.%ld C | %4ld.%ld C |\n", 
				id,                            // temp id
				id < 1 ? names[id] : names[1], // temp name
				a / 10, a % 10,                // temp
				b / 10, b % 10);               // temp max
		}
		
		printf("\n");
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetVoltage()
 ** 
 **********************************************************/

ULONG GetVoltage(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Raspberry Pi voltage (millivolts) information:");
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG id, a, b;
		
		static STRPTR names[] = { 
			"Reserved",    // 0x00 Reserved
			"Core",        // 0x01 Core
			"SD-Ram Ctrl", // 0x02 SDRam Controller
			"SD-Ram Phy",  // 0x03 SDRam P
			"SD-Ram I/O",  // 0x04 SDRam Input/Output
			"Unknown"      // 0x05 Unknown
		};

		mbox_get_board_model(&a, &b);
		
		printf("\033uNote\033n: your RPi is %soverclocked.\n\n", 
			((b >> 31) & 1) ? "" : "not ");
		
		printf("| %-2s | %-11s | %-9s | %-9s | %-9s |\n", 
			"Id",
			"Unit",
			"Measured",
			"Min/idle",
			"Max safe");
		
		printf("+%s+%s+%s+%s+%s+\n", 
			"----",
			"-------------",
			"-----------",
			"-----------",
			"-----------");
		
		for (id = 1; id < 5; id++)
		{
			ULONG a, b, c;
			
			mbox_get_voltage(id, &a, &b, &c);
			
			printf("| %02ld | %-11s | %ld.%ld mV | %ld.%ld mV | %ld.%ld mV |\n", 
				id,                            // voltage id
				id < 5 ? names[id] : names[5], // voltage name
				a / 1000, (a % 1000) / 100,    // voltage
				b / 1000, (b % 1000) / 100,    // voltage min
				c / 1000, (c % 1000) / 100);   // voltage max
		}
		
		printf("\n");
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** SetClockRate()
 ** 
 **********************************************************/

ULONG SetClockRate(ULONG rate)
{
	ULONG result = RETURN_WARN;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG a, b, c, d;
		UBYTE sbuf[128];
		
		mbox_get_clock_rate(
			3,   // clock id (ARM)
			&a,  // clock rate (in Hz)
			&b,  // clock rate measured (in Hz)
			&c,  // clock rate minimum (in Hz)
			&d); // clock rate maximum (in Hz)
		
		rate *= 1000000;
		
		if (rate < c) rate = c;
		if (rate > d) rate = d;
		
		mbox_set_clock_rate(
			3,    // clock id (ARM)
			rate, // clock rate (in Hz)
			0);   // do not skip setting turbo
		
		PrintFreq(sbuf, rate, FALSE);
		
		printf("ARM clock rate set to %s.\n", sbuf);
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** SetLED()
 ** 
 **********************************************************/

ULONG SetLED(ULONG status)
{
	ULONG result = RETURN_WARN;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		if (status > 1)
			status = 1;
		
		mbox_set_led_status(
			LED_PIN_POWER,
			status ? 0 : 1);
		
		printf("Power LED (Pin %ld) set to %ld.\n", 
			LED_PIN_POWER,
			status);
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** SetTurbo()
 ** 
 **********************************************************/

ULONG SetTurbo(ULONG level)
{
	ULONG result = RETURN_WARN;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		if (level > 1)
			level = 1;
		
		mbox_set_turbo(0, level);
		
		if (level)
		{
			ULONG a, b, c, d;
			
			mbox_get_clock_rate(
				3,       // clock id (ARM)
				&a,      // clock rate (in Hz)
				&b,      // clock rate measured (in Hz)
				&c,      // clock rate minimum (in Hz)
				&d);     // clock rate maximum (in Hz)
			
			if (d != a)
			{
				mbox_set_clock_rate(
					3,   // clock id (ARM)
					d,   // clock rate (in Hz)
					0);  // do not skip setting turbo
			}
		}
		
		printf("Turbo mode set to %ld.\n", level);
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** SetBlank()
 ** 
 **********************************************************/

ULONG SetBlank(ULONG state)
{
	ULONG result = RETURN_WARN;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		mbox_set_alpha_mode(state);
	//	mbox_set_screen_blank(state);
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** SetGamma()
 ** 
 **********************************************************/

ULONG SetGamma(UBYTE value)
{
	ULONG result = RETURN_WARN;
	
	if (!MailBox)
	{
		mbox_init();
	}
	
	if (MailBox)
	{
		ULONG i;
		ULONG d = 1;
		UBYTE table[768];
		
		for (i = 0; i < 768; i++)
		{
			table[i] = (value);
		}
		
		CacheClearE(table, 768, CACRF_ClearD);
		
		mbox_set_screen_gamma(d, ((ULONG)table) | 0xC0000000);
		
		printf("Screen Gamma updated to display #%ld.\n", d);
		
		result = RETURN_OK;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** HardReset()
 ** 
 **********************************************************/

ULONG HardReset(void)
{
	// /!\  CAUTION  /!\  HARD REBOOT  /!\
	// /!\  CAUTION  /!\  HARD REBOOT  /!\
	// /!\  CAUTION  /!\  HARD REBOOT  /!\
	
	VC4_FullReset();
	
	// /!\  CAUTION  /!\  HARD REBOOT  /!\
	// /!\  CAUTION  /!\  HARD REBOOT  /!\
	// /!\  CAUTION  /!\  HARD REBOOT  /!\
	
	return (RETURN_WARN);
}

/**********************************************************
 ** 
 ** SetAttnFlags()
 ** 
 **********************************************************/

ULONG SetAttnFlags(STRPTR value)
{
	ULONG result = RETURN_WARN;
	ULONG newval = strtol(value, NULL, 16);
	
	printf("\n");
	printf("Please note, this feature is for experienced users only.\n");
	printf("This will very probably make your OS unstable. If so, reboot.\n");
	printf("\n");
	printf("Examples:\n");
	printf("0x8003 => 10,20,PRIV.\n");
	printf("0x800f => 10,20,30,40,PRIV.\n");
	printf("0x804f => 10,20,30,40,FPU40,PRIV.\n");
	printf("0x807f => 10,20,30,40,881,882,FPU40,PRIV (Default).\n");
	printf("\n");
	printf("Your new settings:\n");
	
	if (newval == 0)
	{
		printf("Incorrect value (requires Hexadecimal, eg. 0x807f).'\n");
	}
	else
	{
		newval = (newval & 0xffff);
		
		SysBase->AttnFlags = newval;
		
		printf("AmigaOS Exec -> AttnFlags now is 0x%04lx.\n", newval);
		
		result = RETURN_OK;
	}
	
	printf("\n");
	
	return (result);
}

/**********************************************************
 ** 
 ** GetVC4Info()
 ** 
 **********************************************************/

ULONG GetVC4Info(void)
{
	ULONG result = RETURN_WARN;
	
	PrintTitle("Emu68 VC4-drivers information:");
	
	GetVC4Mem();
	VC4_GetInfo();
	
	printf("\n");
	
	return (result);
}

/**********************************************************
 ** 
 ** Entry point
 ** 
 **********************************************************/

ULONG main(ULONG argc, char *argv[])
{
	ULONG result = RETURN_FAIL;
	
	LONG opts[OPT_COUNT];
	struct RDArgs *rdargs;
	
	if (DeviceTreeBase = (struct Library *)OpenResource("devicetree.resource"))
	{
		if (argc > 1)
		{
			memset((char *)opts, 0, sizeof(opts));
			
			if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
			{
				if (opts[OPT_HELP])
				{
					result = Help();
				}
				else
				{
					if (opts[OPT_AMIGA])
						result = GetAmiga();
					
					if (opts[OPT_BOOTARGS])
						result = GetBootArgs();
					
					if (opts[OPT_BOARD])
						result = GetBoard();
					
					if (opts[OPT_BOARDID])
						result = GetBoardId();
					
					if (opts[OPT_BOARDNAME])
						result = GetBoardName();
					
					if (opts[OPT_CHIPID])
						result = GetAmigaChipId();
					
					if (opts[OPT_CLOCKRATE])
						result = GetClockRate();
					
					if (opts[OPT_CLOCKRATES])
						result = GetClockRates();
					
					if (opts[OPT_CMDLINE])
						result = GetCmdLine();
					
					if (opts[OPT_COUNTERS])
						result = GetCounters();
					
					if (opts[OPT_CPU])
						result = GetCPU();
					
					if (opts[OPT_DEBUG])
						result = GetDebug();
					
					if (opts[OPT_DETECT])
						result = GetDetect();
					
					if (opts[OPT_DENISEID])
						result = GetAmigaDeniseId(NULL);
					
					if (opts[OPT_DMA])
						result = GetDMAChannels();
					
					if (opts[OPT_EDID])
						result = GetEDID();
					
					if (opts[OPT_MODULES])
						result = GetModules();
					
					if (opts[OPT_EXPANSION])
						result = GetExpansion();
					
					if (opts[OPT_HARDRESET])
						result = HardReset();
					
					if (opts[OPT_IDSTRING])
						result = GetIdString();
					
					if (opts[OPT_JIT])
						result = GetJIT();
					
					if (opts[OPT_LED])
						result = GetLED();
					
					if (opts[OPT_MEMLIST])
						result = GetMemList();
					
					if (opts[OPT_POWERSTATE])
						result = GetPowerState();
					
					if (opts[OPT_SETBLANK])
						result = SetBlank(*(LONG *)opts[OPT_SETBLANK]);
					
					if (opts[OPT_SETATTN])
						result = SetAttnFlags((STRPTR)opts[OPT_SETATTN]);
					
					if (opts[OPT_SETCLOCKRATE])
						result = SetClockRate(*(LONG *)opts[OPT_SETCLOCKRATE]);
					
					if (opts[OPT_SETGAMMA])
						result = SetGamma(*(LONG *)opts[OPT_SETGAMMA]);
					
					if (opts[OPT_SETLED])
						result = SetLED(*(LONG *)opts[OPT_SETLED]);
					
					if (opts[OPT_SETTURBO])
						result = SetTurbo(*(LONG *)opts[OPT_SETTURBO]);
					
					if (opts[OPT_TEMPERATURE])
						result = GetTemperature();
					
					if (opts[OPT_UPTIME])
						result = GetBootTime();
					
					if (opts[OPT_VC4])
						result = GetVC4Info();
					
					if (opts[OPT_VOLTAGE])
						result = GetVoltage();
				}
				
				FreeArgs(rdargs);
			}
			else
			{
				printf("\nInvalid arguments.\n\n");
				result = Help();
			}
		}
		else
		{
			result = Help();
		}
	}
	else
	{
		printf("\nCant open devicetree.resource!\n");
	}
	
	return (result);
}

/**********************************************************
 **
 ** END
 **
 **********************************************************/
