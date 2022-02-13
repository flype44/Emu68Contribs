/**********************************************************
 ** 
 ** DeviceTree Helper functions
 ** 
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/errors.h>

#include <proto/exec.h>
#include <proto/devicetree.h>

/**********************************************************
 ** 
 ** GetProp()
 ** 
 **********************************************************/

APTR GetProp(STRPTR key, STRPTR property)
{
	APTR p;
	
	if (p = DT_OpenKey(key))
	{
		return DT_FindProperty(p, property);
	}
	
	return (0);
}

/**********************************************************
 ** 
 ** GetPropInt()
 ** 
 **********************************************************/

ULONG GetPropInt(STRPTR key, STRPTR property)
{
	APTR bProperty;
	CONST_APTR bValue;
	
	if (bProperty = GetProp(key, property))
	{
		if (bValue = DT_GetPropValue(bProperty))
		{
			ULONG length = DT_GetPropLen(bProperty);
			
		//	printf("l:%lu\n", length);
			
			if (length == 4)
			{
				return (*(ULONG *)bValue);
			}
			
			if (length == 8)
			{
				return (*(ULONG *)bValue);
			}
		}
	}
	
	return (0);
}

/**********************************************************
 ** 
 ** GetPropStr()
 ** 
 **********************************************************/

STRPTR GetPropStr(UBYTE * buffer, STRPTR key, STRPTR property)
{
	APTR bProperty;
	CONST_APTR bValue;
	
	if (bProperty = GetProp(key, property))
	{
		if (bValue = DT_GetPropValue(bProperty))
		{
			ULONG length = DT_GetPropLen(bProperty);
			if (length > 4095) length = 4095;
			strncpy(buffer, bValue, length);
			buffer[length] = '\0';
			return (buffer);
		}
	}
	
	return (NULL);
}

/**********************************************************
 ** 
 ** GetPropValueRecursive()
 ** 
 **********************************************************/

CONST_APTR GetPropValueRecursive(APTR key, CONST_STRPTR property)
{
    do
    {
		APTR p;
		
		if (p = DT_FindProperty(key, property))
		{
			return DT_GetPropValue(p);
		}
		
		key = DT_GetParent(key);
		  
	} while (key);
	
	return NULL;
}

/**********************************************************
 ** 
 ** GetField()
 ** 
 **********************************************************/

LONG GetField(STRPTR str, STRPTR key, ULONG base)
{
	STRPTR p;
	
	if (p = strstr(str, key))
	{
		return (LONG)strtol(p + strlen(key), NULL, base);
	}
	
	return (0L);
}

/**********************************************************
 ** 
 ** END OF FILE
 ** 
 **********************************************************/
