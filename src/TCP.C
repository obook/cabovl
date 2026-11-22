/*
 * tcp.c
 * Interface TCP/IP unique pour les diff‚rentes piles.
 *
 * Les fonctions X_TCP_open, X_TCP_send, X_TCP_close, X_resolve,
 * X_CNget_block, etc. masquent la pile TCP/IP utilis‚e (STiK,
 * STinG, IConnect, MiNT) au reste du module. init_tcp d‚tecte la
 * pile disponible.
 *
 * Utilise : GLOBALS.H, RSC.H, TOOLS.H
 * Utilis‚ par : CACHE_IP.C, GET_INFO.C, HEADER.C, HTTP_GET.C, MAILTO.C,
 *   MAIN.C, MY_HEAD.C, TOOLS.C
 *
 * Cr‚ation : 16/07/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

#define __STIK__
#define __ICONNECT__
/* #define __DRACONIS__ */
/* #define __MINTNET__ */
#define __OTHER_THAN_STING__
#define	X_LOG_printf	trash	/* pas de message */



/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<yield\yield.h>

#ifdef __DRACONIS__
	#include	<draconis\stsocket.h>
#endif

#include	<sting\sting.h>
#include	"rsc.h"

#ifdef __STIK__
	#include "tcp\stik.h"
#endif

#ifdef __ICONNECT__
	#include "tcp\iconnect.h"
#endif

#ifdef __MINTNET__
	#include "tcp\mintnet.h"
#endif

#include	"globals.h"
#include	"tools.h"

/* ---       Includes internes       -- */

#include "tcp.h"	

/* --- Constantes globales internes --- */

#define NS_INADDRSZ	4	/* IPv4 T_A */
#define NS_IN6ADDRSZ	16	/* IPv6 T_AAAA */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

UBYTE *tcp_version = "";
UBYTE *transport = "";

#ifdef __DRACONIS__
	BOOLEAN draconis = FALSE;
	LOCAL UBYTE draconis_string[]="Draconis";
#endif

#ifdef __OTHER_THAN_STING__
/*LOCAL	sockaddr_in hisctladdr;*/
#endif


/* Les drapeaux : */

BOOLEAN sting = FALSE;
BOOLEAN iconnect = FALSE;

/* ---     Prototypage interne      --- */

LOCAL VOID trash( void *fmt , ... );

/* ---         Fonctions            --- */

BOOLEAN init_tcp( VOID )
{
/* INT ret; */
	
	LOG_printf("> init_tcp\n");

	/* MinNet */

#ifdef __MINTNET__

	if ( MINTNET_init() == TRUE )
	{
		mintnet = TRUE;
		return(TRUE);
	}

#endif

	/* STinG */

#ifdef __STIK__

	if( STIK_init() == TRUE )
	{
		sting = TRUE;
		return(TRUE);
	}

#endif

	/* ICONNECT */
	
#ifdef __ICONNECT__

	if ( ICONNECT_init() == TRUE )
	{
		iconnect = TRUE;
		return(TRUE);
	}

#endif
	
	/* DRACONIS */

#ifdef __DRACONIS__

	ret = get_connected();
	ret = -1;
	
	if ( ret != DRACONIS_ABSENT )
	{
		draconis = TRUE;
		transport = draconis_string;
/*	if ( ret == DRACONIS_CONNECTE ) */
		return(TRUE);
	}
	else


#endif

return(FALSE);
}

int16 X_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize)
{
#ifdef __DRACONIS__
	sockaddr_in 	sin;
	INT ret;
	INT handle;
	LONG lret;
#endif

	X_LOG_printf("> X_TCP_open\n");
	
#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return(-1);
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
	 return( ICONNECT_TCP_open(rhost,  rport, tos, obsize) );
	}

#endif

#ifdef __STIK__

	if ( sting == TRUE )
	{
		return( STIK_TCP_open( rhost,  rport, tos, obsize) );
	}

#endif


return(-1);
}

int16 X_TCP_wait_state(int16 cn, int16 state, int16 timeout )
{
	X_LOG_printf("> X_TCP_wait_state\n");
	
#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return(-1);
	}
	
#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_TCP_wait_state(cn,  state, timeout ) );
	}
	
#endif

#ifdef __STIK__

	if( sting == TRUE )
	{
		return( STIK_TCP_wait_state( cn,  state, timeout) );
	}

#endif

return(-1);
}

int16 X_TCP_send(int16 cn, char *buf, int16 len)
{
/* INT ret; */

	X_LOG_printf("> X_TCP_send handle <%d> size <%d>\n", cn, len);

#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return(-1);
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_TCP_send(cn,  buf, len) );
	}

#endif

#ifdef __STIK__

	if ( sting == TRUE )
	{
		return( STIK_TCP_send( cn,  buf, len) );
	}

#endif
	
return( E_BADHANDLE );
}

int16 X_TCP_close(int16 cn, int16 timeout)
{
	X_LOG_printf("> X_TCP_close\n");

#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return(-1);
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_TCP_close(cn,  timeout) );
	}

#endif

#ifdef __STIK__

	if( sting == TRUE )
	{

		return( STIK_TCP_close( cn,  timeout) );
	}
#endif

return(-1);
}

char  *X_get_err_text(int16 code)
{
/*X_LOG_printf("> X_get_err_text <%d>\n", code);*/

#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return("Draconis down");
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
/*	return( ICONNECT_get_err_text(code) ); */
		return( STinGRessource(code) );
	}

#endif

	/* STinG */
	
#ifdef __STIK__

	if( sting == TRUE )
	{

		return( STinGRessource(code) );
	}

#endif

return("??");
}

/*
  - Resolve a domain name into one or more IP addresses.
    dn is the domain name.
    rdn is the real domain name, which is returned if dn is an alias (CNAME).
    alist is a pointer to an array where the IP address(s) are returned.
    lsize is the size of that array.
  - If the information is in local cache, then resolve will return it
    immediately, otherwise an algorithm for query of nameservers
    is initiated.  This can take some time, however, the resolver has
    an internal limit on the amout of work it will do in attempting
    to resove a hostname.  (Hence the error code `work limit reached')
  - If rdn == (char **)NULL then no value is assigned to it.
    Otherwise, a pointer to the domain name associated with the addresses
    is assigned to *rdn, even if it is the same as that requested.
    This pointer must be freed using KRfree().
  - resolve() returns the number of addresses copied to alist,
    or a negative error code.
  - In retrospect, it might not have been all that smart of me to write
    my own DNS resolver, but it is at least educational :-))
    All the same, it is *not* yet finished.  There are some bugs in
    the code at present, and also inefficiency in my algorithm.
  - I'll work on improving it when (if?) I get the chance.
  - NOTE that STiK.ACC will save the dns cache to domain.txt
    every five minutes if AES is active and STiK is enabled.
*/

int16 X_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize)
{
#ifdef __OTHER_THAN_STING__
/*static hostent *host;*/
#endif

#ifdef __DRACONIS__
	struct in_addr **hptr;
	static char **addr_ptr;
#endif

	X_LOG_printf("> X_resolve\n");

#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
/*
		bzero((char *)&hisctladdr, sizeof (hisctladdr));
		hisctladdr.sin_addr.s_addr = inet_addr(dn);
*/
		host = gethostbyname( dn );
		if ( host != NULL )
		{
		switch( host->h_addrtype )
		{
			case AF_INET :
					for (hptr = (struct in_addr **)host->h_addr_list; *hptr != NULL; hptr++)
					{
				 		memcpy( &addr_ptr,*hptr, NS_INADDRSZ);
						X_LOG_printf("! TEST = <0x%lx>\n", *hptr);
					}
/*
					if (numnsaddr < NUMNSADDR)
					 {
					 		memcpy( &nsipaddr[numnsaddr],*hptr, NS_INADDRSZ);
					 		numnsaddr++;
					 		numaddrs++;
			 			}
*/

				memcpy( &addr_ptr, host->h_addr, NS_INADDRSZ);
				addr_ptr = host->h_addr_list;
				X_LOG_printf("~ addr_ptr = <0x%lx>\n", addr_ptr);
				X_LOG_printf("~ addr_ptr = <0x%lx>\n", addr_ptr[0]);
				X_LOG_printf("~ addr_ptr = <0x%lx>\n", *addr_ptr);
				X_LOG_printf("~ addr_ptr = <0x%lx>\n", *addr_ptr[0]);
				if ( rdn != NULL )		*rdn = host->h_name;
			 	if ( alist != NULL )	*alist = (uint32)( *addr_ptr[0] );
/*
				hisctladdr.sin_family = hp->h_addrtype;
				bcopy(hp->h_addr_list[0],
					(caddr_t)&hisctladdr.sin_addr, hp->h_length);
*/
				return(0);

/*
			case AF_INET6;
			break;
*/

			default :
				return(-1);
		}
	 }
	 X_LOG_printf("~ gethostbyname NULL\n");
/*
		x = X_resolve(server,(char **)NULL, &addr, 1);
		      char  *h_name;
          char  **h_aliases;
          int   h_addrtype;
          int   h_length;
          char  **h_addr_list;
*/
	 return(-1);
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_resolve(dn,rdn,alist,lsize) );
	}

#endif

#ifdef __STIK__

	if ( sting == TRUE )
	{
		return( STIK_resolve(dn,rdn,alist,lsize) );
	}

#endif

return(-1);
}

int16 X_CNbyte_count(int16 cn)
{
/*X_LOG_printf("> X_CNbyte_count\n"); */
/* INT ret; */

#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return(-1);
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_CNbyte_count(cn) );
	}

#endif

#ifdef __STIK__


	if ( sting == TRUE )
	{
		return( STIK_CNbyte_count(cn) );
	}

#endif

return(-1);
}

int16 X_CNget_block(int16 cn, char *blk, int16 len)
{
/* INT ret; */

		X_LOG_printf("> X_CNget_block\n");

#ifdef __DRACONIS__

	if ( draconis == TRUE )
	{
		return(-1);
	}

#endif

#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_CNget_block( cn, blk, len) );
	}

#endif

#ifdef __STIK__

	if( sting == TRUE )
	{
		return( STIK_CNget_block( cn, blk, len) );
	}
#endif

return(0);
}

char *X_getvstr(char *variable )
{
#ifdef __ICONNECT__

	if ( iconnect == TRUE )
	{
		return( ICONNECT_getvstr(variable) );
	}

#endif

#ifdef __STIK__

	if( sting == TRUE )
	{
		return( STIK_getvstr(variable) );
	}

#endif

return("0");
}

/* ----- Outils ----- */

LOCAL VOID trash( void *fmt , ... )
{

if ( fmt )	return;

}

/* ---          EOF                 --- */
