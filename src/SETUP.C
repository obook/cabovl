/*
 * setup.c
 * Page de r‚glage du module (http://cab.ovl).
 *
 * G‚nŠre la page HTML de configuration et lit les valeurs
 * envoy‚es par le formulaire.
 *
 * Utilise : CONFIG.H, GLOBALS.H, HTTP.H, MAIN.H, REPORT.H, RSC.H,
 *   TOOLS.H, VERSION.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 17/06/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<atarierr.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<sting\sting.h>
#include	<upoint\memory.h>
#include	<upoint\struct.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"main.h"
#include	"version.h"
#include	"http.h"
#include	"tools.h"
#include	"rsc.h"
#include	"report.h"
#include	"config.h"
#include	"tools.h"

/* ---       Includes internes       -- */

#include	"setup.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL UBYTE	query_string[256];
LOCAL UBYTE	buffer[256];

/* ---     Prototypage interne      --- */

LOCAL INT			open_file( REQUEST *request, UBYTE *file);
LOCAL	LONG 		close_file( REQUEST *request, INT file_handle );
LOCAL VOID		printf_setup( REQUEST *request, INT file_handle);
LOCAL BOOLEAN	regle_setup( REQUEST *request, INT file_handle, UBYTE *query );
LOCAL VOID		printf_string( REQUEST *request, INT file_handle, BYTE *fmt, ... );

/* ---         Fonctions            --- */

LONG SendSetup( REQUEST *request, UBYTE *uri)
{
UBYTE	*p;
INT file_handle;

	LOG_printf("> SendSetup <%s>\n", uri );
	
	if ( (file_handle=open_file( request, request->filename )) < 0 )
	{
		return( EWRITF );
	}
	
	*query_string = '\0';

	if ( (*uri == '\0') || ( strnicmp( uri, "welcome.html", 12) == SUCCESS ))
		printf_setup( request, file_handle );
	else
	if ( strnicmp( uri, "setup.html", 10) == SUCCESS )
		printf_setup( request, file_handle);
	else
	if ( strnicmp( uri, "setup-cgi", 9) == SUCCESS )
	{
		if ( (p=strchr( uri,'?') ) !=NULL )
			strncat( query_string, p+1 ,255);
		regle_setup( request, file_handle, query_string );
	}

	close_file( request, file_handle);
	
return( 0L );
}

LOCAL INT open_file( REQUEST *request, UBYTE *file)
{
INT file_handle;

	clear_info(request);
	request->content_length = 0L;
	strncat( request->content_type, "text/html", 255);
	request->mime_version = (double)(1.0);
	strncat( request->server ,"cab.ovl", 255);

	file_handle=(int)Fcreate(file, 0);
	
	if (file_handle<0)
	{
		browser->msg_error(EINTRN);
		ReportRscMessage( file, &file_handle, REPO_CACHE_FILE);
		LOG_printf("? Couldn't create %s!\n", file);
	}

return( file_handle );
}

LOCAL	LONG close_file( REQUEST *request, INT file_handle )
{
INT x;

	if( file_handle <=0 )	return(-1L);
	
	if(	(x=Fclose(file_handle)) < 0 )
	{
		browser->msg_error(EINTRN);
		ReportRscMessage( request->filename, &file_handle, REPO_TMP_CLOSE);
		LOG_printf("? Error with Fclose! (%i)\n", x);
		return( EWRITF );
	}

return(0L);
}

LOCAL BOOLEAN	regle_setup( REQUEST *request, INT file_handle, UBYTE *query )
{
UBYTE *token;
BOOLEAN logfile_active = LOG_isopen();
BOOLEAN close_logfile = TRUE;
UBYTE *string;

	LOG_printf("> regle_setup\n");
	
	LOG_level(DEBUG_0);

	config.logfile_open = FALSE;
	config.debug = FALSE;
	config.cookie = CAB_COOKIE;
	config.online = FALSE;
	
	token = strtok( query, "&");
	while( token != NULL )
	{
		LOG_printf("regle_setup process <%s>\n",token);

		/* "a" : logfile_open */
		
		if ( strnicmp( token, "a=on", 4) == SUCCESS )
		{
			config.logfile_open = TRUE;
			LOG_printf("! regle_setup : logfile = ON\n");
			if ( logfile_active == FALSE )
			{
				/*               						                        							      rien    !       ? */
				LOG_open( LOG_FILE, 500L, LOG_CESSION|LOG_FREE|LOG_CLOCKSTAMP, "OVL ", DEBUG_0|DEBUG_2|DEBUG_3);
				LOG_welcome();
			}
			close_logfile = FALSE;
		}
		else

		/* "b" : mode debug */

		if ( strnicmp( token, "b=on", 4) == SUCCESS )
		{
			config.debug = TRUE;
			/*        rien    ~       !      	?				>			 */
			LOG_level(DEBUG_0|DEBUG_1|DEBUG_2|DEBUG_3|DEBUG_12);
			LOG_printf("~ CAB version <V%s>\n", cab_version_str );
			LOG_printf("~ OVL version <V%s>\n", ovl_version_str );
		}
		else

		/* "d" : timeout */

		if ( strnicmp( token, "d=", 2) == SUCCESS )
		{
			config.timeout = atoi(&token[2]);
		}
		else

		/* "g" : gem timer */

		if ( strnicmp( token, "g=", 2) == SUCCESS )
		{
			config.gem = (FLOAT)atoi(&token[2]);
		}

		/* "h" : interne timer */

		else
		if ( strnicmp( token, "h=", 2) == SUCCESS )
		{
			config.timer = atoi(&token[2]);
		}
		else

		/* "i" : use cookie type */

		if ( strnicmp( token, "i=", 2) == SUCCESS )
		{
			config.cookie = atoi(&token[2]);
		}
		else

		/* "o" : online */

		if ( strnicmp( token, "o=on", 4) == SUCCESS )
		{
			config.online = TRUE;
		}

		token = strtok(NULL,"&");
	}
	
	if ( close_logfile == TRUE )
	{
		LOG_bye();
		LOG_close();
		config.logfile_open = FALSE;
	}

	SaveConf(CONFIG_FILE);
	
	/* affichage de la page OK */
	
	while ( NextString( SETOK, &string ) > 0 )
	{
		printf_string( request, file_handle , string);
	}

return( TRUE );
}

LOCAL VOID	printf_setup( REQUEST *request, INT file_handle)
{
ULONG ul_today;
UBYTE date[51];
UBYTE *string;
INT index;

	time(&( (time_t)(ul_today) ));
	Strftime(date,50,"%a, %d %b %Y %X",localtime(&((time_t)(ul_today))));

	/* affichage de la page OK */
	
	while ( (index=NextString( SETUP, &string )) > 0 )
	{

		if ( index == SETU_DATE )
		{
			printf_string( request, file_handle, date);
		}
		else if ( index == SETU_ADDRESS )
		{
			printf_string( request, file_handle,"cab.ovl (c) <A HREF=\"mailto:xxx@xxx.xxx\">Olivier Booklage</A>");
		}
		else if ( index == SETU_LOGFILE )
		{
			printf_string( request, file_handle , string);
			if ( config.logfile_open == TRUE )
				printf_string( request, file_handle, "checked");
		}
		else if ( index == SETU_DEBUG )
		{
			printf_string( request, file_handle , string);
			if ( config.debug == TRUE )
				printf_string( request, file_handle, "checked");
		}
		else if ( index == SETU_TIMEOUT )
		{
			printf_string( request, file_handle , string, config.timeout);
		}
		else if ( index == SETU_GEM )
		{
			printf_string( request, file_handle , string, (INT)config.gem);
		}
		else if ( index == SETU_TIMER )
		{
			printf_string( request, file_handle , string, config.timer);
		}
		else if ( index == SETU_ONLINE )
		{
			printf_string( request, file_handle , string);
			if ( config.online == TRUE )
				printf_string( request, file_handle, "checked");
		}
		else if ( index == SETU_ACC_COOK )
		{
			if ( config.cookie == ACCEPT_COOKIE )
				printf_string( request, file_handle , string, " SELECTED");
			else
				printf_string( request, file_handle , string, "");
		}
		else if ( index == SETU_REF_COOK )
		{
			if ( config.cookie == REFUSE_COOKIE )
				printf_string( request, file_handle , string, " SELECTED");
			else
				printf_string( request, file_handle , string, "");
		}
		else if ( index == SETU_STO_COOK )
		{
			if ( config.cookie == STORE_COOKIE )
				printf_string( request, file_handle , string, " SELECTED");
			else
				printf_string( request, file_handle , string, "");
		}
		else if ( index == SETU_ASK_COOK )
		{
			if ( config.cookie == ASK_COOKIE )
				printf_string( request, file_handle , string, " SELECTED");
			else
				printf_string( request, file_handle , string, "");
		}
		else if ( index == SETU_CAB_COOK )
		{
			if ( config.cookie == CAB_COOKIE )
				printf_string( request, file_handle , string, " SELECTED");
			else
				printf_string( request, file_handle , string, "");
		}
		else
			printf_string( request, file_handle , string);
	}

}

LOCAL VOID printf_string( REQUEST *request, INT file_handle, BYTE *fmt, ... )
{
LONG	len;
va_list	args;

	if (file_handle<0)	return;

	va_start(args,fmt);
	vsprintf(buffer,fmt,args);
	va_end(args);
	
	len = strlen(buffer);
	
	request->content_length += len;

	if ( Fwrite(file_handle, len, buffer) < 0L )
	{
		browser->msg_error(EINTRN);
		ReportRscMessage( request->filename, &file_handle, REPO_CACHE_WRITE);
		LOG_printf("! cab.ovl: WARNING during write on file, disk full?\n");
	}
	
}

/* ---          EOF                 --- */
