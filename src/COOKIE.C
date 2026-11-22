/*
 * cookie.c
 * Gestion des cookies et points d'entr‚e de CAB avec cookie.
 *
 * D‚coupe les lignes Set-Cookie, demande … l'utilisateur d'accepter
 * ou de refuser chaque cookie, stocke les cookies accept‚s dans un
 * fichier et construit la ligne Cookie des requˆtes suivantes.
 * Fournit aussi get_url_cookie, get_url_ifmod_cookie et
 * post_cookie, variantes des points d'entr‚e qui passent un cookie.
 *
 * Utilise : CONFIG.H, GET_URL.H, GLOBALS.H, HTTP.H, HTTP_GET.H,
 *   IF_MODIF.H, MAIN.H, POST_URL.H, RSC.H, TOOLS.H, VERSION.H
 * Utilis‚ par : HEADER.C, HTTP_GET.C, MAIN.C, MY_HEAD.C, TOOLS.C
 *
 * Cr‚ation : 14/07/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

#define NOT_SET_EXPIRE
/* #define DEF_PATH_IS_ROOT */
/*------------------------------------------------
//
// Module: cookie.c
//
// Objet : 
//
// ToDo  : V‚rifier la date de validit‚
//
						mettre l'URI et non l'URL dans path si absent !!!
						
Set-Cookie: CUSTOMER=WILE_E_COYOTE; path=/; expires=Wednesday, 09-Nov-99 23:12:40 GMT

avec yahoo :

Set-Cookie: Y=v=1&n=9pnd5pgftg31k; domain=.yahoo.com; path=/; expires=Thursday, 31-Dec-98 23:00:00 GMT

void ___CDECL (*cookie)(char *url, char *name, char *value,  char *expires, char *path, char *domain, int secure);


!! Il est possible que cab.app ne fonctionne pas bien avec
des formes de date comme :"Wednesday, 01-Sep-99 07:41:51 GMT"
car il dit vouloir un autre format (voir plus bas)
=> faut-t'il transformer ?
pourtant pas mal de serveurs utilisent ce format!

//
//
// Maintenance :
//
//     Auteur  : Olivier Booklage
//     Version : V1.0
//     Date    : 14/07/1998
//     Remarq. :
//
//
//
//-----------------------------------------------*/


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"main.h"
#include	"http_get.h"
#include	"get_url.h"
#include	"post_url.h"
#include	"if_modif.h"

#include	"tools.h"
#include	"http.h"
#include	"config.h"
#include	"rsc.h"
#include	"tools.h"
#include	"version.h"

/* ---       Includes internes       -- */

#include "cookie.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL UBYTE *version;
LOCAL UBYTE *name;
LOCAL UBYTE *value;
LOCAL UBYTE *expires;
LOCAL UBYTE *path;
LOCAL UBYTE *domain;

/*LOCAL UBYTE *empty_string = "-";*/
LOCAL UBYTE *empty_string = "0";
#ifdef DEF_PATH_IS_ROOT
	LOCAL UBYTE *root_string = "/";
#endif
LOCAL UBYTE URI[1024];

LOCAL UBYTE line[128];
LOCAL UBYTE save[128];

LOCAL UBYTE domain_str[128];	/* rfc2019 : 4096 bytes pour chaques variables */
LOCAL UBYTE path_str[128];
LOCAL UBYTE expires_str[128];
LOCAL UBYTE data_str[128];
LOCAL INT		secure;

LOCAL UBYTE cookie_buffer[256];	/* Pour preparer */

UBYTE *cookie_str;
UBYTE *new_location_cookie_str;

/* ---     Prototypage interne      --- */

LOCAL BOOLEAN SplitCookie( REQUEST *request, UBYTE *line );

LOCAL VOID ProcessLine(BYTE *line, BYTE *p1, BYTE *p2, BYTE *p3, BYTE *p4, LONG limit);
EXTERN 	UBYTE *skipwhite ( const UBYTE *src );
LOCAL VOID propre( UBYTE *string );

/* ---         Fonctions            --- */

/*
Identisch to get_url, until to the  additional(ly) parameter(s) "cookie".
In "cookie" stands the  Cookie-Information for the  existing URL (or 0L, 
in the case that no Cookie-Information available). 
*/

long ___CDECL get_url_cookie(char *url, char *filename, char *cookie)
{

	LOG_printf("- ***************************************************\n");
	LOG_printf("> get_url_cookie\n");

	cookie_str = cookie;

return(http_get(url, "", "", filename, GET_METHOD,0L));
}

/*
Identisch to get_url_if_modified, until to the  additional(ly) parameter(s) "cookie".
In "cookie" stands the  Cookie-Information for the  existing URL (or 0L, in the case that
no Cookie-Information available). If this function implements\ed will(_be),
so must too get_url_if_modified become implements\ed (both are but optional).
*/
long ___CDECL get_url_ifmod_cookie(char *url, char *filename, long *time, char *cookie)
{

	LOG_printf("- ***************************************************\n");
	LOG_printf("> get_url_ifmod_cookie\n");

	cookie_str = cookie;
	
return(http_get(url, "", "", filename, GET_METHOD, *time));
}

/*
Identisch to post, until to the  additional(ly) parameter(s) "cookie".
In "cookie" stands the  Cookie-Information for the  existing URL (or 0L, in the case that
no Cookie-Information available). If this function implements\ed will(_be),
so must too post become implements\ed.

! content semble etre le filename !! 

*/
long ___CDECL post_cookie(char *url, char *content, char *enctype, char *filename, char *cookie)
{
	LOG_printf("- ***************************************************\n");
	LOG_printf("> post_cookie\n");

	cookie_str = cookie;

return(http_get(url, content, enctype, filename, POST_METHOD,0L));
}

BOOLEAN AddCookie( REQUEST *request, UBYTE *line )
{
FILE	*file_ptr;

	LOG_printf("> AddCookie\n");

	/* D‚coupage des informations */

	if ( config.cookie == REFUSE_COOKIE )
	{
		LOG_printf("~ Cannot set cookie because you refuse cookies.\n");
		return(FALSE);
	}

	if( SplitCookie( request, line ) != TRUE )
	{
		LOG_printf("! SplitCookie error, can not set cookie.\n");
		return (FALSE);
	}

	if ( config.cookie == ASK_COOKIE )
	{
		if ( ask_cookie_accepte( request->true_server, domain, path, name, value, expires) != COOK_ACCEPT )
			return (FALSE);
	}

	if ( ( cab_version >= 0x30323730L ) && ( config.cookie == CAB_COOKIE ) )
	{

/*
With this can the  OVL a "Cookie",that  from the  HTTP-answer
extract\ed became, put. Ever\y after effected settings will(_be) CAB
the  Cookie in the  Cookie-databank insert,that  user(s) ask,
or the  Cookie ignore.

Url -is the  URL of the  document, for that the  Cookie supplied became.
Name -name of the  Cookies
value -value of the  Cookies (or 0, in the case that no value placed)
expires -Verfallsdatum (or 0, in the case that not placed) as\than String in the
 format "Wdy, DD-Mon-YYYY HH:MM:SS GMT"
path -access path, for that the  Cookie is valid.
Domain -Domain, for that the  Cookie valid is
secure -0 for the  normal (unsichere) transfer, 1 for sure(ly) HTTPS.

This function is available since CAB 2.7 (i.e. version>=0x30323730)
and(!) if derentsprechende entry in the  structure is not 0L.
*/

#ifdef notdef
		if ( stricmp( expires, empty_string) == SUCCESS )
/*		expires = "0\0"; */
			expires = "0";
#endif

		LOG_printf("~ browser->cookie(");
		LOG_puts("~", request->URL);
		LOG_puts("~", ", ");
		LOG_puts("~", name);
		LOG_puts("~", ", ");
		LOG_puts("~", value);
		LOG_puts("~", ", ");
		LOG_puts("~", path);
		LOG_puts("~", ", ");
		LOG_puts("~", domain);
		LOG_puts("~", ", ");
		LOG_puts("~", expires);
		LOG_puts("~", ", ");
		if ( secure > 0)
			LOG_puts("~", "1");
		else
			LOG_puts("~", "0");
		LOG_puts("~", ");\n");
		browser->set_cookie( request->URL, name, value, expires, path, domain, secure);
		cookie_str = cookie_buffer; /* Nous pourrions l'annuler non ? */
	}
	else
	{
		if( (file_ptr=fopen(COOKIE_FILE,"a+")) !=NULL)
		{
			fwrite( domain, sizeof(UBYTE), strlen(domain), file_ptr);
			fprintf( file_ptr, "\t");

			fwrite( path, sizeof(UBYTE), strlen(path), file_ptr);
			fprintf( file_ptr, "\t");

			fwrite( expires, sizeof(UBYTE), strlen(expires), file_ptr);
			fprintf( file_ptr, "\t");

			fwrite( cookie_buffer, sizeof(UBYTE), strlen(cookie_buffer), file_ptr);
			fprintf( file_ptr, "\n");

			fclose(file_ptr);
			return(TRUE);
		}
		LOG_printf("~ Error open write cookie file <%s>\n",COOKIE_FILE);
	}

return(FALSE);
}

BOOLEAN SendCookie( UBYTE *domain , UBYTE *path, UBYTE *buffer, int limit)
{
FILE *file_handle;
UBYTE *p;
INT count = 0;

	LOG_printf("> SendCookie\n");
	
	if  ( ( cab_version < CAB_GERE_COOKIE ) && ( config.cookie == REFUSE_COOKIE ) )
		return (FALSE);
	
/* *cookie_buffer = '\0'; il a l'air d'effacer cookie_str !!*/ 

	if ( config.cookie == CAB_COOKIE )
	{
		if ( cookie_str != NULL  )
		{

			strncat( buffer, "Cookie: ", limit );
			strncat( buffer, cookie_str, limit );
			strncat( buffer, "\r\n", limit );
			LOG_printf("- set cookie <");
			LOG_puts("-", cookie_str);
			LOG_puts("-",">\n");
			cookie_str = NULL;

			*cookie_buffer = '\0';

			return(TRUE);
		}
		LOG_printf("~ cookie_str is NULL, no cookie string given by CAB.\n");

		*cookie_buffer = '\0';

		return(FALSE);
	}

	cookie_str = NULL;

	*cookie_buffer = '\0';

	file_handle = fopen(COOKIE_FILE, "r");
	
	if (file_handle == NULL)
	{
		LOG_printf("~ Cookie file <%s> not found\n",COOKIE_FILE);
		return(FALSE);
	}
	
	while ( fgets(line, 127, file_handle) != NULL )
	{
		if ( (*line != ';') && (*line != '#') )
		{
			if ( (p=strchr(line,10)) != NULL ) *p='\0';
			if ( (p=strchr(line,13)) != NULL ) *p='\0';
			ProcessLine(line, domain_str, path_str, expires_str, data_str, 128);
			if (*domain_str != EOS)
			{
				LOG_printf("~ CheckCookie domain <%s> on <%s>\n", domain, domain_str);
				if ( ( strstr( domain, domain_str ) != NULL ) || ( stricmp(domain, domain_str) == SUCCESS ) )
				{
					if ( *path_str != EOS )
					{
						LOG_printf("~ CheckCookie path <");
						LOG_puts("~",path);
						LOG_puts("~","> on <");
						LOG_puts("~",path_str);
						LOG_puts("~",">\n");
						if ( (strstr(path, path_str) != NULL) || ( stricmp(path, path_str) == SUCCESS ) )
						{
							if ( *data_str != EOS )
							{
								/* manque version , path, domain doivent etre optionnels */
								if ( count == 0 )
									strncat( buffer, "Cookie: ", limit );
								else
/*								strncat( buffer, ";\r\n", limit ); */
									strncat( buffer, "; ", limit );

								strncat( buffer, data_str, limit );

/* Nouveau, on n'envoi pas plus d'une ligne !! */

		fclose(file_handle);
		strncat( buffer, "\r\n", limit );
								count++;
		return( TRUE );

#ifdef NOTDEF
								LOG_printf("- set cookie <");
								LOG_puts("-", variable_str);
								LOG_puts("-",">\n");
#endif
							}
							else
								LOG_printf("~ CheckCookie variable empty\n");
						}
						else
							LOG_printf("~ CheckCookie path do not match\n");
					}
					else
						LOG_printf("~ CheckCookie empty path\n");
				}
				else
					LOG_printf("~ CheckCookie domain do not match\n");
/*
				fclose(file_handle);
				return(TRUE);
*/
			}
		}
	}
	fclose(file_handle);

	if ( count > 0 )
	{
		strncat( buffer, "\r\n", limit );
		return( TRUE );
	}

return(FALSE);
}

VOID	ClearCookie( VOID )
{
	LOG_printf("> ClearCookie\n");
	cookie_str = NULL;
	*cookie_buffer = '\0';
}

BOOLEAN CheckCookieDate( VOID )
{
FILE *file_handle;
FILE *tmp_handle;
time_t	cookie_time;
ULONG		ul_today;

	LOG_printf("> CheckCookieDate\n");
	
	if ( config.cookie == CAB_COOKIE )	return(FALSE);

	time(&( (time_t)(ul_today) ));
	
	file_handle = fopen(COOKIE_FILE, "r");
	
	if (file_handle == NULL)
	{
		LOG_printf("? Cookie file <%s> error\n",COOKIE_FILE);
		return(FALSE);
	}
	
	tmp_handle=fopen(TMP_FILE, "w");

	if ( tmp_handle == NULL )
	{
		LOG_printf("? Unable to open temporary file <%s>\n",TMP_FILE);
		fclose( file_handle );
		return(FALSE);
	}

	while ( fgets(line, 127, file_handle) != NULL )
	{
		*save = '\0';
		strncat( save, line, 255);
		ProcessLine(line, domain_str, path_str, expires_str, data_str, 128);
		if ( *expires_str != EOS )
		{
			if ( strcmp( expires_str, "-") != SUCCESS )
			{
				cookie_time = parse_date(expires_str);
				if ( (cookie_time < ul_today) && ( cookie_time != -1L ) )
					LOG_printf("~ CheckCookieDate <%s> (%ld) < (%ld) WILL DELETE\n", expires_str, cookie_time, ul_today);
				else
					fprintf( tmp_handle, save);
			}
			else
				LOG_printf("~ CheckCookieDate <%s> no expires : WILL DELETE\n", domain_str);
		}else	/* defaut */
			fprintf( tmp_handle, save);
	}

	fclose(file_handle);
	remove( COOKIE_FILE );
	fclose(tmp_handle);
	rename(TMP_FILE, COOKIE_FILE);

return(FALSE);
}

/* Extraction des elements (tokens) d'une chaine */

LOCAL VOID ProcessLine(BYTE *line, BYTE *p1, BYTE *p2, BYTE *p3, BYTE *p4, LONG limit)
{
char *token;
	
	LOG_printf("> ProcessLine <%s>\n",line);
	
	*p1 = *p2 = *p3 = *p4 = EOS;
	
	token = strtok(line, "\t");
	if (token)
	{
		*p1=0;	/* domain */
		strncat(p1, token, limit);
		token = strtok(NULL, "\t");
		if (token)
		{
			*p2=0;	/* path */
			strncat(p2, token,limit);
			token = strtok(NULL, "\t");
			if (token)
			{
				*p3=0;	/* expires */
				strncat(p3, token ,limit);
				token = strtok(NULL, "\0");
				if (token)
				{
					*p4=0;	/* datas */
					strncat(p4, token ,limit);
				}
			}
		}
	}
}

LOCAL BOOLEAN SplitCookie( REQUEST *request, UBYTE *line )
{
UBYTE *token;
UBYTE *ptr;
BOOLEAN find_path = FALSE;
BOOLEAN find_domain = FALSE;

	LOG_printf("> SplitCookie\n");
	
	token = strtok(line, ";");
	
	version = NULL;
	name = NULL;
	value = NULL;
	domain = NULL;
	path = NULL;
	expires = NULL;
	secure = 0;

	while ( token != NULL )
	{
		token = clearwhite( token );
		LOG_printf("~ SplitCookie: token =");
		LOG_puts("~",token);
		LOG_puts("~",">\n");

		if ( strnicmp( token, "version=", 8 ) == SUCCESS )
		{
			version = skipwhite( &token[8]);
		}
		else if ( strnicmp( token, "domain=", 7 ) == SUCCESS )
		{
			domain = skipwhite( &token[7]);
			find_domain = TRUE;
		}
		else if ( strnicmp( token, "path=", 5 ) == SUCCESS )
		{
			path = skipwhite( &token[5] );
			find_path = TRUE;
		}
		else if ( strnicmp( token, "expires=", 8 ) == SUCCESS )
		{
			expires = skipwhite( &token[8]);
		}
		else 	if ( (ptr=strrchr(token,'=')) != NULL ) /* variable */
		{
			/* Il faut d‚couper la valeur */
			name = token;
			value = skipwhite( &ptr[1]);
			*ptr ='\0';
		}
		else if ( strnicmp( token, "secure", 6 ) == SUCCESS )
		{
			secure = 1;
		}
		token = strtok(NULL, ";");
	}

	propre( name );
	propre( value );
	propre( path );
	propre( expires );
	propre( domain );
	
	if ( name == NULL )
	{
		LOG_printf("? SplitCookie: incomplete cookie data, no name found\n");
		return(FALSE);
	}
	if ( value == NULL )
	{
		LOG_printf("? SplitCookie: incomplete cookie data, no value found\n");
		return(FALSE);
	}
	if ( path == NULL )
	{
	UBYTE *p;
#ifdef DEF_PATH_IS_ROOT
		path = root_string;
		LOG_printf("~ SplitCookie: incomplete cookie data, path set as root.\n");
#else
		*URI = '\0';
		strncat( URI, request->URI, 1024);
		p = strchr( URI, '?');
		if ( p != NULL )	*p='\0';
		p = strrchr( URI, '/');
		if ( p != NULL )	*(p+1)='\0';
		path = URI;
		LOG_printf("~ SplitCookie: incomplete cookie data, path set as document path.\n");
#endif
	}
	if ( expires == NULL )
	{
#ifdef NOT_SET_EXPIRE
		expires = empty_string;
#else
		expires = dans_un_ans();
#endif
		LOG_printf("~ SplitCookie: incomplete cookie data, no expires found\n");
	}
	if ( domain == NULL )
	{
	UBYTE *ptr = strchr(request->true_server,'.');
		if ( ptr == NULL )
		{
			LOG_printf("! SplitCookie: incomplete cookie data, not domain available\n");
			return(FALSE);
		}
		domain = ptr;
		LOG_printf("~ SplitCookie: incomplete cookie data, domain set as server\n");
	}
#ifdef notdef
	else	if ( domain[0] != '.' )
	{
		LOG_printf("! SplitCookie: incomplete cookie data, domain do not start with '.'\n");
		return(FALSE);
	}
#endif

	LOG_printf("~ SplitCookie : name <");
	LOG_puts("~", name);
	LOG_puts("~",">\n");

	LOG_printf("~ SplitCookie : value <");
	LOG_puts("~", value);
	LOG_puts("~",">\n");

	LOG_printf("~ SplitCookie : domain <");
	LOG_puts("~", domain);
	LOG_puts("~",">\n");

	LOG_printf("~ SplitCookie : path <");
	LOG_puts("~", path);
	LOG_puts("~",">\n");

	LOG_printf("~ SplitCookie : expires <");
	LOG_puts("~", expires);
	LOG_puts("~",">\n");

	/* ligne pour cookie.set */
	
	*cookie_buffer = '\0';
	if ( version != NULL )
	{
		strncat( cookie_buffer, "$version=", 256);
		strncat( cookie_buffer, version, 256);
		strncat( cookie_buffer, "\r\n", 256);
	}
	strncat( cookie_buffer, name, 256);
	strncat( cookie_buffer, "=", 256);
	strncat( cookie_buffer, value, 256);
/*strncat( cookie_buffer, "", 256);*/
	if ( find_path == TRUE )
	{
		strncat( cookie_buffer, "; ", 256);
		strncat( cookie_buffer, "$path=", 256);
		strncat( cookie_buffer, path, 256);
/*	strncat( cookie_buffer, "", 256); */
	}
	if ( find_domain == TRUE )
	{
		strncat( cookie_buffer, "; ", 256);
		strncat( cookie_buffer, "$domain=", 256);
		strncat( cookie_buffer, domain, 256);
/*	strncat( cookie_buffer, "", 256); */
	}

return(TRUE);
}

LOCAL VOID propre( UBYTE *string )
{
	if ( string == NULL ) return;
	
	while ( *string != '\0' )
	{
		if ( ( *string == 13 ) || ( *string == 10 ) )	*string = '\0';
		string++;
	}
}

/* ---          EOF                 --- */
