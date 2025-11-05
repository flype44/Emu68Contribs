/******************************************************************************
 * 
 * MAILBOX
 * 
 *****************************************************************************/

#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/devicetree.h>

#include "AsmFuncs.h"
#include "DeviceTree.h"
#include "MailBox.h"

#define LE32 asm_le32

/******************************************************************************
 * 
 * GLOBALS
 * 
 *****************************************************************************/

APTR MailBoxRequestBase = NULL;
APTR MailBoxRequest     = NULL;
APTR MailBoxBase        = NULL;
APTR DeviceTreeBase     = NULL;

/******************************************************************************
 * 
 * mbox_init()
 * 
 *****************************************************************************/

BOOL mbox_init(void)
{
	BOOL result = FALSE;
	
	if (MailBoxRequestBase = AllocVec(512 * 4, MEMF_FAST))
	{
		MailBoxRequest = (ULONG *)(((ULONG)MailBoxRequestBase + 127) & ~127);
		
		if (DeviceTreeBase = (struct DeviceTreeBase *)OpenResource("devicetree.resource"))
		{
			APTR key;
			
			if (key = DT_OpenKey("/aliases"))
			{
				CONST_STRPTR mbox_alias = DT_GetPropValue(DT_FindProperty(key, "mailbox"));
				
				DT_CloseKey(key);
				
				if (mbox_alias != NULL)
				{
					if (key = DT_OpenKey(mbox_alias))
					{
						ULONG size_cells = 1;
						ULONG address_cells = 1;
						
						CONST ULONG * siz = GetPropValueRecursive(key, "#size_cells");
						CONST ULONG * adr = GetPropValueRecursive(key, "#address-cells");
						CONST ULONG * reg = DT_GetPropValue(DT_FindProperty(key, "reg"));
						
						if (siz != NULL) size_cells = *siz;
						if (adr != NULL) address_cells = *adr;
						
						MailBoxBase = (APTR)reg[address_cells - 1];
						
						DT_CloseKey(key);
					}
				}
			}
			
			if (key = DT_OpenKey("/soc"))
			{
				ULONG phys_vc;
				ULONG phys_cpu;
				
				ULONG size_cells = 1;
				ULONG address_cells = 1;
				ULONG cpu_address_cells = 1;
				
				CONST ULONG * siz = GetPropValueRecursive(key, "#size_cells");
				CONST ULONG * adr = GetPropValueRecursive(key, "#address-cells");
				CONST ULONG * reg = DT_GetPropValue(DT_FindProperty(key, "ranges"));
				CONST ULONG * cpu = DT_GetPropValue(DT_FindProperty(DT_OpenKey("/"), "#address-cells"));
				
				if (siz != NULL) size_cells = *siz;
				if (adr != NULL) address_cells = *adr;
				if (cpu != NULL) cpu_address_cells = *cpu;
				
				phys_vc  = reg[address_cells - 1];
				phys_cpu = reg[address_cells + cpu_address_cells - 1];
				
				MailBoxBase = (APTR)((ULONG)MailBoxBase - phys_vc + phys_cpu);
				
				DT_CloseKey(key);
				
				result = TRUE;
			}
		}
	}
	
	return (result);
}

/******************************************************************************
 * 
 * mbox_free()
 * 
 *****************************************************************************/

VOID mbox_free(VOID)
{
	if (MailBoxRequestBase != NULL)
	{
		FreeVec(MailBoxRequestBase);
		MailBoxRequestBase = NULL;
		MailBoxRequest = NULL;
		MailBoxBase = NULL;
	}
}

/******************************************************************************
 * 
 * mbox_recv()
 * 
 *****************************************************************************/

static ULONG mbox_recv(ULONG channel)
{
	ULONG status, response;
	
	volatile ULONG * mbox_read   = (ULONG *)((ULONG)MailBoxBase + MBOX_READ);
	volatile ULONG * mbox_status = (ULONG *)((ULONG)MailBoxBase + MBOX_STATUS);
	
	do
	{
		do
		{
			status = LE32(*mbox_status);
			asm_nop();
		}
		while (status & MBOX_STATUS_EMPTY);
		
		asm_nop();
		response = LE32(*mbox_read);
		asm_nop();
	}
	while ((response & MBOX_CHANMASK) != channel);
	
	return (response & ~MBOX_CHANMASK);
}

/******************************************************************************
 * 
 * mbox_send()
 * 
 *****************************************************************************/

static VOID mbox_send(ULONG channel, ULONG data)
{
	ULONG status;
	
	volatile ULONG * mbox_write  = (ULONG *)((ULONG)MailBoxBase + MBOX_WRITE);
	volatile ULONG * mbox_status = (ULONG *)((ULONG)MailBoxBase + MBOX_STATUS);
	
	data &= ~MBOX_CHANMASK;
	data |= channel & MBOX_CHANMASK;
	
	do
	{
		status = LE32(*mbox_status);
		asm_nop();
	}
	while (status & MBOX_STATUS_FULL);
	asm_nop();
	
	*mbox_write = LE32(data);
}

/******************************************************************************
 * 
 * mbox_get_edid_block()
 * 
 *****************************************************************************/

BOOL mbox_get_edid_block(ULONG block_number, UBYTE * edid_out)
{
	if (edid_out != NULL)
	{
		ULONG * msg = MailBoxRequest;
		ULONG size = (8 * 4 + 128);
		ULONG i;
		
		msg[0] = LE32(size);                  // request size
		msg[1] = 0;                           // request code
		msg[2] = LE32(TAG_GET_EDID_BLOCK);    // tag identifier
		msg[3] = LE32(2 * 4 + 128);           // response size
		msg[4] = 0;                           // response code
		msg[5] = LE32(block_number);          // response[0]: block number
		msg[6] = 0;                           // response[1]: status
		msg[7] = LE32(0xDEADBEEF);            // response[2]: edid[0]
		for (i = 0; i < 32; i++)              // response[n]: edid[n]
			msg[7 + i] = 0;                   // response[n]: edid[n] 
		msg[7 + 32] = 0;                      // tag end
		
		CachePreDMA(msg, &size, 0);
		mbox_send(PROPERTY_CHANNEL, (ULONG)msg);
		mbox_recv(PROPERTY_CHANNEL);
		CachePostDMA(msg, &size, 0);
		
		if (LE32(msg[1]) == 0x80000000) // request success
		{
			if (LE32(msg[6]) == 0x00000000) // status ok
			{
				if (LE32(msg[7]) != 0xDEADBEEF) // overwritten
				{
					CopyMem((UBYTE *)&msg[7], edid_out, 128);
					return (TRUE);
				}
			}
		}
	}
	
	return (FALSE);
}

/******************************************************************************
 * 
 * END OF FILE
 * 
 *****************************************************************************/
