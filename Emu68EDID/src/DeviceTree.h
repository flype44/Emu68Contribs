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

LONG GetField(STRPTR, STRPTR, ULONG);
APTR GetProp(STRPTR, STRPTR);
ULONG GetPropInt(STRPTR, STRPTR);
STRPTR GetPropStr(UBYTE *, STRPTR, STRPTR);
APTR GetPropValue(STRPTR, STRPTR, ULONG *);
CONST_APTR GetPropValueRecursive(APTR, CONST_STRPTR);

#endif /* _DEVICETREE_HELPER_H */
