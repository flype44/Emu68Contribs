/******************************************************************************
 * 
 * Project: Emu68Version
 * Version: 1.0 (30.11.2025)
 * Author:  Philippe CARPENTIER
 * 
 *****************************************************************************/

#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/devicetree.h>
#include "main.h"

/*****************************************************************************
 * 
 * DEFINES
 * 
 *****************************************************************************/

#define TEMPLATE "\
VERSION/N,\
REVISION/N,\
HOTFIX/N,\
SHORT/S,\
FULL/S,\
GITHASH/S,\
VARIANT/S,\
HELP/S"

enum {
	OPT_VERSION, OPT_REVISION, OPT_HOTFIX,
	OPT_SHORT,   OPT_FULL, 
	OPT_GITHASH, OPT_VARIANT,
	OPT_HELP,    OPT_COUNT
};

struct Emu68VersionStruct {
	ULONG ver_major;
	ULONG ver_minor;
	ULONG ver_patch;
};

/*****************************************************************************
 * 
 * PROTOTYPES
 * 
 *****************************************************************************/

static VOID  GetEmu68Help(VOID);
static APTR  GetEmu68Property(STRPTR);
static ULONG GetEmu68Version(LONG *);
 
/*****************************************************************************
 * 
 * GLOBALS
 * 
 *****************************************************************************/

APTR DeviceTreeBase = NULL;
extern struct ExecBase * SysBase;
extern struct DosLibrary * DOSBase;

static STRPTR VerString = VERSTRING;
static struct Emu68VersionStruct versionOld;
static struct Emu68VersionStruct * version = NULL;

/*****************************************************************************
 * 
 * GetEmu68Help()
 * 
 *****************************************************************************/

static VOID GetEmu68Help(VOID)
{
	Printf("%s\n%s\n\n%s\n", VerString + 6, TEMPLATE,
	"HELP    : Print this help\n"
	"VERSION : Check the Emu68 version number (major)\n"
	"REVISION: Check the Emu68 revision number (minor)\n"
	"HOTFIX  : Check the Emu68 hotfix number (patch)\n"
	"SHORT   : Print the Emu68 version string (short)\n"
	"FULL    : Print the Emu68 version string (full)\n"
	"GITHASH : Print the Emu68 git hash string\n"
	"VARIANT : Print the Emu68 variant string");
}

/*****************************************************************************
 * 
 * GetEmu68Property()
 * 
 *****************************************************************************/

static APTR GetEmu68Property(STRPTR propertyName)
{
	APTR property;
	
	if (property = DT_FindProperty(DT_OpenKey("/emu68"), propertyName)) {
		return (APTR)DT_GetPropValue(property);
	}
	
	return (NULL);
}

/*****************************************************************************
 * 
 * ParseEmu68IdString()
 * 
 *****************************************************************************/

static struct Emu68VersionStruct * ParseEmu68IdString(STRPTR idString)
{
	STRPTR s = (STRPTR)((ULONG)idString + 6);
	
	while (*s && *s != ' ') s++;
	
	s += 1;
	s += StrToLong(s, (LONG *)&versionOld.ver_major) + 1;
	s += StrToLong(s, (LONG *)&versionOld.ver_minor) + 1;
	s += StrToLong(s, (LONG *)&versionOld.ver_patch) + 1;
	
	return (&versionOld);
}

/*****************************************************************************
 * 
 * GetEmu68Version()
 * 
 *****************************************************************************/

static ULONG GetEmu68Version(LONG * opts)
{
	APTR value = NULL;
	
	// GET version using the new method (Emu68 >= 1.1)
	if (value = GetEmu68Property("version")) {
		version = (struct Emu68VersionStruct *)value;
	} else {
		// GET version using the old method (Emu68 < 1.1)
		if (value = GetEmu68Property("idstring")) {
			version = ParseEmu68IdString((STRPTR)value);
		} else {
			Printf("Cant open the emu68/idstring property!\n");
			return (RETURN_ERROR);
		}
	}
	
	// VERSION
	if (!opts[OPT_SHORT] && !opts[OPT_FULL] && 
		!opts[OPT_GITHASH] && !opts[OPT_VARIANT]) {
			Printf("Emu68 %ld.%ld.%ld\n", version->ver_major, 
				version->ver_minor, version->ver_patch);
	}
	
	// SHORT
	if (opts[OPT_SHORT]) {
		Printf("%ld.%ld.%ld\n", version->ver_major,
			version->ver_minor, version->ver_patch);
	}
	
	// FULL
	if (opts[OPT_FULL]) {
		if (value = GetEmu68Property("idstring")) {
			Printf("%s\n", (STRPTR)((ULONG)value + 6));
		} else {
			Printf("Cant open the emu68/idstring property!\n");
			return (RETURN_ERROR);
		}
	}
	
	// GITHASH
	if (opts[OPT_GITHASH]) {
		if (value = GetEmu68Property("git-hash")) {
			Printf("%s\n", value);
		} else {
			Printf("Cant open the emu68/githash property!\n");
			return (RETURN_ERROR);
		}
	}
	
	// VARIANT
	if (opts[OPT_VARIANT]) {
		if (value = GetEmu68Property("variant")) {
			Printf("%s\n", value);
		} else {
			Printf("Cant open the emu68/variant property!\n");
			return (RETURN_ERROR);
		}
	}
	
	// VERSION
	if (opts[OPT_VERSION]) {
		if ((*(LONG *)opts[OPT_VERSION]) > version->ver_major) {
			return (RETURN_WARN);
		}
	}
	
	// REVISION
	if (opts[OPT_REVISION]) {
		if ((*(LONG *)opts[OPT_REVISION]) > version->ver_minor) {
			return (RETURN_WARN);
		}
	}
	
	// HOTFIX
	if (opts[OPT_HOTFIX]) {
		if ((*(LONG *)opts[OPT_HOTFIX]) > version->ver_patch) {
			return (RETURN_WARN);
		}
	}
	
	// NO WARN
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** Entry point
 ** 
 **********************************************************/

ULONG main(ULONG argc, STRPTR * argv)
{
	ULONG rc;
	LONG opts[OPT_COUNT];
	struct RDArgs * rdargs;
	
	opts[OPT_VERSION ] = 0L;
	opts[OPT_REVISION] = 0L;
	opts[OPT_HOTFIX  ] = 0L;
	opts[OPT_SHORT   ] = 0L;
	opts[OPT_FULL    ] = 0L;
	opts[OPT_GITHASH ] = 0L;
	opts[OPT_VARIANT ] = 0L;
	opts[OPT_HELP    ] = 0L;
	
	if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL)) {
		if (opts[OPT_HELP]) {
			GetEmu68Help();
			rc = RETURN_OK;
		} else {
			if (DeviceTreeBase = (struct Library *)OpenResource(DEVICETREE_NAME)) {
				if (DT_OpenKey("/emu68")) {
					rc = GetEmu68Version(opts);
				} else {
					Printf("Cant open the devicetree emu68 key!\n");
					rc = RETURN_ERROR;
				}
			} else {
				Printf("Cant open " DEVICETREE_NAME "!\n");
				rc = RETURN_ERROR;
			}
		}
		FreeArgs(rdargs);
	} else {
		PutStr("Bad argument, use HELP for more information.\n");
		rc = RETURN_FAIL;
	}
	
	return (rc);
}

/*****************************************************************************
 * 
 * END OF FILE
 * 
 *****************************************************************************/
