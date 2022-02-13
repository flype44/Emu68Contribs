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
 ** DEFINES
 ** 
 **********************************************************/

UBYTE sModules[128];

/**********************************************************
 ** 
 ** MyIdString()
 ** 
 **********************************************************/

STRPTR Modules_IdString(STRPTR idString)
{
	UBYTE *src = idString;
	UBYTE *dst = sModules;
	
	do
	{
		if (*src == 0x0A)
			continue;
		
		if (*src == 0x0D)
			continue;
		
		*dst++ = *src;
		
	} while (*src++);
	
	*dst = 0;
	
	return (sModules[0] == '$' ? sModules + 6 : sModules);
}

/**********************************************************
 ** 
 ** GetModules_DeviceTree()
 ** 
 **********************************************************/

void Modules_GetDeviceTree(Module *m)
{
	if (GetPropStr(sModules, "/emu68", "idstring"))
	{
		m->addr     = (APTR)-1L; // fake
		m->ver      = atoi(strstr(sModules + 6, " ") + 1);
		m->rev      = atoi(strstr(sModules + 6, ".") + 1);
		m->idString = Modules_IdString(sModules);
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
		
		m->addr     = lib;
		m->ver      = lib->lib_Version;
		m->rev      = lib->lib_Revision;
		m->idString = Modules_IdString(lib->lib_IdString);
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
		strcpy(sModules, "Public message port found");
		
		m->addr     = res;
		m->ver      = 0L;
		m->rev      = 0L;
		m->idString = sModules;
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
		m->addr = res;
		m->ver  = res->rt_Version;
		m->rev  = 0;
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
	
	for (i = 0; i < count; i++)
	{
		Module m = items[i];
		
		switch (m.type)
		{
		case MODULE_DEVICE:
			Modules_GetLibrary(&m, &SysBase->DeviceList);
			break;
		
		case MODULE_LIBRARY:
			Modules_GetLibrary(&m, &SysBase->LibList);
			break;
		
		case MODULE_RESOURCE:
			Modules_GetLibrary(&m, &SysBase->ResourceList);
			break;
		
		case MODULE_RESIDENT:
			Modules_GetResident(&m);
			break;
		
		case MODULE_MSGPORT:
			Modules_GetPort(&m);
			break;
		
		case MODULE_DEVICETREE:
			Modules_GetDeviceTree(&m);
			break;
		}
		
		printf("| $%08lx | %-20s | %3ld | %4ld | %s\n", 
			m.addr, m.name, m.ver, m.rev, m.idString);
	}
	
	for (i = 0; i < count; i++)
	{
		Module m = items[i];
		
		printf("| $%08lx | %-20s | %3ld | %4ld | %s\n", 
			items[i].addr, items[i].name, m.ver, m.rev, m.idString);
	}
	
	return (1);
}

/**********************************************************
 ** 
 ** END OF FILE
 ** 
 **********************************************************/
