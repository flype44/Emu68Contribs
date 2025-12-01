# **Emu68Version** version 1.0

## **NAME**

```
	EMU68VERSION - Finds PiStorm/Emu68 version and revision numbers.
```

## **FORMAT**

```
	EMU68VERSION [<VERSION #>] [<REVISION #>] [<HOTFIX #>] 
		[SHORT] [FULL] [GITHASH] [VARIANT]
```

## **TEMPLATE**

```
	VERSION/N,REVISION/N,HOTFIX/N,SHORT/S,FULL/S,GITHASH/S,VARIANT/S
```

## **PATH**

```
	C:EMU68VERSION
```

## **IMPORTANT**

```
	This command requires the Emu68 JIT baremetal emulator.
```

## **FUNCTION**

```
	EMU68VERSION prints the version string of the currently running Emu68.
	It can also test for a specific version/revision/hotfix and warn if
	the version/revision/hotfix is lower than the specified value.
	This is useful in scripts.
	
	The SHORT option prints a shorter version string.
	
	The FULL option also prints the last modification time and additional 
	information.
	
	The GITHASH option prints the 40 digits of the Emu68 GIT hash string.
	This hash string corresponds to some specific GitHub build number.
	
	The VARIANT option prints the Emu68 variant. Values can be for example 
	pistorm, pistorm16, pistorm32lite, ...
	
	When a <VERSION #> is specified, VERSION sets the condition flag 
	to 5 (WARN) if the version is lower than the specified value.
	
	When a <REVISION #> is specified, REVISION sets the condition flag 
	to 5 (WARN) if the revision is lower than the specified value.
	
	When a <HOTFIX #> is specified, HOTFIX sets the condition flag 
	to 5 (WARN) if the hotfix is lower than the specified value.
```

## **RETURN CODES**

```
	FAIL    (20) Bad argument
	FAIL    (20) Can't allocate memory
	ERROR   (10) Can't open devicetree.resource
	ERROR   (10) Can't open emu68 key
	ERROR   (10) Can't open property (idstring)
	ERROR   (10) Can't open property (git-hash)
	ERROR   (10) Can't open property (variant)
	WARNING (5)  The Emu68 version is lower than the specified value
	SUCCESS (0)  The Emu68 version is greater or equal than the specified value
```

## **EXAMPLES**

```
	1> C:EMU68VERSION
	Emu68 1.0.6
```

```
	1> C:EMU68VERSION SHORT
	1.0.6
```

```
	1> C:EMU68VERSION FULL
	Emu68 1.0.6 (27.10.2025) git:562a2aa
```

```
	1> C:EMU68VERSION GITHASH
	562a2aaf75fdae1e77eb80940677853d53af034f
```

```
	1> C:EMU68VERSION VARIANT
	pistorm32lite
```

```
	1> C:EMU68VERSION FULL GITHASH VARIANT
	Emu68 1.0.6 (27.10.2025) git:562a2aa
	562a2aaf75fdae1e77eb80940677853d53af034f
	pistorm32lite
```

```
	1> C:EMU68VERSION 1
	$RC=5 if the Emu68 version is lower than to 1
	$RC=0 if the Emu68 version is greater than or equal to 1
```

```
	1> C:EMU68VERSION 1 0
	$RC=5 if the Emu68 version is lower than to 1.0
	$RC=0 if the Emu68 version is greater than or equal to 1.0
```

```
	1> C:EMU68VERSION 1 0 6
	$RC=5 if the Emu68 version is lower than to 1.0.6
	$RC=0 if the Emu68 version is greater than or equal to 1.0.6
```

```
	1> C:EMU68VERSION HELP
	Emu68Version 1.0 (30.11.2025) [SAS/C 6.59] Philippe CARPENTIER
	
	HELP/S     : Print this help
	VERSION/N  : Check the Emu68 version (major)
	REVISION/N : Check the Emu68 revision (minor)
	HOTFIX/N   : Check the Emu68 hotfix (patch)
	SHORT/S    : Print the Emu68 version string (short)
	FULL/S     : Print the Emu68 version string (full)
	GITHASH/S  : Print the Emu68 git-hash string
	VARIANT/S  : Print the Emu68 variant string
```

## **EXAMPLE SCRIPT**

```
	FAILAT 10
	
	C:EMU68VERSION 1 0 6 >NIL:
	
	IF NOT WARN
	  ECHO "GREATER OR EQUAL THAN 1.0.6"
	ELSE
	  ECHO "LOWER THAN 1.0.6"
	ENDIF
```

.
