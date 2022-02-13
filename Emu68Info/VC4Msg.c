/**********************************************************
 ** 
 ** VC4 Helper functions
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

#include "VC4Msg.h"

/**********************************************************
 ** 
 ** DEFINES
 ** 
 **********************************************************/

ULONG VC4_GetInfo(void)
{
	struct MsgPort *vc4Port;
	struct MsgPort *replyPort;
	
	if (vc4Port = FindPort("Emu68 VC4"))
	{
		if (replyPort = CreateMsgPort())
		{
			struct VC4Msg cmd;
			
			cmd.msg.mn_ReplyPort = replyPort;
			cmd.msg.mn_Length = sizeof(struct VC4Msg);
			
			// VCMD_GET_PHASE
			
			cmd.cmd = VCMD_GET_PHASE;
			PutMsg(vc4Port, &cmd.msg);
			WaitPort(replyPort);
			GetMsg(replyPort);
			printf("Phase:     %ld\n", cmd.GetPhase);//.val);
			
            // VCMD_GET_SCALER
            
			cmd.cmd = VCMD_GET_SCALER;
			PutMsg(vc4Port, &cmd.msg);
			WaitPort(replyPort);
			GetMsg(replyPort);
			
			printf("Scaler1:   %ld\n", cmd.GetScaler.val & 1);
			printf("Scaler2:   %ld\n", cmd.GetScaler.val & 2);

            // VCMD_GET_KERNEL
            
			cmd.cmd = VCMD_GET_KERNEL;
			PutMsg(vc4Port, &cmd.msg);
			WaitPort(replyPort);
			GetMsg(replyPort);
			
			printf("Kernel:    %ld\n", cmd.GetKernel.kernel);
			printf("KernelB:   %ld\n", cmd.GetKernel.b);
			printf("KernelC:   %ld\n", cmd.GetKernel.c);
			
			DeleteMsgPort(replyPort);
		}
	}
	else
	{
		printf("Cannot find Emu68-VC4 port.\n");
	}
	
	return (0);
}

/**********************************************************
 ** 
 ** END OF FILE
 ** 
 **********************************************************/
