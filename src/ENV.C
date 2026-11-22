/*
 * env.c
 * Cr‚ation d'un nouvel environnement pour un programme lanc‚.
 *
 * PutEnv construit le bloc d'environnement, FreeEnv le libŠre.
 *
 * Utilise : GLOBALS.H
 * Utilis‚ par : EXEC.C
 *
 * Cr‚ation : 06/07/1997
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<tos.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<upoint\memory.h>
#include	<upoint\struct.h>

#include	"globals.h"	/* juste pour _LOG_ */
#include	"xprintf.h"

/* ---       Includes internes       -- */

#include	"env.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL		memdb	env= _MEMDB;	/* bloc environnement */
EXTERN	BASPAG *_BasPag;
LOCAL		UBYTE *p;
UBYTE		Err_Env_Full[]="! WARNING env size if FULL\n";

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

UBYTE	*NewEnv(LONG size)
{
LONG env_size=0L;
memdb	memory= _MEMDB;

	LOG_printf("> NewEnv %ld bytes\n",size);

	p = _BasPag->p_env;
	if ( p != NULL )
	{	/* Compter */
		for ( env_size = 0L ; !( (p[env_size] == '\0') && ( p[env_size+1L] == '\0') ) ; env_size++ );
		if( NewMemBlk(&memory,env_size+size,sizeof(UBYTE)) > 0)
		{
			env = memory;
			memcpy(env.beg,p,env_size);
			p = (UBYTE*)env.beg + env_size + 1L;
			env.used = env_size;
			return((UBYTE*)env.beg);
		}
	}

return((UBYTE*)NULL);
}

BOOLEAN PutEnv(UBYTE *buffer)
{
INT len = (INT)strlen(buffer) + 1;

	LOG_printf("> PutEnv %s, free = %ld bytes\n", buffer, env.len - env.used);

	if ( (len + 2) < env.len - env.used )
	{
		memcpy(p,buffer,len);
		p = p + len ;
		memcpy(p,"\0\0",2L);	/* Terminal */
		env.used = (LONG)p - (LONG)env.beg ;
	}
	else
	{
		LOG_printf(Err_Env_Full);
		xprintf(Err_Env_Full);
		return(FALSE);
	}

return(TRUE);
}

BOOLEAN FreeEnv(VOID)
{

	LOG_printf("> FreeEnv\n");
	FreeMemBlk(&env);

return(TRUE);
}


/* ---          EOF                 --- */
