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
VERSION/N,REVISION/N,HOTFIX/N,\
SHORT/S,FULL/S,GITHASH/S,\
VARIANT/S,HELP/S"

enum {
	OPT_VERSION, OPT_REVISION, OPT_HOTFIX,
	OPT_SHORT, OPT_FULL, OPT_GITHASH, 
	OPT_VARIANT, OPT_HELP, OPT_COUNT
};

typedef struct {
	ULONG ver_major;
	ULONG ver_minor;
	ULONG ver_patch;
} VERS3;

/*****************************************************************************
 * 
 * PROTOTYPES
 * 
 *****************************************************************************/

static VOID GetHelp(VOID);
static APTR GetProperty(APTR key, STRPTR);
static ULONG GetVersion(APTR key, LONG *);
static VERS3 * ParseIdString(STRPTR);

/*****************************************************************************
 * 
 * GLOBALS
 * 
 *****************************************************************************/

APTR DeviceTreeBase = NULL;
extern struct ExecBase * SysBase;
extern struct DosLibrary * DOSBase;

static VERS3 versionOld;
static VERS3 * version = NULL;
static CONST_STRPTR VSTRING = VERSTRING;

/*****************************************************************************
 * 
 * GetHelp()
 * 
 *****************************************************************************/

static VOID GetHelp(VOID)
{
	Printf("%s\n\n%s\n", VSTRING + 6,
	"HELP/S     : Print this help\n"
	"VERSION/N  : Check the Emu68 version (major)\n"
	"REVISION/N : Check the Emu68 revision (minor)\n"
	"HOTFIX/N   : Check the Emu68 hotfix (patch)\n"
	"SHORT/S    : Print the Emu68 version string (short)\n"
	"FULL/S     : Print the Emu68 version string (full)\n"
	"GITHASH/S  : Print the Emu68 git-hash string\n"
	"VARIANT/S  : Print the Emu68 variant string");
}

/*****************************************************************************
 * 
 * GetProperty()
 * 
 *****************************************************************************/

static APTR GetProperty(APTR key, STRPTR name)
{
	APTR property;
	
	if (property = DT_FindProperty(key, name)) {
		return (APTR)DT_GetPropValue(property);
	}
	
	return (NULL);
}

/*****************************************************************************
 * 
 * ParseIdString()
 * 
 *****************************************************************************/

static VERS3 * ParseIdString(STRPTR s)
{
	s += 6;
	
	while (*s && *s != ' ') ++s;
	
	s += StrToLong(++s, (LONG *)&versionOld.ver_major);
	s += StrToLong(++s, (LONG *)&versionOld.ver_minor);
	s += StrToLong(++s, (LONG *)&versionOld.ver_patch);
	
	return (&versionOld);
}

/*****************************************************************************
 * 
 * GetVersion()
 * 
 *****************************************************************************/

static ULONG GetVersion(APTR key, LONG * opts)
{
	APTR value = NULL;
	
	// GET VERSION using the new method (Emu68 >= 1.1)
	if (value = GetProperty(key, "version")) {
		version = (VERS3 *)value;
	} else {
		// GET VERSION using the old method (Emu68 < 1.1)
		if (value = GetProperty(key, "idstring")) {
			version = ParseIdString((STRPTR)value);
		} else {
			Printf("Can't open property (idstring)!\n");
			return (RETURN_ERROR);
		}
	}
	
	// PRINT VERSION
	if (!opts[OPT_SHORT] && !opts[OPT_FULL] && 
		!opts[OPT_GITHASH] && !opts[OPT_VARIANT]) {
			Printf("Emu68 %ld.%ld.%ld\n", 
				version->ver_major, 
				version->ver_minor, 
				version->ver_patch);
	}
	
	// PRINT SHORT
	if (opts[OPT_SHORT]) {
		Printf("%ld.%ld.%ld\n", 
			version->ver_major,
			version->ver_minor, 
			version->ver_patch);
	}
	
	// PRINT FULL
	if (opts[OPT_FULL]) {
		if (value = GetProperty(key, "idstring")) {
			Printf("%s\n", (STRPTR)((ULONG)value + 6));
		} else {
			Printf("Can't open property (idstring)!\n");
			return (RETURN_ERROR);
		}
	}
	
	// PRINT GITHASH
	if (opts[OPT_GITHASH]) {
		if (value = GetProperty(key, "git-hash")) {
			Printf("%s\n", value);
		} else {
			Printf("Can't open property (git-hash)!\n");
			return (RETURN_ERROR);
		}
	}
	
	// PRINT VARIANT
	if (opts[OPT_VARIANT]) {
		if (value = GetProperty(key, "variant")) {
			Printf("%s\n", value);
		} else {
			Printf("Can't open property (variant)!\n");
			return (RETURN_ERROR);
		}
	}
	
	// CHECK VERSION
	if (opts[OPT_VERSION]) {
		if ((*(LONG *)opts[OPT_VERSION]) > version->ver_major) {
			return (RETURN_WARN);
		}
	}
	
	// CHECK REVISION
	if (opts[OPT_REVISION]) {
		if ((*(LONG *)opts[OPT_REVISION]) > version->ver_minor) {
			return (RETURN_WARN);
		}
	}
	
	// CHECK HOTFIX
	if (opts[OPT_HOTFIX]) {
		if ((*(LONG *)opts[OPT_HOTFIX]) > version->ver_patch) {
			return (RETURN_WARN);
		}
	}
	
	// NO WARN
	return (RETURN_OK);
}

/*****************************************************************************
 * 
 * Entry point
 * 
 *****************************************************************************/

ULONG main(ULONG argc, STRPTR * argv)
{
	APTR key;
	ULONG rc;
	LONG * opts = NULL;
	struct RDArgs * rdargs = NULL;
	
	if (!(opts = AllocVec(OPT_COUNT * sizeof(LONG), MEMF_PUBLIC|MEMF_CLEAR))) {
		PutStr("Can't allocate memory.\n");
		rc = RETURN_FAIL;
		goto cleanExit;
	}
	
	if (!(rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))) {
		PutStr("Bad argument, use HELP for more information!\n");
		rc = RETURN_FAIL;
		goto cleanExit;
	}
	
	if (opts[OPT_HELP]) {
		GetHelp();
		rc = RETURN_OK;
		goto cleanExit;
	}
	
	if (!(DeviceTreeBase = OpenResource(DEVICETREE_NAME))) {
		PutStr("Can't open " DEVICETREE_NAME "!\n");
		rc = RETURN_ERROR;
		goto cleanExit;
	}
	
	if (!(key = DT_OpenKey("/emu68"))) {
		PutStr("Can't open emu68 key!\n");
		rc = RETURN_ERROR;
		goto cleanExit;
	}
	
	rc = GetVersion(key, opts);
	
cleanExit:
	if (rdargs) FreeArgs(rdargs);
	if (opts) FreeVec(opts);
	return (rc);
}

/*****************************************************************************
 * 
 * END OF FILE
 * 
 *****************************************************************************/
