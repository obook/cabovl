/*
 * iconnect.c
 * Interface TCP/IP pour IConnect (PPP-Connect).
 *
 * Appelle la bibliothŠque de sockets d'IConnect pour TCP.C.
 *
 * Utilise : GLOBALS.H
 *
 * Cr‚ation : 02/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<iconnect\sockinit.h>
#include	<iconnect\socket.h>
#include	<iconnect\netdb.h>
#include	<iconnect\inet.h>
#include	<iconnect\sockerr.h>
#include	<iconnect\sockios.h>
#include	<iconnect\sfcntl.h>
#include	<iconnect\usis.h>

#include	"globals.h"
#include	"..\globals.h"
#include	"..\tools.h"

/* ---       Includes internes       -- */

typedef          char  int8;          /*   Signed  8 bit (char)             */
typedef unsigned char uint8;          /* Unsigned  8 bit (byte, octet)      */
typedef          int   int16;         /*   Signed 16 bit (int)              */
typedef unsigned int  uint16;         /* Unsigned 16 bit (word)             */
typedef          long  int32;         /*   Signed 32 bit                    */
typedef unsigned long uint32;         /* Unsigned 32 bit (longword)         */

#include "iconnect.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL UBYTE iconnect_string[]="Iconnect";
LOCAL	sockaddr_in hisctladdr;
EXTERN UBYTE *tcp_version;	/* Pas tres adroit, dans tcp.c ! */
LOCAL UBYTE version[]="00.00";

LOCAL UBYTE iconnect_email[127];
LOCAL UBYTE iconnect_http_proxy_port[127];
LOCAL UBYTE iconnect_http_proxy[127];
LOCAL UBYTE iconnect_ftp_proxy_port[127];
LOCAL UBYTE iconnect_ftp_proxy[127];
LOCAL UBYTE iconnect_smtp_server[127];

/* ---     Prototypage interne      --- */

LOCAL UBYTE *ICONNECT_getvstr_err( UBYTE *variable, INT err );

/* ---         Fonctions            --- */

BOOLEAN ICONNECT_init(VOID)
{
INT ret;

	*iconnect_email = '\0';
	*iconnect_http_proxy_port = '\0';
	*iconnect_http_proxy = '\0';
	*iconnect_ftp_proxy_port = '\0';
	*iconnect_ftp_proxy = '\0';
	*iconnect_smtp_server = '\0';

	ret = sock_init();
	
	if ( ret != SE_NINSTALL )
	{
		transport = iconnect_string;
		tcp_version = version;
		return(TRUE);
	}

return(FALSE);
}

int16 ICONNECT_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize)
{
sockaddr_in 	sin;
INT ret;
INT handle;
LONG lret;

	trash( (VOID*)obsize, (VOID*)tos );
	
	if ( iconnect == TRUE )
	{
		handle = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
		if ( handle < 0 )
			return( -1 );	/* mettre une erreure */
		/* non bloquant */
		lret = sfcntl( handle, F_GETFL, 0);
		lret = sfcntl( handle, F_SETFL, lret|O_NONBLOCK);
		memset((void *)&sin, 0, sizeof(sin));
		sin.sin_family = IPPROTO_TCP;
		sin.sin_port=htons(rport);
		memcpy(&sin.sin_addr,&rhost, sizeof(rhost) );
/*		char					sin_zero[8]; */
		ret = connect( handle, &sin, (INT)sizeof(sin) );
		if ( ( ret < 0 ) && (ret != EINPROGRESS) && (ret != EALREADY) )
			return( -1 );	/* mettre une erreure */
	 return( handle );
	}

return(-9);	/*E_BADHANDLE*/
}

int16 ICONNECT_TCP_wait_state(int16 cn, int16 state, int16 timeout )
{
	trash( (VOID*)cn, (VOID*)state, (VOID*)timeout);

return(0);
}

int16 ICONNECT_TCP_send(int16 cn, char *buf, int16 len)
{
INT ret;

	if ( iconnect == TRUE )
	{
		do
		{
			ret = swrite( cn, (void *)buf, len );
/*		_appl_yield(); */
		}while ( ret == EWOULDBLOCK );
/*	LOG_printf("! DEBUG : swrite return <%d>\n", ret ); */
		if ( ret == len )	return( 0 );	/*E_NORMAL*/
		return( -9 );	/*E_BADHANDLE*/
	}

return( -9 );
}

int16 ICONNECT_TCP_close(int16 cn, int16 timeout)
{
	trash( (VOID*)timeout );
	
	if ( iconnect == TRUE )
	{
		return( sclose( cn ) );
	}
return(-9); /* E_BADHANDLE */
}

char  *ICONNECT_get_err_text(int16 code)
{
	if ( iconnect == TRUE )
	{
		if ( code >= 0 )
			return("No error");
		else	if ( code == EPROTONOSUPPORT )
			return( "Protocol not supported");
		else	if ( code == ENOTCONN )
			return("Socket is not connected");
		else if ( code == ECONNRESET )
			return("Connection reset by peer");
	
	}

return( "?? unkown iconnect error ??");
}

int16 ICONNECT_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize)
{
static hostent *host;

	if ( iconnect == TRUE )
	{
	INT i;
	unsigned long *ip;

		host = gethostbyname(dn);
	
		if ( host == (hostent*)NULL )
		{
			return(-17);	/*E_CANTRESOLVE*/
		}
		
		trash( (void*)rdn );
		trash( (void*)lsize );
	
		i = 0;
		while ( host->h_aliases[i] != '\0' )
		{
			i++;
		}

		i = 0;
		while ( host->h_addr_list[i] != '\0' )
		{
			ip = (unsigned long*)host->h_addr_list[i];
			*alist = *ip;
			i++;
		}

		return( 0 );	/* E_NORMAL */
	}

return(-9); /* E_BADHANDLE */
}

int16 ICONNECT_CNbyte_count(int16 cn)
{
	if ( iconnect == TRUE )
	{
		fd_set	rset;
		long test;
		timeval	timeout;

		FD_ZERO( &rset );
		FD_SET( cn, &rset);
	
		timeout.tv_sec = 3;		/* seconds */
		timeout.tv_usec = 0;	/* microseconds (NOT milliseconds!!) */

		test=(long)select( cn+1, &rset, NULL, NULL, &timeout );
		
		if ( test<0L )	/* Bloquant ?  on essaye timeout */
		{
			LOG_printf("! select error <%ld>\n", test);
			test = -3 ;	/* E_EOF */
		}
		else
		{
	
			test = FD_ISSET( cn, &rset);

			LOG_printf("! FD_ISSET = <%ld>\n", test);

			if ( test < 0L )
			{
				test = -3;	/*E_EOF*/
			}
	
			if ( test > 0L )	test = TCP_BUFFER;
		}
	
		return( (int16)test );
	}

return(-9); /* E_BADHANDLE */
}

int16 ICONNECT_CNget_block(int16 cn, char *blk, int16 len)
{
INT ret;

	if ( iconnect == TRUE )
	{
/*	LOG_printf("! DEBUG X_CNget_block max <%d>: sread\n", len); */
		ret = (int16)sread( cn, (VOID *)blk, len);
		LOG_printf("! DEBUG X_CNget_block: sread return <%d>\n", ret );
		if ( ret == ECONNRESET )	ret = -3;	/* E_EOF */
		if ( ret == EWOULDBLOCK )	ret = 0;
		return( ret );
	}

return(-9); /* E_BADHANDLE */
}

LOCAL UBYTE *ICONNECT_getvstr_err( UBYTE *variable, INT err )
{
UBYTE *mess;

	switch ( err )
	{
		case UA_FOUND :
			mess = "No error";
		break;
		case UA_NOTSET :
			mess = "Not Set";
		break;
		case UA_OFFLINE :
			mess = "You are offline";
		break;
		default :
			mess = "Unknown";
		break;
	}
	
	LOG_printf("! Variable <%s> : <%s>\n", variable, mess);

return("0");
}

char *ICONNECT_getvstr(char *variable )
{
USIS_REQUEST ur;
INT ret;

	LOG_printf("> ICONNECT_getvstr <%s>\n", variable);

	if ( strnicmp( variable, "EMAIL", 5 ) == SUCCESS )
	{
		ur.request = UR_EMAIL_ADDR;	/* TOP SECRET ! */
		ur.result = "";
		ur.free1 = "";
		ur.free2 = "";
		ur.other = (void*)"";
		ret = usis_query( &ur );
		if ( ret == UA_FOUND )
		{
			*iconnect_email = '\0';
			strncat( iconnect_email, ur.result, 126);
			LOG_printf("~ EMAIL <%s>\n", iconnect_email );
			return(iconnect_email);
		}
		else
			return( ICONNECT_getvstr_err( "EMAIL", ret) );
	}

	else
	if ( strnicmp( variable, "HTTP_PROXY_PORT", 15 ) == SUCCESS )
	{
		ur.request = UR_PROXY_IP;
		ur.result = "";
		ur.free1 = "http";
		ur.free2 = "";
		ur.other = (void*)"";
		ret = usis_query( &ur );
		if ( ret == UA_FOUND )
		{
			*iconnect_http_proxy_port = '\0';
			itoa(ur.port, iconnect_http_proxy_port, 10);
			LOG_printf("~ UR_PROXY_IP http port <%s>\n", iconnect_http_proxy_port );
			return(iconnect_http_proxy_port);
		}
		else
			return( ICONNECT_getvstr_err( "HTTP_PROXY_PORT", ret) );
	}

	else
	if ( strnicmp( variable, "HTTP_PROXY", 10 ) == SUCCESS )
	{
		ur.request = UR_PROXY_IP;
		ur.result = "";
		ur.free1 = "http";
		ur.free2 = "";
		ur.other = (void*)"";
		ret = usis_query( &ur );
		if ( ret == UA_FOUND )
		{
			*iconnect_http_proxy = '\0';
			strncat( iconnect_http_proxy, inet_ntoa( ur.ip ), 126);
			LOG_printf("~ UR_PROXY_IP http <%s>\n", iconnect_http_proxy );
			return(iconnect_http_proxy);
		}
		else
			return( ICONNECT_getvstr_err( "HTTP_PROXY", ret) );
	}

	else
	if ( strnicmp( "FTP_PROXY_PORT", variable, 14 ) == SUCCESS )
	{
		ur.request = UR_PROXY_IP;
		ur.result = "";
		ur.free1 = "ftp";
		ur.free2 = "";
		ur.other = (void*)"";
		ret = usis_query( &ur );
		if ( ret == UA_FOUND )
		{
			*iconnect_ftp_proxy_port = '\0';
			itoa(ur.port, iconnect_ftp_proxy_port, 10);
			LOG_printf("~ UR_PROXY_IP ftp port <%s>\n", iconnect_ftp_proxy_port );
			return(iconnect_ftp_proxy_port);
		}
		else
			return( ICONNECT_getvstr_err( "FTP_PROXY_PORT", ret) );
	}

	else
	if ( strnicmp( "FTP_PROXY", variable, 9 ) == SUCCESS )
	{
		ur.request = UR_PROXY_IP;
		ur.result = "";
		ur.free1 = "ftp";
		ur.free2 = "";
		ur.other = (void*)"";
		ret = usis_query( &ur );
		if ( ret == UA_FOUND )
		{
			*iconnect_ftp_proxy = '\0';
			strncat( iconnect_ftp_proxy, inet_ntoa( ur.ip ), 126);
			LOG_printf("~ UR_PROXY_IP ftp <%s>\n", iconnect_ftp_proxy );
			return(iconnect_ftp_proxy);
		}
		else
			return( ICONNECT_getvstr_err( "FTP_PROXY", ret) );
	}

	else
	if ( strnicmp( "SMTP_SERVER", variable, 11 ) == SUCCESS )
	{
		ur.request = UR_SMTP_IP;
		ur.result = "";
		ur.free1 = "";
		ur.free2 = "";
		ur.other = (void*)"";
		ret = usis_query( &ur );
		if ( ret == UA_FOUND )
		{
			*iconnect_smtp_server = '\0';
			strncat( iconnect_smtp_server, inet_ntoa( ur.ip ), 126);
			LOG_printf("~ SMTP_SERVER <%s>\n", iconnect_smtp_server );
			return(iconnect_smtp_server);
		}
		else
			return( ICONNECT_getvstr_err( "SMTP_SERVER", ret) );
	}

return( ICONNECT_getvstr_err( variable, UA_UNKNOWN) );
}

/* ---          EOF                 --- */
