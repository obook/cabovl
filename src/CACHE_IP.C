/*
 * cache_ip.c
 * Cache du r‚solveur de noms (my_resolver).
 *
 * Se place devant la r‚solution de noms de TCP.C pour ne pas
 * r‚soudre deux fois le mˆme serveur de suite.
 *
 * Utilise : GLOBALS.H, MAIN.H, TCP.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 13/10/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<tos.h>
#include	<portab.h>
#include	<cab\module.h>
#include	<sting\transprt.h>		/* Pour  TESTABLISH */
#include	<logfile.h>

#include	"globals.h"
#include	"main.h"
#include	"tcp.h"

/* ---       Includes internes       -- */

#include	"cache_ip.h"

/* --- Constantes globales internes --- */

#define	CACHE_SIZE	50		/* 50 m‚moires cache */
#define CACHE_NAME	255		/* nom des hosts max 255 car */

/* --- Structures globales internes --- */

typedef struct Host
{
	UBYTE	name[CACHE_NAME+1];
	uint32 addr;
}HOST;

/* --- Variables globales  internes --- */

LOCAL HOST		hosts[CACHE_SIZE-1];
LOCAL	WORD		index = 0;
LOCAL BOOLEAN start = TRUE;

/* ---     Prototypage interne      --- */

LOCAL VOID Init_Hosts( VOID );

/* ---         Fonctions            --- */

int16 my_resolver(UBYTE *server, uint32 *addr)
{
int16 ret;
WORD	i;

	LOG_printf("> my_resolver <%s>\n", server);
	
	if ( start == TRUE	) Init_Hosts();
	
	*addr = 0L;

	for ( i = 0; i < (CACHE_SIZE-1) ; i++ )
	{
		if ( stricmp( hosts[i].name, server ) == SUCCESS )
		{
			*addr = hosts[i].addr;
/*		LOG_printf("~ Find <%s> in DNS cache <%d>\n", server, i); */
			return(0);
		}
	}

	browser->msg_status(STATUS_RESOLVING_HOST,0);
	ret = X_resolve(server,(char **)NULL, addr, 1);
	if (ret>0)
	{
		*hosts[index].name = '\0';
		strncat( hosts[index].name, server, CACHE_NAME);
		hosts[index].addr = *addr;
/*	LOG_printf("~ Put <%s> in DNS cache <%d>\n", server, index); */
		index++;
		if ( index > (CACHE_SIZE-1) ) index = 0;
	}

return(ret);
}

VOID Clear_Last_Host( VOID )
{
	index--;
	if ( index < 0 )
		index = (CACHE_SIZE-1);
/*LOG_printf("~ Clear <%s> from DNS cache <%d>\n", hosts[index].name, index); */
	*hosts[index].name = '\0';
}

LOCAL VOID Init_Hosts( VOID )
{
WORD	i;

/*LOG_printf("> Init_Hosts DNS cache\n");*/

	for ( i = 0; i < (CACHE_SIZE-1) ; i++ )
	{
		*hosts[i].name = '\0';
		hosts[i].addr = 0L;
	}

	start = FALSE;
}

/* ---          EOF                 --- */
