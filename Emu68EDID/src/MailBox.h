#ifndef __MAILBOX_H
#define __MAILBOX_H

/******************************************************************************
 * 
 * INCLUDES
 * 
 *****************************************************************************/

#include <exec/exec.h>
#include <exec/types.h>

/******************************************************************************
 * 
 * DEFINES
 * 
 *****************************************************************************/

// Mailbox register offsets (from base)
#define MBOX_READ   (0x00UL)
#define MBOX_STATUS (0x18UL)
#define MBOX_WRITE  (0x20UL)

// Mailbox status bits
#define MBOX_STATUS_FULL  (1UL<<31)
#define MBOX_STATUS_EMPTY (1UL<<30)

// Mailbox channel mask
#define MBOX_CHANMASK (0xF)

// Mailbox channel to use
#define PROPERTY_CHANNEL (8UL)

// Mailbox tags
#define TAG_GET_EDID_BLOCK (0x00030020)

/******************************************************************************
 * 
 * PROTOTYPES
 * 
 *****************************************************************************/

BOOL mbox_init(VOID);
VOID mbox_free(VOID);
BOOL mbox_get_edid_block(ULONG block_number, UBYTE * edid_out);

#endif /* _MAILBOX_H */
