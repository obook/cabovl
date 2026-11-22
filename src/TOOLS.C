/*
 * tools.c
 * Fonctions utilitaires du module.
 *
 * Lecture des dates HTTP (parse_date), d‚coupage d'URL
 * (split_URL), lecture des r‚glages de proxy, fonctions de
 * chaŒnes (skipwhite, clearwhite), ouverture et fermeture de
 * fichiers avec trace, texte de version de CAB.
 *
 * Utilise : CONFIG.H, COOKIE.H, GLOBALS.H, HTTP.H, MAIN.H, RSC.H, TCP.H
 * Utilis‚ par : COOKIE.C, EXEC.C, HEADER.C, HTTP_GET.C, LANGUAGE.C,
 *   MAILTO.C, MAIN.C, MY_HEAD.C, REPORT.C, SETUP.C, TCP.C
 *
 * Cr‚ation : 10/07/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<tos.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>
#include	<upoint\memory.h>
#include	<upoint\struct.h>

#define _TOOLS_INTERNAL_
#include	"globals.h"
#include	"config.h"
#include	"main.h"
#include	"tcp.h"
#include	"http.h"
#include	"rsc.h"
#include	"cookie.h"

/* ---       Includes internes       -- */

#ifdef MDEBUG
#include	<logfile.h>
#endif

#include "tools.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

/************************************************************************
	This is where my private routines start
************************************************************************/

time_t parse_date(char *date_string)
{
size_t index;
char tmp[5]={0, 0, 0, 0, 0};
struct tm time;

	LOG_printf("> parse_date\n");
	
	if (strchr(date_string, '-')!=NULL) {
		/* Sunday, 06-Nov-94 08:49:37 GMT	RFC 850, obsoleted by RFC 1036 */
		index=strcspn(date_string, ",");
		time.tm_sec=atoi(strncpy(tmp, &date_string[index+18], 2));
		time.tm_min=atoi(strncpy(tmp, &date_string[index+15], 2));
		time.tm_hour=atoi(strncpy(tmp, &date_string[index+12], 2));
		time.tm_year=atoi(strncpy(tmp, &date_string[index+9], 2));
		time.tm_mday=atoi(strncpy(tmp, &date_string[index+2], 2));
	} else if (strchr(date_string, ',')!=NULL) {
		/* Sun, 06 Nov 1994 08:49:37 GMT		RFC 822, updated by RFC 1123 */
		time.tm_sec=atoi(strncpy(tmp, &date_string[23], 2));
		time.tm_min=atoi(strncpy(tmp, &date_string[20], 2));
		time.tm_hour=atoi(strncpy(tmp, &date_string[17], 2));
		time.tm_mday=atoi(strncpy(tmp, &date_string[5], 2));
		time.tm_year=atoi(strncpy(tmp, &date_string[12], 4))-1900;
	} else {
		/* Sun Nov	6 08:49:37 1994			ANSI C's asctime() format */
		time.tm_sec=atoi(strncpy(tmp, &date_string[17], 2));
		time.tm_min=atoi(strncpy(tmp, &date_string[14], 2));
		time.tm_hour=atoi(strncpy(tmp, &date_string[11], 2));
		time.tm_mday=atoi(strncpy(tmp, &date_string[8], 2));
		time.tm_year=atoi(strncpy(tmp, &date_string[20], 4))-1900;
	}
	/* The month is generic... */
	if (strstr(date_string, "Jan")>0)
		time.tm_mon=0;
	else if (strstr(date_string, "Feb")>0)
		time.tm_mon=1;
	else if (strstr(date_string, "Mar")>0)
		time.tm_mon=2;
	else if (strstr(date_string, "Apr")>0)
		time.tm_mon=3;
	else if (strstr(date_string, "May")>0)
		time.tm_mon=4;
	else if (strstr(date_string, "Jun")>0)
		time.tm_mon=5;
	else if (strstr(date_string, "Jul")>0)
		time.tm_mon=6;
	else if (strstr(date_string, "Aug")>0)
		time.tm_mon=7;
	else if (strstr(date_string, "Sep")>0)
		time.tm_mon=8;
	else if (strstr(date_string, "Oct")>0)
		time.tm_mon=9;
	else if (strstr(date_string, "Nov")>0)
		time.tm_mon=10;
	else
		time.tm_mon=11;
	/* DOSTIME is in 2 second intervals... */

/*time.tm_sec=(time.tm_sec & 0xFFFE); NON car fix‚ … la fermeture du fichier */

/*	LOG_printf("~ %ld\n", (long) mktime(&time));*/
return(mktime(&time));
}

/* Retourne le time_t dans un ans, pour les cookies */

UBYTE *dans_un_ans( VOID )
{
char tmp[5]={0, 0, 0, 0, 0};
size_t index;
struct tm t;
ULONG ul_today;
UBYTE date_string[50];

	time(&( (time_t)(ul_today) ));
	Strftime(date_string,50,"%a, %d %b %Y %X GMT",localtime(&((time_t)(ul_today))));

	LOG_printf("> dans_un_ans de <%s>\n", date_string);

	if (strchr(date_string, '-')!=NULL) {
		/* Sunday, 06-Nov-94 08:49:37 GMT	RFC 850, obsoleted by RFC 1036 */
		index=strcspn(date_string, ",");
		t.tm_sec=atoi(strncpy(tmp, &date_string[index+18], 2));
		t.tm_min=atoi(strncpy(tmp, &date_string[index+15], 2));
		t.tm_hour=atoi(strncpy(tmp, &date_string[index+12], 2));
		t.tm_year=atoi(strncpy(tmp, &date_string[index+9], 2)) + 1;  /* Dans un ans */
		t.tm_mday=atoi(strncpy(tmp, &date_string[index+2], 2));
	} else if (strchr(date_string, ',')!=NULL) {
		/* Sun, 06 Nov 1994 08:49:37 GMT		RFC 822, updated by RFC 1123 */
		t.tm_sec=atoi(strncpy(tmp, &date_string[23], 2));
		t.tm_min=atoi(strncpy(tmp, &date_string[20], 2));
		t.tm_hour=atoi(strncpy(tmp, &date_string[17], 2));
		t.tm_mday=atoi(strncpy(tmp, &date_string[5], 2));
		t.tm_year=atoi(strncpy(tmp, &date_string[12], 4))-1900 +1; /* Dans un ans */
	} else {
		/* Sun Nov	6 08:49:37 1994			ANSI C's asctime() format */
		t.tm_sec=atoi(strncpy(tmp, &date_string[17], 2));
		t.tm_min=atoi(strncpy(tmp, &date_string[14], 2));
		t.tm_hour=atoi(strncpy(tmp, &date_string[11], 2));
		t.tm_mday=atoi(strncpy(tmp, &date_string[8], 2));
		t.tm_year=atoi(strncpy(tmp, &date_string[20], 4))-1900 +1; /* Dans un ans */
	}

	/* The month is generic... */
	if (strstr(date_string, "Jan")>0)
		t.tm_mon=0;
	else if (strstr(date_string, "Feb")>0)
		t.tm_mon=1;
	else if (strstr(date_string, "Mar")>0)
		t.tm_mon=2;
	else if (strstr(date_string, "Apr")>0)
		t.tm_mon=3;
	else if (strstr(date_string, "May")>0)
		t.tm_mon=4;
	else if (strstr(date_string, "Jun")>0)
		t.tm_mon=5;
	else if (strstr(date_string, "Jul")>0)
		t.tm_mon=6;
	else if (strstr(date_string, "Aug")>0)
		t.tm_mon=7;
	else if (strstr(date_string, "Sep")>0)
		t.tm_mon=8;
	else if (strstr(date_string, "Oct")>0)
		t.tm_mon=9;
	else if (strstr(date_string, "Nov")>0)
		t.tm_mon=10;
	else
		t.tm_mon=11;
	/* DOSTIME is in 2 second intervals... */

/*t.tm_sec=(t.tm_sec & 0xFFFE); NON car fix‚ la la fermeture du fichier */

	/* "Sun, 06 Nov 1994 08:49:37 GMT" => RFC 822 */
	Strftime(date_string,50,"%a, %d %b %Y %X GMT", &t);

	LOG_printf("> dans_un_ans c'est <%s>\n", date_string);

return(date_string);
}

VOID clear_info(REQUEST *request)
{

	LOG_printf("> clear_info\n");

	request->content_encoding[0]=
	request->content_type[0]=
	request->location[0]=
	request->pragma[0]=
	request->host_server[0]=
	request->www_authenticate[0]=0;

	request->mime_version=
	request->date=
	request->expires=
	request->last_modified=-1;

	request->content_length=-1L;
	*request->http_line = '\0';
}

VOID split_URL(UBYTE *URL, REQUEST *request)
{
size_t colon_index, slash_index;

	LOG_printf("> split_URL\n");

	colon_index=strcspn(URL, ":");
	strncpy(request->protocol, URL, colon_index);

	request->protocol[colon_index]=0;
	strcpy(request->URI, &URL[colon_index+3]);

	slash_index=strcspn(request->URI, "/");
	strncpy(request->server, request->URI, slash_index);

	request->server[slash_index]=0;
	strcpy(request->URI, &URL[colon_index+slash_index+3]);

	colon_index=strcspn(request->server, ":");

	/* Nouveau, test colon_index!=0 18/01/2000 */
	
	if ( (colon_index<strlen(request->server)) && (colon_index!=0) )
	{
		request->port=atoi(&(request->server[colon_index+1]));
		request->server[colon_index]=0;
	}
	else
	{
		request->port=80;
	}

	if ( request->port == 0 )	request->port = 80;

	/* correction pour http://my.server.com */
	
	if ( ( *request->URI == '\0' ) || ( *request->URI == ' ' ) )
	{
		*request->URI = '\0';
		strncat( request->URI, "/", 2047 );
	}

}

/* R‚gle la struct selon la config de STinG */

proxy_url	*fix_proxy( proxy_url	*proxy )
{
UBYTE *ptr;

	LOG_printf("> fix_proxy\n");
	
/*if ( sting != TRUE ) return(NULL); */

/*	*proxy->no_proxy = '\0'; */

	*proxy->ftp_proxy = '\0';
	ptr = X_getvstr("FTP_PROXY");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->ftp_proxy, ptr, 126);
		ptr = X_getvstr("FTP_PROXY_PORT");
		if (ptr[0] != '0' && ptr[0] != '1')
		{
			strncat( proxy->ftp_proxy, ":", 126);
			strncat( proxy->ftp_proxy, ptr, 126);
		}
	}

	*proxy->http_proxy = '\0';
	ptr = X_getvstr("HTTP_PROXY");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->http_proxy, ptr, 126);
		ptr = X_getvstr("HTTP_PROXY_PORT");
		if (ptr[0] != '0' && ptr[0] != '1')
		{
			strncat( proxy->http_proxy, ":", 126);
			strncat( proxy->http_proxy, ptr, 126);
		}
		else
		{
			strncat( proxy->http_proxy, ":", 126);
			strncat( proxy->http_proxy, "80", 126);
		}
	}

	*proxy->wais_proxy = '\0';
	ptr = X_getvstr("WAIS_PROXY");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->wais_proxy, ptr, 126);
		ptr = X_getvstr("WAIS_PROXY_PORT");
		if (ptr[0] != '0' && ptr[0] != '1')
		{
			strncat( proxy->wais_proxy, ":", 126);
			strncat( proxy->wais_proxy, ptr, 126);
		}
	}

	*proxy->gopher_proxy = '\0';
	ptr = X_getvstr("GOPHER_PROXY");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->gopher_proxy, ptr, 126);
		ptr = X_getvstr("GOPHER_PROXY_PORT");
		if (ptr[0] != '0' && ptr[0] != '1')
		{
			strncat( proxy->gopher_proxy, ":", 126);
			strncat( proxy->gopher_proxy, ptr, 126);
		}
	}

	*proxy->news_proxy = '\0';
	ptr = X_getvstr("NEWS_PROXY");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->news_proxy, ptr, 126);
		ptr = X_getvstr("NEWS_PROXY_PORT");
		if (ptr[0] != '0' && ptr[0] != '1')
		{
			strncat( proxy->news_proxy, ":", 126);
			strncat( proxy->news_proxy, ptr, 126);
		}
	}

	*proxy->smtp_server = '\0';
	ptr = X_getvstr("SMTP_SERVER");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->smtp_server, ptr, 126);
	}

	*proxy->nntp_server = '\0';
	ptr = X_getvstr("NNTP_SERVER");
	if (ptr[0] != '0' && ptr[0] != '1')
	{
		strncat( proxy->nntp_server, ptr, 126);
	}


return( proxy );
}

BOOLEAN http_proxy_cfg(char *ps, int pslen, int *pp)
{
UBYTE *proxy;
size_t colon_index;

	LOG_printf("> http_proxy_cfg (Getting proxy)\n");

	proxy = browser->proxy->http_proxy;

	if ( *proxy != '\0' )
	{
		colon_index=strcspn(proxy, ":");

/* Nouveau test avec colon_index<strlen(proxy) 18/01/2000 */

		if ( (colon_index<strlen(proxy)) && (colon_index!=0) )
		{
			*ps = '\0';
			strncat(ps, proxy, colon_index);
			*pp = atoi(&proxy[colon_index+1]);
		}
		else
		{
			strncpy(ps, proxy, (size_t)pslen);
			*pp = 80;
		}
		return(TRUE);
	}

return (FALSE);
}

int ftp_proxy_cfg(char *ps, int pslen, int *pp)
{
UBYTE	*ptr;

	LOG_printf("> ftp_proxy_cfg (Getting proxy)\n");

/*if ( sting != TRUE ) return(FALSE);*/

	ptr = X_getvstr("FTP_PROXY");
	if (ptr[0] == '0' || ptr[0] == '1')		return (FALSE);

	strncpy(ps, ptr, (size_t)pslen);

	ptr = X_getvstr("FTP_PROXY_PORT");

		/* If there is some kind of standard proxy port, then we could
		 *use it as a default instead of return (FALSE) here...
		 */
	if (ptr[0] == '0' || ptr[0] == '1')	return (FALSE);
	*pp = atoi(ptr);

return (TRUE);
}

UBYTE *skipwhite ( const UBYTE *src )
{
char *c = (char *)src;

/*LOG_printf("> skipwhite\n");*/

	while ( *c == ' ' || *c == '\x09' ) c++;

return (c);
}

UBYTE *clearwhite ( const UBYTE *src )
{
char *c = (char *)src;
char *d;

/*LOG_printf("> clearwhite\n");*/

	while ( *c == ' ' || *c == '\x09' ) c++;

	d = c;

	while ( *d >= ' ' ) d++;

	*d = '\0';

return (c);
}

/* ----------------------------------------------------------------------*

	Fonction de Centralisation

 * ----------------------------------------------------------------------*/

long  X_File_create( const char *fname, int attr , const char *file, long line )
{
long ret;

	ret = Fcreate( fname, attr );
	
	LOG_printf("> Fcreate %s %ld <%s> = <%ld>\n", file, line, fname, ret);

return( ret );
}

long  X_File_open( const char *fname, int mode , const char *file, long line )
{
long ret;

	ret = Fopen( fname, mode );
	
	LOG_printf("> Fopen %s %ld <%s> = <%ld>\n", file, line, fname, ret);

return( ret );
}

WORD X_File_close ( WORD handle , const char *file, long line )
{
WORD ret = handle;

	LOG_printf("> Fclose %s %ld <%d>\n", file, line, handle );

	if ( handle > 0 )
	{
		ret = Fclose( handle );
		handle = -1;
	}

return( ret );
}


int		X_Fdelete( const char *fname , const char *file, long line )
{
int ret;

	ret = Fdelete( fname );
	LOG_printf("> Fdelete %s %ld <%s> = <%ld>\n", file, line, fname, ret);

return ( ret );
}

FILE  *X_file_open( const char *filename, const char *mode , const char *file, long line )
{
FILE *ret;

	ret = fopen( filename, mode );
	if ( ret != NULL )
		LOG_printf("> fopen %s %ld <%s> = <%d>\n", file, line, filename , ret->Handle);
	else
		LOG_printf("> fopen %s %ld <%s> = NULL\n", file, line, filename);

return( ret );
}

WORD X_file_close ( FILE *stream , const char *file, long line )
{
WORD ret = -1;

	if ( stream != NULL )
	{
		ret = fclose( stream );
		LOG_printf("> fclose %s %ld <%d> = <%d>\n", file, line, stream->Handle , ret);
		stream = NULL;
	}
	else
		LOG_printf("> fclose %s %ld NULL\n", file, line );

return( ret );
}

int		X_remove( const char *fname , const char *file, long line )
{
int ret;

	ret = remove( fname );
	LOG_printf("> remove %s %ld <%s> = <%ld>\n", file, line, fname, ret);

return ( ret );
}

/* CAB */

UBYTE *VersionCABString( LONG version )
{
static UBYTE string[PATHSIZE+1];
UBYTE c1 = (UBYTE)((version>>24)&0xFF);
UBYTE c2 = (UBYTE)((version>>16)&0xFF);
UBYTE c3 = (UBYTE)((version>>8)&0xFF);
UBYTE c4 = (UBYTE)((version)&0xFF);

	if ( c1 != '0' )
		sprintf( string, "%c%c.%c%c", c1, c2, c3, c4);
	else
		sprintf( string, "%c.%c%c", c2, c3, c4);

return(string);
}

/* OVL */

UBYTE *VersionOVLString( LONG version )
{
static UBYTE string[PATHSIZE+1];

/*

 C'‚tait pour CAB.OVL marche t'il pour CAB ? => NON

*/

		if ( ((version>>24)&0xFF) != 0 )
			sprintf( string, "%lx%lx.%02lx",\
				(version>>24)&0xFF, (version>>16)&0xFF, (version>>8)&0xFF );
		else
			sprintf( string, "%lx.%02lx",\
				(version>>16)&0xFF, (version>>8)&0xFF );

return(string);
}

VOID trash( void *fmt , ... )
{

	if ( fmt )	return;

}

/*
**	Function:		Strftime()
**	Status:			Global
**	Description:	Fixes Y2K bug in strftime. %y yields to 10 not to 00
**	Returns:		Value of strftime
**	Author:		Daniel R”sen, J”rg Spilker
*/

EXTERN size_t  strftime( char *s, size_t max_size,
                                  const char *format,
                                  const struct tm *timeptr );

size_t Strftime(char *s, size_t max_size, const char *format, const struct tm *timeptr)
{

/*
	 Pour simplifier on n'a pas utilis‚ la fonction directe Stristr
	 de Daniel roesen
*/

	if ( (timeptr->tm_year >= 100) && (strstr(format, "%y")) )
		timeptr->tm_year = timeptr->tm_year - 100;
		
	return strftime(s, max_size, format, timeptr);
}
