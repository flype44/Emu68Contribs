/**********************************************************
 ** 
 ** AmigaOS Modules Helper functions
 ** 
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/errors.h>

#include <proto/exec.h>

#include "Modules.h"
#include "DeviceTree.h"

/**********************************************************
 ** 
 ** Modules_IdString()
 ** 
 **********************************************************/

void Modules_IdString(STRPTR srcIdString, STRPTR dstIdString)
{
	UBYTE *src = srcIdString;
	UBYTE *dst = dstIdString;
	
	// ignoring '$VER:' if present
	
	if (*src == '$')
		src += 6;
	
	do
	{
		// ignoring return carriages
		
		if (*src == 0x0A)
			continue;
		
		if (*src == 0x0D)
			continue;
		
		*dst++ = *src;
		
	} while (*src++);
	
	// null-terminating char
	
	*dst = 0;
}

/**********************************************************
 ** 
 ** Modules_GetDeviceTree()
 ** 
 **********************************************************/

void Modules_GetDeviceTree(Module *m)
{
	UBYTE s[256];
	
	if (GetPropStr(s, "/emu68", "idstring"))
	{
		Modules_IdString(s, m->idString);
		
		m->addr     = (APTR)0L;
		m->ver      = atoi(strstr(s + 6, " ") + 1);
		m->rev      = atoi(strstr(s + 6, ".") + 1);
		m->loaded   = TRUE;
	}
}

/**********************************************************
 ** 
 ** Modules_GetLibrary()
 ** 
 **********************************************************/

void Modules_GetLibrary(Module *m, struct List *list)
{
	struct Node * node;
	
	if (node = FindName(list, m->name))
	{
		struct Library *lib = (struct Library *)node;
		
		Modules_IdString(lib->lib_IdString, m->idString);
		
		m->addr     = lib;
		m->ver      = lib->lib_Version;
		m->rev      = lib->lib_Revision;
		m->loaded   = TRUE;
	}
}

/**********************************************************
 ** 
 ** GetModules_Port()
 ** 
 **********************************************************/

void Modules_GetPort(Module *m)
{
	APTR res;
	
	if (res = FindPort(m->name))
	{
		strcpy(m->idString, "Public message port found");
		
		m->addr     = res;
		m->ver      = 0L;
		m->rev      = 0L;
		m->loaded   = TRUE;
	}
}

/**********************************************************
 ** 
 ** GetModules_Resident()
 ** 
 **********************************************************/

void Modules_GetResident(Module *m)
{
	struct Resident * res;
	
	if (res = FindResident(m->name))
	{
		m->addr     = res;
		m->ver      = res->rt_Version;
		m->rev      = 0;
		m->loaded   = TRUE;
	}
}

/**********************************************************
 ** 
 ** Modules_Load()
 ** 
 **********************************************************/

ULONG Modules_Load(Module items[], ULONG count)
{
	ULONG i;
	
	if (SysBase == NULL || items == NULL)
	{
		// failure
		return (0);
	}
	
	for (i = 0; i < count; i++)
	{
		switch (items[i].type)
		{
		case MODULE_DEVICE:
			Modules_GetLibrary(&items[i], &SysBase->DeviceList);
			break;
		
		case MODULE_LIBRARY:
			Modules_GetLibrary(&items[i], &SysBase->LibList);
			break;
		
		case MODULE_RESOURCE:
			Modules_GetLibrary(&items[i], &SysBase->ResourceList);
			break;
		
		case MODULE_RESIDENT:
			Modules_GetResident(&items[i]);
			break;
		
		case MODULE_MSGPORT:
			Modules_GetPort(&items[i]);
			break;
		
		case MODULE_DEVICETREE:
			Modules_GetDeviceTree(&items[i]);
			break;
		}
	}
	
	// success
	return (1);
}

/**********************************************************
 ** 
 ** END OF FILE
 ** 
 **********************************************************/
