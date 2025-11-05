#ifndef __ASM_FUNCS_H
#define __ASM_FUNCS_H

#include <exec/types.h>

#ifndef ASM
 #ifdef __GNUC__
  #define ASM
  #define REG(r,y)	y __asm( # r )
 #else
  #define ASM __asm __saveds
  #define REG(r,y)	register __ ## r y
 #endif
#endif

VOID  ASM asm_nop(VOID);
ULONG ASM asm_le32(REG(d0, ULONG a));

#endif
