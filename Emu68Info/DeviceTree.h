#ifndef _DEVICETREE_HELPER_H
#define _DEVICETREE_HELPER_H

/**********************************************************
 ** 
 ** Includes
 ** 
 **********************************************************/

#include <exec/exec.h>
#include <exec/types.h>

/**********************************************************
 ** 
 ** Prototypes
 ** 
 **********************************************************/

LONG GetField(STRPTR str, STRPTR key, ULONG base);
APTR GetProp(STRPTR key, STRPTR property);
ULONG GetPropInt(STRPTR key, STRPTR property);
STRPTR GetPropStr(UBYTE * buffer, STRPTR key, STRPTR property);
CONST_APTR GetPropValueRecursive(APTR key, CONST_STRPTR property);

#endif /* _DEVICETREE_HELPER_H */
