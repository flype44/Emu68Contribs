/**********************************************************
 ** 
 ** MailBox Helper functions
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
#include <proto/expansion.h>
#include <proto/devicetree.h>

#include <libraries/configregs.h>
#include <libraries/configvars.h>

#include "AsmFuncs.h"
#include "DeviceTree.h"
#include "MailBox.h"

/**********************************************************
 ** 
 ** DEFINES
 ** 
 **********************************************************/

/* status register flags */

#define MBOX_TX_FULL   (1UL << 31)
#define MBOX_RX_EMPTY  (1UL << 30)
#define MBOX_CHANMASK  (0xF)
#define MBOX_SIZE(x)   ((2+(x)+1)*4) 
#define MBOX_REQUEST() ((ULONG*)(((ULONG)__req + 31) & ~31))

#define MBOX_SEND() \
	CachePreDMA(m, &n, 0); \
	mbox_send(8, (ULONG)m); \
	mbox_recv(8); \
	CachePostDMA(m, &n, 0);

/*
#define LE32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
*/

extern APTR MailBox;

UBYTE __req[4096*4];

/**********************************************************
 ** 
 ** mbox_init()
 ** 
 **********************************************************/

BOOL mbox_init(void)
{
	BOOL result = FALSE;
	
	if (DeviceTreeBase)
	{
		/* Get VC4 physical address of mailbox interface.
		   Subsequently it will be translated to m68k physical address */
        
		APTR key;
		
		if (key = DT_OpenKey("/aliases"))
		{
			CONST_STRPTR mbox_alias = DT_GetPropValue(DT_FindProperty(key, "mailbox"));
			
			DT_CloseKey(key);
			
			if (mbox_alias != NULL)
			{
				key = DT_OpenKey(mbox_alias);
				
				if (key)
				{
					ULONG size_cells = 1;
					ULONG address_cells = 1;
					
					CONST ULONG *siz = GetPropValueRecursive(key, "#size_cells");
					CONST ULONG *adr = GetPropValueRecursive(key, "#address-cells");
					CONST ULONG *reg = DT_GetPropValue(DT_FindProperty(key, "reg"));
					
					if (siz != NULL) size_cells = *siz;
					if (adr != NULL) address_cells = *adr;
					
					MailBox = (APTR)reg[(1 * address_cells) - 1];
					
					DT_CloseKey(key);
				}
			}
		}
		
		/* Open /soc key and learn about VC4 to CPU mapping.
		   Use it to adjust the addresses obtained above */
		
		if (key = DT_OpenKey("/soc"))
		{
			ULONG phys_vc4;
			ULONG phys_cpu;
			
			ULONG size_cells = 1;
			ULONG address_cells = 1;
			
			CONST ULONG *siz = GetPropValueRecursive(key, "#size_cells");
			CONST ULONG *adr = GetPropValueRecursive(key, "#address-cells");
			CONST ULONG *reg = DT_GetPropValue(DT_FindProperty(key, "ranges"));
			
			if (siz != NULL) size_cells = *siz;
			if (adr != NULL) address_cells = *adr;
			
			phys_vc4 = reg[(1 * address_cells) - 1];
			phys_cpu = reg[(2 * address_cells) - 1];
			
			MailBox = (APTR)((ULONG)MailBox - phys_vc4 + phys_cpu);
			
			DT_CloseKey(key);
			
			result = TRUE;
		}
	}
	
	return (result);
}

/**********************************************************
 ** 
 ** mbox_recv()
 ** 
 **********************************************************/

static ULONG mbox_recv(ULONG channel)
{
	ULONG status, response;
	
	volatile ULONG *mbox_read   = (ULONG*)((ULONG)MailBox + 0x00);
	volatile ULONG *mbox_status = (ULONG*)((ULONG)MailBox + 0x18);
	
	do
	{
		do
		{
			status = asm_le32(*mbox_status);
			asm_nop();
		}
		while (status & MBOX_RX_EMPTY);
		asm_nop();
		
		response = asm_le32(*mbox_read);
		asm_nop();
	}
	while ((response & MBOX_CHANMASK) != channel);
	
	return (response & ~MBOX_CHANMASK);
}

/**********************************************************
 ** 
 ** mbox_send()
 ** 
 **********************************************************/

static void mbox_send(ULONG channel, ULONG data)
{
	ULONG status;
	
	volatile ULONG *mbox_write  = (ULONG*)((ULONG)MailBox + 0x20);
	volatile ULONG *mbox_status = (ULONG*)((ULONG)MailBox + 0x18);
	
	data &= ~MBOX_CHANMASK;
	data |= channel & MBOX_CHANMASK;
	
	do
	{
		status = asm_le32(*mbox_status);
		asm_nop();
	}
	while (status & MBOX_TX_FULL);
	asm_nop();
	
	*mbox_write = asm_le32(data);
}

/**********************************************************
 ** 
 ** mbox_get_board_macaddr()
 ** 
 **********************************************************/

void mbox_get_board_macaddr(
	ULONG *a, // mac address low
	ULONG *b) // mac address high
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_BOARD_MACADDR);    // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = 0;                              // Response[0]: MAC addr 4 bytes
	m[6] = 0;                              // Response[1]: MAC addr 2 bytes
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = m[5];                      // mac address low
	if (b) *b = m[6];                      // mac address high
}

/**********************************************************
 ** 
 ** mbox_get_board_model()
 ** 
 **********************************************************/

void mbox_get_board_model(
	ULONG *a, // board model
	ULONG *b) // board revision
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(4+4);             // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_BOARD_MODEL);     // Tag identifier
	m[ 3] = asm_le32(4);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = 0;                             // Response[0]: model
	m[ 6] = asm_le32(TAG_GET_BOARD_REVISION);  // Tag identifier
	m[ 7] = asm_le32(4);                       // Response size in bytes
	m[ 8] = 0;                             // Request
	m[ 9] = 0;                             // Response[0]: revision
	m[10] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[5]);                // board model
	if (b) *b = asm_le32(m[9]);                // board revision
}

/**********************************************************
 ** 
 ** mbox_get_boardserial()
 ** 
 **********************************************************/

void mbox_get_board_serial(
	ULONG *a, // serial-number low
	ULONG *b) // serial-number high
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_BOARD_SERIAL);     // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = 0;                              // Response[0]: high
	m[6] = 0;                              // Response[1]: low
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[6]);                // serial-number low
	if (b) *b = asm_le32(m[5]);                // serial-number high
}

/**********************************************************
 ** 
 ** mbox_get_clock_rate()
 ** 
 **********************************************************/

void mbox_get_clock_rate(
	ULONG id, // clock id
	ULONG *a, // clock rate
	ULONG *b, // clock rate measured
	ULONG *c, // clock rate min
	ULONG *d) // clock rate max
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(4*5);             // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_CLOCK_RATE);      // Tag identifier
	m[ 3] = asm_le32(8);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = asm_le32(id);                      // Response[0]
	m[ 6] = 0;                             // Response[1]
	m[ 7] = asm_le32(TAG_GET_CLOCK_RATE_M);    // Tag identifier
	m[ 8] = asm_le32(8);                       // Response size in bytes
	m[ 9] = 0;                             // Request
	m[10] = asm_le32(id);                      // Response[0]
	m[11] = 0;                             // Response[1]
	m[12] = asm_le32(TAG_GET_CLOCKRATE_MIN);   // Tag identifier
	m[13] = asm_le32(8);                       // Response size in bytes
	m[14] = 0;                             // Request
	m[15] = asm_le32(id);                      // Response[0]
	m[16] = 0;                             // Response[1]
	m[17] = asm_le32(TAG_GET_CLOCKRATE_MAX);   // Tag identifier
	m[18] = asm_le32(8);                       // Response size in bytes
	m[19] = 0;                             // Request
	m[20] = asm_le32(id);                      // Response[0]
	m[21] = 0;                             // Response[1]
	m[22] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[ 6]);               // clock rate
	if (b) *b = asm_le32(m[11]);               // clock rate measured
	if (c) *c = asm_le32(m[16]);               // clock rate min
	if (d) *d = asm_le32(m[21]);               // clock rate max
}

/**********************************************************
 ** 
 ** mbox_get_clock_state()
 ** 
 **********************************************************/

void mbox_get_clock_state(
	ULONG id, // clock id
	ULONG *a) // clock state
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_CLOCK_STATE);      // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(id);                       // Response[0]
	m[6] = 0;                              // Response[1]
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[6]);                // clock state
}

/**********************************************************
 ** 
 ** mbox_get_command_line()
 ** 
 **********************************************************/

void mbox_get_command_line(
	STRPTR *a) // string
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(3+1024);            // m[] size
	
	memset(__req, 0, sizeof(__req));
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_COMMAND_LINE);     // Tag identifier
	m[3] = asm_le32(1024);                     // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = 0;                              // Response[0]: string
	m[6] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	m[1024] = 0; // NULL-termination
	
	if (a) *a = (STRPTR)&m[5];             // string address
}

/**********************************************************
 ** 
 ** mbox_get_dma_channels()
 ** 
 **********************************************************/

void mbox_get_dma_channels(
	ULONG *a) // dma channels
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(4);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_DMA_CHANNELS);     // Tag identifier
	m[3] = asm_le32(4);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = 0;                              // Response[0]: mask
	m[6] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[5]);                // dma channels
}

/**********************************************************
 ** 
 ** mbox_get_edid_block()
 ** 
 **********************************************************/

ULONG mbox_get_edid_block(
	ULONG   block,  // block number
	STRPTR *buffer) // edid block
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(142);             // m[] size
	
	memset(__req, 0, sizeof(__req));       // m[] clear
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_EDID_BLOCK);      // Tag identifier
	m[ 3] = asm_le32(136);                     // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = asm_le32(block);                   // Response[0]: block number
	m[ 6] = 0;                             // Response[1]: status
	m[ 7] = 0;                             // Response[2]: edid block (128 bytes)
	m[40] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (buffer)
		*buffer = (STRPTR)&m[7];           // edid block (128 bytes)
	
	return (asm_le32(m[6]));
}

/**********************************************************
 ** 
 ** mbox_get_firmware_revision()
 ** 
 **********************************************************/

void mbox_get_firmware_revision(
	ULONG *a) // firmware revision
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(4);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_FIRMWARE_REV);     // Tag identifier
	m[3] = asm_le32(4);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = 0;                              // Response[0]
	m[6] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[5]);                // firmware revision
}

/**********************************************************
 ** 
 ** mbox_get_frame_buffer()
 ** 
 **********************************************************/

void mbox_get_frame_buffer(
	ULONG *pw, // physical width
	ULONG *ph, // physical height
	ULONG *vw, // virtual width
	ULONG *vh, // virtual height
	ULONG *de, // depth
	ULONG *po, // pixel order
	ULONG *am, // alpha mode
	ULONG *pi) // pitch
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5+5+4+4+4+4);     // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	
	m[ 2] = asm_le32(TAG_GET_PHYSICAL_SIZE);   // Tag identifier
	m[ 3] = asm_le32(8);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = 0;                             // Response[0]: width
	m[ 6] = 0;                             // Response[1]: height
	
	m[ 7] = asm_le32(TAG_GET_VIRTUAL_SIZE);    // Tag identifier
	m[ 8] = asm_le32(8);                       // Response size in bytes
	m[ 9] = 0;                             // Request
	m[10] = 0;                             // Response[0]: width
	m[11] = 0;                             // Response[1]: height
	
	m[12] = asm_le32(TAG_GET_DEPTH);           // Tag identifier
	m[13] = asm_le32(4);                       // Response size in bytes
	m[14] = 0;                             // Request
	m[15] = 0;                             // Response[0]: depth
	
	m[16] = asm_le32(TAG_GET_PIXEL_ORDER);     // Tag identifier
	m[17] = asm_le32(4);                       // Response size in bytes
	m[18] = 0;                             // Request
	m[19] = 0;                             // Response[0]: pixel order
	
	m[20] = asm_le32(TAG_GET_ALPHA_MODE);      // Tag identifier
	m[21] = asm_le32(4);                       // Response size in bytes
	m[22] = 0;                             // Request
	m[23] = 0;                             // Response[0]: pitch
	
	m[24] = asm_le32(TAG_GET_PITCH);           // Tag identifier
	m[25] = asm_le32(4);                       // Response size in bytes
	m[26] = 0;                             // Request
	m[27] = 0;                             // Response[0]: pitch
	
	m[28] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (pw) *pw = asm_le32(m[ 5]);             // physical width
	if (ph) *ph = asm_le32(m[ 6]);             // physical height
	if (vw) *vw = asm_le32(m[10]);             // virtual width
	if (vh) *vh = asm_le32(m[11]);             // virtual height
	if (de) *de = asm_le32(m[15]);             // depth
	if (po) *po = asm_le32(m[19]);             // pixel order
	if (am) *am = asm_le32(m[23]);             // alpha mode
	if (pi) *pi = asm_le32(m[27]);             // pitch
}

/**********************************************************
 ** 
 ** mbox_get_led_status()
 ** 
 **********************************************************/

void mbox_get_led_status(
	ULONG id, // pin number
	ULONG *a) // status
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_LED_STATUS);       // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(id);                       // Response[0]: pin number
	m[6] = 0;                              // Response[1]: status
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[6]);                // status led
}

/**********************************************************
 ** 
 ** mbox_get_memory()
 ** 
 **********************************************************/

void mbox_get_memory(
	ULONG *a, // ARM memory base address
	ULONG *b, // ARM memory size in bytes
	ULONG *c, // VC4 memory base address
	ULONG *d) // VC4 memory size in bytes
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5+5);             // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_ARM_MEMORY);      // Tag identifier
	m[ 3] = asm_le32(8);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = 0;                             // Response[0]: base
	m[ 6] = 0;                             // Response[1]: size
	m[ 7] = asm_le32(TAG_GET_VC_MEMORY);       // Tag identifier
	m[ 8] = asm_le32(8);                       // Response size in bytes
	m[ 9] = 0;                             // Request
	m[10] = 0;                             // Response[0]: base
	m[11] = 0;                             // Response[1]: size
	m[12] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[ 5]);               // ARM memory base address
	if (b) *b = asm_le32(m[ 6]);               // ARM memory size in bytes
	if (c) *c = asm_le32(m[10]);               // VC4 memory base address
	if (d) *d = asm_le32(m[11]);               // VC4 memory size in bytes
}

/**********************************************************
 ** 
 ** mbox_get_power_state()
 ** 
 **********************************************************/

void mbox_get_power_state(
	ULONG id, // device id
	ULONG *a, // power state
	ULONG *b) // power timing
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5+5);             // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_POWER_STATE);     // Tag identifier
	m[ 3] = asm_le32(8);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = asm_le32(id);                      // Response[0]: id
	m[ 6] = 0;                             // Response[1]: state
	m[ 7] = asm_le32(TAG_GET_POWER_TIMING);    // Tag identifier
	m[ 8] = asm_le32(8);                       // Response size in bytes
	m[ 9] = 0;                             // Request
	m[10] = asm_le32(id);                      // Response[0]: id
	m[11] = 0;                             // Response[1]: timing
	m[12] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[ 6]);               // power status
	if (b) *b = asm_le32(m[11]);               // power timing
}

/**********************************************************
 ** 
 ** mbox_get_temperature()
 ** 
 **********************************************************/

void mbox_get_temperature(
	ULONG id, // temperature id
	ULONG *a, // temperature
	ULONG *b) // temperature max
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5+5);             // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_TEMPERATURE);     // Tag identifier
	m[ 3] = asm_le32(8);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = asm_le32(id);                      // Response[0]: id
	m[ 6] = 0;                             // Response[1]: temp
	m[ 7] = asm_le32(TAG_GET_TEMPERATURE_MAX); // Tag identifier
	m[ 8] = asm_le32(8);                       // Response size in bytes
	m[ 9] = 0;                             // Request
	m[10] = asm_le32(id);                      // Response[0]: id
	m[11] = 0;                             // Response[1]: temp
	m[12] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[1+(1*5)]);          // temperature
	if (b) *b = asm_le32(m[1+(2*5)]);          // temperature max
}

/**********************************************************
 ** 
 ** mbox_get_turbo()
 ** 
 **********************************************************/

void mbox_get_turbo(
	ULONG id, // turbo id
	ULONG *a) // level
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_GET_TURBO);            // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(id);                       // Response[0]: id
	m[6] = 0;                              // Response[1]: level
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[1+(1*5)]);          // level
}

/**********************************************************
 ** 
 ** mbox_get_voltage()
 ** 
 **********************************************************/

void mbox_get_voltage(
	ULONG id, // voltage id
	ULONG *a, // voltage
	ULONG *b, // voltage min
	ULONG *c) // voltage max
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5+5+5);           // m[] size
	
	m[ 0] = asm_le32(n);                       // Message size in bytes
	m[ 1] = 0;                             // Process request
	m[ 2] = asm_le32(TAG_GET_VOLTAGE);         // Tag identifier
	m[ 3] = asm_le32(8);                       // Response size in bytes
	m[ 4] = 0;                             // Request
	m[ 5] = asm_le32(id);                      // Response[0]: id
	m[ 6] = 0;                             // Response[1]: voltage
	m[ 7] = asm_le32(TAG_GET_VOLTAGE_MIN);     // Tag identifier
	m[ 8] = asm_le32(8);                       // Response size in bytes
	m[ 9] = 0;                             // Request
	m[10] = asm_le32(id);                      // Response[0]: id
	m[11] = 0;                             // Response[1]: voltage
	m[12] = asm_le32(TAG_GET_VOLTAGE_MAX);     // Tag identifier
	m[13] = asm_le32(8);                       // Response size in bytes
	m[14] = 0;                             // Request
	m[15] = asm_le32(id);                      // Response[0]: id
	m[16] = 0;                             // Response[1]: voltage
	m[17] = 0;                             // End of message
	
	MBOX_SEND();                           // Send request
	
	if (a) *a = asm_le32(m[1+(1*5)]);          // voltage
	if (b) *b = asm_le32(m[1+(2*5)]);          // voltage min
	if (c) *c = asm_le32(m[1+(3*5)]);          // voltage max
}

/**********************************************************
 ** 
 ** mbox_set_alpha_mode()
 ** 
 **********************************************************/

void mbox_set_alpha_mode(
	ULONG state) // state
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(4);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_SET_ALPHA_MODE);       // Tag identifier
	m[3] = asm_le32(4);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(state);                    // Response[0]: state
	m[6] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
}

/**********************************************************
 ** 
 ** mbox_set_clock_rate()
 ** 
 **********************************************************/

void mbox_set_clock_rate(
	ULONG id,   // clock id
	ULONG rate, // rate (in Hz)
	ULONG skip) // skip setting turbo
{
	ULONG a, b, c, d;
	
	mbox_get_clock_rate(
		id,  // clock id
		&a,  // clock rate
		&b,  // clock rate measured
		&c,  // clock rate minimum
		&d); // clock rate maximum
	
	if (rate != a)
	{
		ULONG *m = MBOX_REQUEST();         // m[] addr
		ULONG  n = MBOX_SIZE(6);           // m[] size
		
		if (rate < c) rate = c;
		if (rate > d) rate = d;
		
		m[0] = asm_le32(n);                    // Message size in bytes
		m[1] = 0;                          // Process request
		m[2] = asm_le32(TAG_SET_CLOCK_RATE);   // Tag identifier
		m[3] = asm_le32(8);                    // Response size in bytes
		m[4] = 0;                          // Request
		m[5] = asm_le32(id);                   // Response[0]: clock id
		m[6] = asm_le32(rate);                 // Response[1]: rate (in Hz)
		m[7] = asm_le32(skip);                 // Response[2]: skip setting turbo
		m[8] = 0;                          // End of message
		
		MBOX_SEND();                       // Send request
	}
}

/**********************************************************
 ** 
 ** mbox_set_led_status()
 ** 
 **********************************************************/

void mbox_set_led_status(
	ULONG id,     // pin number
	ULONG status) // status
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_SET_LED_STATUS);       // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(id);                       // Response[0]: pin number
	m[6] = asm_le32(status);                   // Response[1]: status
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
}

/**********************************************************
 ** 
 ** mbox_set_screen_blank()
 ** 
 **********************************************************/

void mbox_set_screen_blank(
	ULONG state) // state
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(4);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_BLANK_SCREEN);         // Tag identifier
	m[3] = asm_le32(4);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(state);                    // Response[0]: state
	m[6] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
}

/**********************************************************
 ** 
 ** mbox_set_screen_gamma()
 ** 
 **********************************************************/

void mbox_set_screen_gamma(
	ULONG id,    // display number
	ULONG table) // gamma table
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_SET_SCREEN_GAMMA);     // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(id);                       // Response[0]: display number
	m[6] = asm_le32(table);                    // Response[1]: gamma table
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
}

/**********************************************************
 ** 
 ** mbox_set_turbo()
 ** 
 **********************************************************/

void mbox_set_turbo(
	ULONG id,    // turbo id
	ULONG level) // level
{
	ULONG *m = MBOX_REQUEST();             // m[] addr
	ULONG  n = MBOX_SIZE(5);               // m[] size
	
	m[0] = asm_le32(n);                        // Message size in bytes
	m[1] = 0;                              // Process request
	m[2] = asm_le32(TAG_SET_TURBO);            // Tag identifier
	m[3] = asm_le32(8);                        // Response size in bytes
	m[4] = 0;                              // Request
	m[5] = asm_le32(id);                       // Response[0]: id
	m[6] = asm_le32(level);                    // Response[1]: level
	m[7] = 0;                              // End of message
	
	MBOX_SEND();                           // Send request
}

/**********************************************************
 ** 
 ** END OF FILE
 ** 
 **********************************************************/
