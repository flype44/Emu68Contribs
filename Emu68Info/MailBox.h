#ifndef _MAILBOX_H
#define _MAILBOX_H

/**********************************************************
 ** 
 ** Includes
 ** 
 **********************************************************/

#include <exec/exec.h>
#include <exec/types.h>

/**********************************************************
 ** 
 ** Defines
 ** 
 **********************************************************/

// mailbox tags

#define TAG_GET_FIRMWARE_REV    (0x00000001)

#define TAG_GET_BOARD_MODEL     (0x00010001)
#define TAG_GET_BOARD_REVISION  (0x00010002)
#define TAG_GET_BOARD_MACADDR   (0x00010003)
#define TAG_GET_BOARD_SERIAL    (0x00010004)
#define TAG_GET_ARM_MEMORY      (0x00010005)
#define TAG_GET_VC_MEMORY       (0x00010006)
#define TAG_GET_CLOCKS          (0x00010007)

#define TAG_GET_POWER_STATE     (0x00020001)
#define TAG_GET_POWER_TIMING    (0x00020002)
#define TAG_SET_POWER_STATE     (0x00028001)

#define TAG_GET_CLOCK_STATE     (0x00030001)
#define TAG_GET_CLOCK_RATE      (0x00030002)
#define TAG_GET_VOLTAGE         (0x00030003)
#define TAG_GET_CLOCKRATE_MAX   (0x00030004)
#define TAG_GET_VOLTAGE_MAX     (0x00030005)
#define TAG_GET_TEMPERATURE     (0x00030006)
#define TAG_GET_CLOCKRATE_MIN   (0x00030007)
#define TAG_GET_VOLTAGE_MIN     (0x00030008)
#define TAG_GET_TURBO           (0x00030009)
#define TAG_GET_TEMPERATURE_MAX (0x0003000a)

#define TAG_ALLOCATE_MEMORY     (0x0003000c)
#define TAG_LOCK_MEMORY         (0x0003000d)
#define TAG_UNLOCK_MEMORY       (0x0003000e)
#define TAG_RELEASE_MEMORY      (0x0003000f)

#define TAG_GET_EXECUTE_CODE    (0x00030010)
#define TAG_GET_DISPMANX_HANDLE (0x00030014)

#define TAG_GET_EDID_BLOCK      (0x00030020)
#define TAG_GET_LED_STATUS      (0x00030041)
#define TAG_GET_CLOCK_RATE_M    (0x00030047)

#define TAG_TEST_LED_STATUS     (0x00034041)

#define TAG_SET_CLOCK_STATE     (0x00038001)
#define TAG_SET_CLOCK_RATE      (0x00038002)
#define TAG_SET_VOLTAGE         (0x00038003)
#define TAG_SET_TURBO           (0x00038009)
#define TAG_SET_LED_STATUS      (0x00038041)

#define TAG_ALLOCATE_BUFFER     (0x00040001)
#define TAG_BLANK_SCREEN        (0x00040002)
#define TAG_GET_PHYSICAL_SIZE   (0x00040003)
#define TAG_GET_VIRTUAL_SIZE    (0x00040004)
#define TAG_GET_DEPTH           (0x00040005)
#define TAG_GET_PIXEL_ORDER     (0x00040006)
#define TAG_GET_ALPHA_MODE      (0x00040007)
#define TAG_GET_PITCH           (0x00040008)
#define TAG_GET_VIRTUAL_OFFSET  (0x00040009)
#define TAG_GET_OVERSCAN        (0x0004000a)
#define TAG_GET_PALETTE         (0x0004000b)

#define TAG_TEST_PHYSICAL_SIZE  (0x00044003)
#define TAG_TEST_VIRTUAL_SIZE   (0x00044004)
#define TAG_TEST_DEPTH          (0x00044005)
#define TAG_TEST_PIXEL_ORDER    (0x00044006)
#define TAG_TEST_ALPHA_MODE     (0x00044007)
#define TAG_TEST_VIRTUAL_OFFSET (0x00044009)
#define TAG_TEST_OVERSCAN       (0x0004400a)
#define TAG_TEST_PALETTE        (0x0004400b)

#define TAG_RELEASE_BUFFER      (0x00048001)
#define TAG_SET_PHYSICAL_SIZE   (0x00048003)
#define TAG_SET_VIRTUAL_SIZE    (0x00048004)
#define TAG_SET_DEPTH           (0x00048005)
#define TAG_SET_PIXEL_ORDER     (0x00048006)
#define TAG_SET_ALPHA_MODE      (0x00048007)
#define TAG_SET_VIRTUAL_OFFSET  (0x00048009)
#define TAG_SET_OVERSCAN        (0x0004800a)
#define TAG_SET_PALETTE         (0x0004800b)

#define TAG_SET_CURSOR_INFO     (0x00008010)
#define TAG_SET_CURSOR_STATE    (0x00008011)
#define TAG_SET_SCREEN_GAMMA    (0x00008012)

#define TAG_GET_COMMAND_LINE    (0x00050001)
#define TAG_GET_DMA_CHANNELS    (0x00060001)

// led id

#define LED_PIN_STATUS          (42)
#define LED_PIN_POWER           (130)

// voltage id

#define VOLTAGE_ID_RESERVED     (0x00000000)
#define VOLTAGE_ID_CORE         (0x00000001)
#define VOLTAGE_ID_SDRAM_C      (0x00000002)
#define VOLTAGE_ID_SDRAM_P      (0x00000003)
#define VOLTAGE_ID_SDRAM_I      (0x00000004)

/**********************************************************
 ** 
 ** Prototypes
 ** 
 **********************************************************/

BOOL mbox_init(void);

void mbox_set_alpha_mode(
	ULONG state);     // state

void mbox_set_screen_blank(
	ULONG state);     // state

void mbox_get_frame_buffer(
	ULONG *pw,        // physical width
	ULONG *ph,        // physical height
	ULONG *vw,        // virtual width
	ULONG *vh,        // virtual height
	ULONG *de,        // depth
	ULONG *po,        // pixel order
	ULONG *am,        // alpha mode
	ULONG *pi);       // pitch

void mbox_get_board_macaddr(ULONG *a, ULONG *b);
void mbox_get_board_model(ULONG *a, ULONG *b);
void mbox_get_board_serial(ULONG *a, ULONG *b);
void mbox_get_clock_rate(ULONG id, ULONG *a, ULONG *b, ULONG *c, ULONG *d);
void mbox_get_clock_state(ULONG id, ULONG *a);
void mbox_get_command_line(STRPTR *a);
void mbox_get_dma_channels(ULONG *a);

ULONG mbox_get_edid_block(
	ULONG   block,    // block number
	STRPTR *buffer);  // edid block

void mbox_get_firmware_revision(ULONG *a);
void mbox_get_led_status(ULONG id, ULONG *a);
void mbox_get_memory(ULONG *a, ULONG *b, ULONG *c,ULONG *d);
void mbox_get_power_state(ULONG id, ULONG *a, ULONG *b);
void mbox_get_temperature(ULONG id, ULONG *a, ULONG *b);
void mbox_get_turbo(ULONG id, ULONG *a);
void mbox_get_voltage(ULONG id, ULONG *a, ULONG *b, ULONG *c);

void mbox_set_clock_rate(ULONG id, ULONG a, ULONG b);
void mbox_set_led_status(ULONG id, ULONG a);
void mbox_set_turbo(ULONG id, ULONG a);
void mbox_set_screen_gamma(ULONG id, ULONG table);

#endif /* _MAILBOX_H */
