#ifndef __MODULES_H
#define __MODULES_H

/**********************************************************
 ** 
 ** INCLUDES
 ** 
 **********************************************************/

#include <exec/types.h>

/**********************************************************
 ** 
 ** DEFINES
 ** 
 **********************************************************/

typedef enum {
	MODULE_DEVICE,    // 0
	MODULE_LIBRARY,   // 1
	MODULE_RESOURCE,  // 2
	MODULE_RESIDENT,  // 3
	MODULE_MSGPORT,   // 4
	MODULE_DEVICETREE // 5
} MODULE_TYPE;

typedef struct {
	ULONG  type;
	STRPTR name;
	APTR   addr;
	ULONG  ver;
	ULONG  rev;
	STRPTR idString;
} Module;

/**********************************************************
 ** 
 ** PROTOTYPES
 ** 
 **********************************************************/

ULONG Modules_Load(Module modules[], ULONG count);

#endif /* __MODULES_H */
