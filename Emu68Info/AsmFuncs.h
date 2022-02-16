#ifndef __ASM_FUNCS_H__
#define __ASM_FUNCS_H__

#include <exec/types.h>

#ifdef __GNUC__
#define ASM
#define REG(r,y)	y __asm( # r )
#else
#define ASM __asm __saveds
#define REG(r,y)	register __ ## r y
#endif

void ASM asm_nop(void);
ULONG ASM asm_le32(REG(d0,ULONG a));
ULONG ASM asm_div64(REG(d0,ULONG a), REG(d1,ULONG b), REG(d2,ULONG c));

ULONG asm_cpu_cacr(void);
ULONG asm_cpu_vbr(void);
ULONG asm_cpu_rate(void);
ULONG asm_fpu_rate(void);

ULONG ASM asm_cnt_frq(void);
ULONG ASM asm_cnt_val_lo(void);
ULONG ASM asm_cnt_val_hi(void);
ULONG ASM asm_cnt_68k_lo(void);
ULONG ASM asm_cnt_68k_hi(void);
ULONG ASM asm_cnt_arm_lo(void);
ULONG ASM asm_cnt_arm_hi(void);

ULONG ASM asm_jit_size(void);
ULONG ASM asm_jit_free(void);
ULONG ASM asm_jit_count(void);
ULONG ASM asm_jit_sfthresh(void);
ULONG ASM asm_jit_sfthresh_set(REG(d0, ULONG a));
ULONG ASM asm_jit_ctrl(void);
ULONG ASM asm_jit_ctrl_set(REG(d0, ULONG a));
ULONG ASM asm_jit_cmiss(void);

ULONG ASM asm_dbg_ctrl(void);
ULONG ASM asm_dbg_addr_lo(void);
ULONG ASM asm_dbg_addr_hi(void);

#endif
