/*
 * stik.c
 * Interface TCP/IP pour STiK et STinG.
 *
 * Fonctions STIK_* appel‚es par TCP.C : ouverture, envoi,
 * attente d'‚tat et fermeture d'une connexion, r‚solution de noms.
 *
 * Cr‚ation : 09/04/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<sting\sting.h>

#include "..\rsc.h"

/* ---       Includes internes       -- */

#include "stik.h"

/* --- Constantes globales internes --- */

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL UBYTE stik_string[]="STiK";
EXTERN UBYTE *tcp_version;	/* Pas tres adroit, dans tcp.c ! */
EXTERN UBYTE *transport;

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

BOOLEAN STIK_init(VOID)
{
INT ret;

	LOG_printf("> STIK_init()\n");

	ret = init_network();
	if ( (ret == NETWORK_OK) || (ret == NETWORK_STIK_OK) )
	{
		tcp_version = tpl->version;
		transport = stik_string;
		return(TRUE);
	}
	
return(FALSE);
}

int16 STIK_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize)
{
	
	LOG_printf("> STIK_TCP_open <%s> port <%d> tos <%d> buffer <%u>\n", inetoa(rhost), rport, tos, obsize);

return( TCP_open( rhost,  rport, tos, obsize) );
}

int16 STIK_TCP_wait_state(int16 cn, int16 state, int16 timeout )
{
	LOG_printf("> STIK_TCP_wait_state\n");
	
return( TCP_wait_state( cn,  state, timeout) );
}

int16 STIK_TCP_send(int16 cn, char *buf, int16 len)
{

	LOG_printf("> STIK_TCP_send handle <%d> size <%d>\n", cn, len);

return( TCP_send( cn,  buf, len) );
}

int16 STIK_TCP_close(int16 cn, int16 timeout)
{
	LOG_printf("> STIK_TCP_close\n");

return( TCP_close( cn,  timeout, (int16 *)NULL) );
}

char  *STIK_get_err_text(int16 code)
{
	LOG_printf("> STIK_get_err_text <%d>\n", code);
	
return( STinGRessource(code) );
}


int16 STIK_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize)
{
	LOG_printf("> STIK_resolve\n");

return( resolve(dn,rdn,alist,lsize) );
}

int16 STIK_CNbyte_count(int16 cn)
{
	return( CNbyte_count(cn) );
}

int16 STIK_CNget_block(int16 cn, char *blk, int16 len)
{
NDB  *ndb;
int16 readed;

	ndb = CNget_NDB (cn);
	if (ndb != NULL )
	{
		readed = min ( ndb->len, len );
		memcpy( blk, ndb->ndata, readed );
		KRfree (ndb->ptr);
		KRfree (ndb);
		return( readed );
	}

/* No data available yet, or an error occured ! */

return( CNget_block( cn, blk, len) );


#ifdef notdef
NDB  *ndb;
int16 readed;
int16	total=0;

	LOG_printf("> STIK_CNget_block\n");

	do
	{

		ndb = CNget_NDB (cn);
	  if (ndb != NULL )
	  {
  		readed = min ( ndb->len, len );
			memcpy( blk, ndb->ndata, readed );
			blk = blk + readed;
			total = total + readed;
	  	KRfree (ndb->ptr);
  	  KRfree (ndb);
		}
	}while ( (ndb != NULL) && ( total < len )  );

  /* No data available yet, or an error occured ! */
	if ( ( total == 0 ) && ( ndb == NULL ) )
		return( CNget_block( cn, blk, len) );
		
return ( total );

#endif

}

char *STIK_getvstr(char *variable )
{
	LOG_printf("> STIK_getvstr <%s>\n", variable);

return(  getvstr(variable) );
}

/* ---          EOF                 --- */
