/**********************************************************
 ** 
 ** INCLUDES
 ** 
 **********************************************************/

#include <exec/types.h>

#include "AsmFuncs.h"
#include "VC4.h"

/**********************************************************
 ** 
 ** VC4_ResetRPi()
 ** 
 **********************************************************/

void VC4_FullReset(void)
{
	// trigger a restart by instructing the GPU to boot from partition 0.
	
	ULONG rsts = asm_le32(*PM_RSTS) & ~0xfffffaaa;
	
	*PM_RSTS = asm_le32(PM_WDOG_MAGIC | rsts);
	*PM_WDOG = asm_le32(PM_WDOG_MAGIC | 10);
	*PM_RSTC = asm_le32(PM_WDOG_MAGIC | PM_RSTC_FULLRST);
	
	// infinite loop => WatchDog will catch it and restart the system.
	
	while (1);
}

/**********************************************************
 ** 
 ** END OF FILE
 ** 
 **********************************************************/
