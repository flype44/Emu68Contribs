/********************************************************************
 *
 * Program:  Emu68EDID.c
 * Purpose:  EDID command for Emu68
 * Authors:  Philippe CARPENTIER
 * Target:   AmigaOS 3.x
 * Compiler: SAS/C Amiga Compiler 6.59
 *
 * Usage:
 * Emu68EDID DUMP [TO=<file>]
 * Emu68EDID PARSE [FROM=<file>] [FULL]
 *
 ********************************************************************/

#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "MailBox.h"
#include "EDID.h"

/******************************************************************************
 *
 * DEFINES
 *
 ******************************************************************************/

#define EDID_BLOCK_SIZE (128)
#define EDID_BLOCK_COUNT (256)

#define TEMPLATE "DUMP/S,TO/K,PARSE/S,FROM/K,FULL/S"

typedef enum {
	OPT_DUMP,
	OPT_TO,
	OPT_PARSE,
	OPT_FROM,
	OPT_FULL,
	OPT_COUNT
} OPT_ARGS;

/******************************************************************************
 *
 * GLOBALS
 *
 ******************************************************************************/

UBYTE edid_data[EDID_BLOCK_SIZE * EDID_BLOCK_COUNT];
#ifdef VERSION_STRING
UBYTE VERSTRING[] = "\0" VERSION_STRING;
#else
UBYTE VERSTRING[] = "\0$VER: Emu68EDID 1.0 (3.11.2025)";
#endif
/******************************************************************************
 *
 * EXTERNS
 *
 ******************************************************************************/

extern struct ExecBase * SysBase;
extern struct DosLibrary * DOSBase;

/******************************************************************************
 *
 * PROTOTYPES
 *
 ******************************************************************************/

static ULONG EDID_GetData(VOID);
static UBYTE EDID_Checksum(UBYTE * buffer, ULONG size);
static LONG  EDID_DumpToFile(STRPTR filename, UBYTE * buffer, ULONG size);
static VOID  EDID_DumpToStdout(UBYTE * buffer, ULONG size);
static LONG  EDID_Dump(STRPTR filename);
static LONG  EDID_ParseFromFile(STRPTR filename, BOOL full);
static LONG  EDID_ParseFromHardware(BOOL full);
static LONG  EDID_Parse(STRPTR filename, BOOL full);

/******************************************************************************
 *
 * EDID_Checksum()
 *
 ******************************************************************************/

static UBYTE EDID_Checksum(UBYTE * buffer, ULONG size)
{
	ULONG i;
	UBYTE sum = 0;

	for (i = 0; i < size; i++)
	{
		sum += buffer[i];
	}

	return sum;
}

/******************************************************************************
 *
 * EDID_GetData()
 *
 ******************************************************************************/

static ULONG EDID_GetData(VOID)
{
	UBYTE * buffer = (UBYTE *)edid_data;

	if (mbox_init())
	{
		if (mbox_get_edid_block(0, buffer) == TRUE)
		{
			if (EDID_Checksum(buffer, EDID_BLOCK_SIZE) == 0)
			{
				UBYTE extensions = buffer[126];

				buffer += EDID_BLOCK_SIZE;

				if (extensions > 0 && extensions < EDID_BLOCK_COUNT)
				{
					ULONG block;

					for (block = 0; block < extensions; block++)
					{
						if (mbox_get_edid_block(block + 1, buffer) == TRUE)
						{
							buffer += EDID_BLOCK_SIZE;
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		mbox_free();
	}

	return (ULONG)(buffer - edid_data);
}

/******************************************************************************
 *
 * EDID_DumpToFile()
 *
 ******************************************************************************/

static LONG EDID_DumpToFile(STRPTR filename, UBYTE * buffer, ULONG size)
{
	static BPTR file;
	static LONG errorCode = 0;

	if (file = Open(filename, MODE_NEWFILE))
	{
		if (Write(file, buffer, size) != size)
		{
			errorCode = IoErr();
		}

		Close(file);
	}

	return (errorCode);
}

/******************************************************************************
 *
 * EDID_DumpToStdout()
 *
 ******************************************************************************/

static VOID EDID_DumpToStdout(UBYTE * buffer, ULONG size)
{
	static ULONG i;

	for (i = 0; i < size; i++)
	{
		Printf("%02lx ", buffer[i]);

		if ((i + 1) % 16 == 0)
		{
			PutStr("\n");
		}
	}
}

/******************************************************************************
 *
 * EDID_Dump()
 *
 ******************************************************************************/

static LONG EDID_Dump(STRPTR filename)
{
	ULONG edid_size = 0;

	if (edid_size = EDID_GetData())
	{
		if (filename != NULL)
		{
			return EDID_DumpToFile(filename, edid_data, edid_size);
		}

		EDID_DumpToStdout(edid_data, edid_size);
	}
	else
	{
		SetIoErr(ERROR_NO_MORE_ENTRIES);
	}

	return (IoErr());
}

/******************************************************************************
 *
 * EDID_ParseFromFile()
 *
 ******************************************************************************/

static LONG EDID_ParseFromFile(STRPTR filename, BOOL full)
{
	BPTR file;
	APTR buffer;
	_MonitorInfo * info;
	struct FileInfoBlock __aligned fib;

	if (file = Open(filename, MODE_OLDFILE))
	{
		if (ExamineFH(file, &fib))
		{
			if (buffer = AllocVec(fib.fib_Size + 1, MEMF_PUBLIC | MEMF_CLEAR))
			{
				if (Read(file, buffer, fib.fib_Size) == fib.fib_Size)
				{
					if (info = decode_edid(buffer))
					{
						if (full) dump_monitor_info(info);
						else dump_monitor_info_short(info);
						free_monitor_info(info);
					}
				}

				FreeVec(buffer);
			}
			else
			{
				SetIoErr(ERROR_NO_FREE_STORE);
			}
		}

		Close(file);
	}

	return (IoErr());
}

/******************************************************************************
 *
 * EDID_ParseFromHardware()
 *
 ******************************************************************************/

static LONG EDID_ParseFromHardware(BOOL full)
{
	if (EDID_GetData())
	{
		_MonitorInfo * info;

		if (info = decode_edid(edid_data))
		{
			if (full) dump_monitor_info(info);
			else dump_monitor_info_short(info);
			free_monitor_info(info);
		}
	}
	else
	{
		SetIoErr(ERROR_NO_MORE_ENTRIES);
	}

	return (IoErr());
}

/******************************************************************************
 *
 * EDID_Parse()
 *
 ******************************************************************************/

static LONG EDID_Parse(STRPTR filename, BOOL full)
{
	if (filename != NULL)
	{
		return EDID_ParseFromFile(filename, full);
	}

	return EDID_ParseFromHardware(full);
}

/******************************************************************************
 *
 * main()
 *
 ******************************************************************************/

ULONG main(ULONG argc, STRPTR * argv)
{
	ULONG rc = RETURN_FAIL;
	LONG errorCode = 0;
	LONG opts[OPT_COUNT];
	struct RDArgs * rdargs;

	opts[OPT_DUMP ] = 0L;
	opts[OPT_TO   ] = 0L;
	opts[OPT_PARSE] = 0L;
	opts[OPT_FROM ] = 0L;
	opts[OPT_FULL ] = 0L;

	if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
	{
		rc = RETURN_ERROR;

		if (opts[OPT_DUMP])
		{
			errorCode = EDID_Dump((STRPTR)opts[OPT_TO]);
			rc = (errorCode == 0) ? RETURN_OK : RETURN_WARN;
		}
		else if (opts[OPT_PARSE])
		{
			errorCode = EDID_Parse((STRPTR)opts[OPT_FROM],
				opts[OPT_FULL] ? TRUE : FALSE);
			rc = (errorCode == 0) ? RETURN_OK : RETURN_WARN;
		}
		else
		{
			errorCode = ERROR_REQUIRED_ARG_MISSING;
		}

		FreeArgs(rdargs);
	}
	else
	{
		errorCode = IoErr();
	}

	if (errorCode)
	{
		PrintFault(errorCode, NULL);
	}

	return (rc);
}

/******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
