/**********************************************************
 ** 
 ** File:     DeviceTree.c
 ** Short:    Print contents of the devicetree.resource.
 ** Author:   Philippe CARPENTIER
 ** Requires: AmigaOS, Emu68, devicetree.resource
 ** 
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/devicetree.h>

#include "DeviceTree.h"

/**********************************************************
 * DEFINES
 **********************************************************/

#define TEMPLATE "KEY,PROPERTY,VALUE,LEVEL/N,HELP/S"

typedef enum {
	OPT_KEY,
	OPT_PROPERTY,
	OPT_VALUE,
	OPT_LEVEL,
	OPT_HELP,
	OPT_COUNT
} OPT_ARGS;

/**********************************************************
 ** 
 ** Globals
 ** 
 **********************************************************/

UBYTE keyName[4096];

STRPTR AppName = APP_NAME;
STRPTR AppVers = APP_VSTRING;

extern struct ExecBase * SysBase;

static STRPTR verstring = APP_VSTRING;

/**********************************************************
 ** 
 ** Help()
 ** 
 **********************************************************/

ULONG Help(void)
{
	printf("\n"
"   NAME\n	DEVICETREE - Print contents of the devicetree.resource.\n\n"
"   VERSION\n	%s\n"
"   FORMAT\n	DEVICETREE [KEY] [PROPERTY] [VALUE] [LEVEL] [HELP]\n\n"
"   TEMPLATE\n	" TEMPLATE "\n\n"
"   PATH\n	C:DEVICETREE\n\n"
"   FUNCTION\n"
"   	DEVICETREE allows you to read the content of the devicetree.resource.\n"
"   	The devicetree.resource is a library that is available only in a Amiga\n"
"   	that is equipped with a PiStorm/Emu68 accelerator, and it contains a\n"
"   	tree of keypair values which comes from the Raspberry firmware itself.\n"
"   	This tool is eventually useful for Emu68 developers or advanced users.\n\n"
"   RESULT\n"
"   	Returns 20 (FAIL) : Wrong DOS arguments\n"
"   	Returns 20 (FAIL) : Can not find \"devicetree.resource\"\n"
"   	Returns 05 (WARN) : Can not find a key or property or value\n"
"   	Returns 00 (OK)   : Found a key or property or value\n\n"
"   OPTIONS\n\n"
"   	KEY:      Key to find (optional).\n\n"
"   	PROPERTY: Property to find (optional).\n\n"
"   	VALUE:    Value to find (strings only) (optional).\n\n"
"   	LEVEL:    Level of verbosity (optional)\n"
"   		  LEVEL=0: Print out the found keys (Default).\n"
"   		  LEVEL=1: Print out the found keys + values (short).\n"
"   		  LEVEL=2: Print out the found keys + values (dump).\n\n"
"   	HELP:     This Help.\n\n"
"   EXAMPLES\n\n"
"   	> DEVICETREE LEVEL=0 >dt0.txt\n"
"   	> DEVICETREE LEVEL=1 >dt1.txt\n\n"
"   	> DEVICETREE emu68\n"
"   	[\"/emu68\"][\"idstring\"] : chars[43]\n"
"   	[\"/emu68\"][\"git-hash\"] : chars[40]\n"
"   	[\"/emu68\"][\"vc4-mem\"] : long[8]\n\n"
"   	> DEVICETREE emu68 LEVEL=1\n"
"   	[\"/emu68\"][\"idstring\"] : chars[43] = \"$VER: Emu68 0.11.4 (31.12.2022) git:33caf66U\"\n"
"   	[\"/emu68\"][\"git-hash\"] : chars[40] = \"33caf66414033a330e161d26979b0666c9c03fd1\"\n"
"   	[\"/emu68\"][\"vc4-mem\"] : long[8] = 1D 00 00 00 01 00 00 00\n\n"
"   	> DEVICETREE emu68 idstring LEVEL=1\n"
"   	[\"/emu68\"][\"idstring\"] : chars[43] = \"$VER: Emu68 0.11.4 (31.12.2022) git:33caf66U\"\n\n"
"   	> DEVICETREE \"\" \"\" model LEVEL=1\n"
"   	[\"/\"][\"compatible\"] : chars[40] = \"raspberrypi,3-model-b-plus\"\n\n"
"   	> DEVICETREE \"\" \"\" \"$VER\" LEVEL=2\n"
"   	[\"/emu68\"][\"idstring\"] : chars[43]\n"
"   	0000 | 24 56 45 52  3A 20 45 6D  75 36 38 20  30 2E 31 31 | $VER: Emu68 0.11\n"
"   	0010 | 2E 34 20 28  33 31 2E 31  32 2E 32 30  32 32 29 20 | .4 (31.12.2022) \n"
"   	0020 | 67 69 74 3A  33 33 63 61  66 36 36 --  -- -- -- -- | git:33caf66-----\n\n"
"   	> DEVICETREE \"/\" \"model\" \"Raspberry Pi 3\" >NIL:\n"
"   	> IF NOT WARN\n"
"   	>   ECHO \"Your RPi model is a RPi3\"\n"
"   	> ENDIF\n\n"
"   SEE ALSO\n"
"   	EMUCONTROL, EMU68INFO\n\n"
, verstring + 6);
	
	return (RETURN_OK);
}

/**********************************************************
 ** 
 ** IsASCII()
 ** 
 **********************************************************/

BOOL IsASCII(UBYTE* buffer, ULONG num_bytes)
{
	ULONG i;
	
	// Check if size <= 8 and not odd size
	
	if ((num_bytes <= 8) && (num_bytes % 2 == 0))
	{
		return (FALSE);
	}
	
	// Check if the buffer contains only chars
	
	for (i = 0; i < num_bytes - 1; i++)
	{
		if (buffer[i] && buffer[i] <= 31 || buffer[i] >= 127)
		{
			return (FALSE);
		}
	}
	
	return (TRUE);
}

/**********************************************************
 ** 
 ** DumpBYTES()
 ** 
 **********************************************************/

void DumpBYTES(UBYTE* buffer, ULONG num_bytes)
{
	ULONG i;
	
	for (i = 0; i < num_bytes; i++)
	{
		printf("%02X ", buffer[i]);
	}
}

/**********************************************************
 ** 
 ** DumpROWS()
 ** 
 **********************************************************/

void DumpROWS(UBYTE* buffer, ULONG num_bytes, ULONG length)
{
	static UBYTE row[128];
	ULONG i, j = 0, offset = 0;
	
	while (j < num_bytes)
	{
		// Print OFFSET
		
		printf(COLOR5 "%04X " COLOR7 "|" COLOR4, offset);
		
		// Print RAW
		
		for (i = 0; i < length; i++)
		{
			if (i > 0 && i % 4 == 0)
			{
				printf(" ");
			}
			
			if (j < num_bytes)
			{
				printf(COLOR4 " %02X", buffer[offset + i]);
			}
			else
			{
				printf(COLOR6 " --");
			}
			
			j++;
		}
		
		// Print ASCII		
		
		row[0] = 0;
		
		printf(COLOR7 " | " COLOR4);
		
		for (i = 0; i < length; i++)
		{
			if (offset >= num_bytes)
			{
				row[i] = '-';
			}
			else
			{
				if ((buffer[offset] > 31) && (buffer[offset] < 127))
				{
					row[i] = buffer[offset];
				}
				else
				{
					row[i] = '-';
				}
			}
			
			offset++;
		}
		
		row[i] = 0;
		printf(COLOR6 "%s", row);
		printf(COLOR7 "\n");
	}
}

/**********************************************************
 ** 
 ** GetProperty()
 ** 
 **********************************************************/

ULONG GetProperty(of_property_t * property, STRPTR name1, ULONG level)
{
	ULONG result = 0;
	STRPTR name = (STRPTR)property->op_name;
	ULONG length = (ULONG)property->op_length;
	UBYTE * bytes = (UBYTE *)property->op_value;
	
	if (strlen(name1) == 0 || strstr(bytes, name1))
	{
		printf("[\"%s\"][\"%s\"] : %s[%lu]", 
			keyName, name,
			IsASCII(bytes, length) ? "chars" : 
			(length > 8 ? "bytes" : "long"), 
			length);
		
		if (level == 1)
		{
			printf(" = ");
			
			if (IsASCII(bytes, length))
			{
				printf("\"%s\"", bytes);
			}
			else
			{
				DumpBYTES(bytes, length);
			}
		}
		
		if (level == 2)
		{
			printf("\n");
			DumpROWS(bytes, length, 16);
		}
		
		printf("\n");
		result = 1;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetProperties()
 ** 
 **********************************************************/

ULONG GetProperties(of_property_t * property, STRPTR name1, STRPTR name2, ULONG level)
{
	ULONG result = 0;
	
	while (property)
	{
		if (strlen(name1) == 0 || strstr(property->op_name, name1))
		{
			result += GetProperty(property, name2, level);
		}
		
		property = property->op_next;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** GetNode()
 ** 
 **********************************************************/

ULONG GetNodes(of_node_t *node, STRPTR name1, STRPTR name2, STRPTR name3, ULONG level, ULONG n)
{
	ULONG result = 0;
	
	while (node)
	{
		// Get the name
		
		if (keyName[n - 1] != '/')
		{
			strcat(keyName, "/");
		}
		
		strcat(keyName, node->on_name);
		
		// Get the properties
		
		if (strlen(name1) == 0 || strstr(keyName, name1))
		{
			result += GetProperties(
				node->on_properties, 
				name2, name3, level);
		}
		
		// Get the children (recursive)
		
		if (node->on_children)
		{
			result += GetNodes(
				node->on_children, 
				name1, name2, name3, 
				level, strlen(keyName));
		}
		
		// Next
		
		keyName[n] = 0;
		node = node->on_next;
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** Entry Point
 ** 
 **********************************************************/

ULONG main(ULONG argc, char *argv[])
{
	ULONG result = RETURN_FAIL;
	
	// Read DOS arguments
	
	struct RDArgs *rdargs;
	LONG opts[OPT_COUNT];
	memset((char *)opts, 0, sizeof(opts));
	
	if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
	{
		if (opts[OPT_HELP])
		{
			result = Help();
		}
		else
		{
			// Open the Emu68/DeviceTree library
			
			struct DeviceTreeBase * DTBase;
			
			if (DTBase = (struct DeviceTreeBase *)OpenResource("devicetree.resource"))
			{
				// Read the Emu68/DeviceTree content
				
				result = GetNodes(
					DTBase->dt_Root, 
					(STRPTR)opts[OPT_KEY], 
					(STRPTR)opts[OPT_PROPERTY], 
					(STRPTR)opts[OPT_VALUE], 
					*(LONG *)opts[OPT_LEVEL], 
					0) ? RETURN_OK : RETURN_WARN;
				
				FreeArgs(rdargs);
			}
		}
	}
	else
	{
		// Wrong aruments
		
		printf("wrong arguments.\nusage: DEVICETREE HELP\n");
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** End of file
 ** 
 **********************************************************/
