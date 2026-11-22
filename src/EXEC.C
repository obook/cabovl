/*
 * exec.c
 * Lancement d'un programme ou du shell (ExecPrg).
 *
 * Pr‚pare l'environnement et appelle le programme (CallShell).
 *
 * Utilise : ENV.H, GLOBALS.H, TOOLS.H
 *
 * Cr‚ation : 09/11/1997
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/*------------------------------------------------
//
// Module: exec.c
//
// Objet : lancement d'un prg ou du shell
//
// ToDo  :	Terrible bug de pile d‚bord‚e car trop de
//					donn‚es locales dans la fonction ExecPrg => fix‚
//
//
//
// Maintenance :
//
//     Auteur  : Olivier Booklage
//     Version : V1.0
//     Date    : 09/11/1997
//     Remarq. :
//
//
//
//-----------------------------------------------*/


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ext.h>
#include	<portab.h>
#include	<sting\sting.h>
#include	<logfile.h>

#include	"globals.h"
#include	"xprintf.h"
#include	"tools.h"
#include	"clients.h"
#include	"url.h"
#include	"file.h"
#include	"env.h"

/* ---       Includes internes       -- */

#include "exec.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

/* LOCAL CLIENT_HTTP tmp; */

LOCAL UBYTE cbuf[127];
LOCAL UBYTE url_tmp[PATH_MAX+1];
LOCAL UBYTE path[PATH_MAX+1];
LOCAL UBYTE name[PATH_MAX+1];
LOCAL WORD	non_std_handleout;
LOCAL WORD	non_std_handlein;
LOCAL UBYTE *envptr;
LOCAL UBYTE SHELL_FILE[PATH_MAX+1];	/* chemin du shell dans l'environnement */

/* ---     Prototypage interne      --- */

LOCAL INT CallShell( UBYTE *batchfile, UBYTE *command, UBYTE *envptr, WORD type);
LOCAL BOOLEAN Prepare( CLIENT_HTTP *client );
LOCAL VOID UnPrepare( CLIENT_HTTP *client );

/* ---         Fonctions            --- */

/* ----- Execution d'un programme ----- */

int16 ExecPrg(CLIENT_HTTP *client)
{
int		retour;	/* retour d'execution */
int 	ret;		/* retour de divers fonctions */
int		status;

	LOG_printf("> ExecPrg %s\n",client->netfile.filepath);

	if ( Prepare( client ) != TRUE )	return(-1);

	if( client->url.ptr_query_string != NULL )
	{
		UBYTE *ptr;

/* Par securit‚, ou retire le ";" MUPFEL prend comme commande suivante */

		if( (ptr=strchr(client->url.ptr_query_string,';'))!= NULL )
			*ptr = 0;
		strncpy( &cbuf[1], client->url.ptr_query_string, 126 );
		cbuf[127] = 0;
		cbuf[0] = (int)strlen( &cbuf[1] );
	}
	else
	{
		cbuf[0] = 0; /* Length zero */
		cbuf[1] = 0;
	}

/* Pexec */

	LOG_printf("~ Pexec %s %s\n",client->netfile.filepath,&cbuf[1]);

	if ( (client->netfile.type == NETFILE_TYPE_BATCH) || (client->netfile.type == NETFILE_TYPE_PERL) )
		retour=CallShell(client->netfile.filepath,cbuf,envptr,client->netfile.type);
	else
		retour=(INT)Pexec(0,client->netfile.filepath,cbuf,envptr);

	LOG_printf("~ Pexec return %d\n",retour);

	if ( retour != 0 )
	{
/*
		xprintf("! Pexec %s error %d (%s)\n",client->netfile.basename,retour,sys_errlist[errno]);
		LOG_printf("! Pexec <%s> error (%d) <%s>\n",client->netfile.filepath,retour,sys_errlist[errno]);
*/
		xprintf("! Pexec %s error %d\n",client->netfile.filepath, retour);
		LOG_printf("! Pexec <%s %s> error (%d)\n",client->netfile.filepath, &cbuf[1], retour);
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		UnPrepare( client );
		return(-1);
	}

	sprintf(url_tmp,"/%s.tmp",wrkname);

/*if ( ParseFile( &tmp, url_tmp, IS_TEMP_FILE) == FALSE ) */

	if ( strnicmp(client->netfile.basename,"nph",3) == SUCCESS )
		status = ADD_NOTHING;
	else
		status = ADD_STATUS;

	if ( ParseFile( client, url_tmp, IS_TEMP_FILE) == FALSE )
	{
/*	xprintf("? ParseFile %s ERROR\n",tmp.netfile.filepath); */
		xprintf("? ParseFile %s ERROR\n",client->netfile.filepath);
		LOG_printf("? ParseFile %s ERROR\n",client->file_process_out);
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		UnPrepare( client );
		return(-1);
	}

	UnPrepare( client );	/* Redirige, referme, remet Dcurpath */

	ret =  SendFile( client, &client->netfile, status, KILL);

#ifdef notdef
	*client->netfile.filepath = 0;
	strncat( client->netfile.filepath , tmp.netfile.filepath, PATH_MAX );
#endif
		
return(ret);
}


LOCAL INT CallShell( UBYTE *batchfile, UBYTE *command, UBYTE *envptr, WORD type)
{
UBYTE *shell;
/* UBYTE *ptr; */
LOCAL UBYTE cbuf[127];

	LOG_printf("> CallShell\n");

	*SHELL_FILE=0;
	
	if ( type == NETFILE_TYPE_BATCH )
		shell = getenv("SHELL");
	else
		shell = getenv("PERL");

	if ( shell == NULL )
	{
			xprintf("? 'SHELL/PERL' environnement not set\n");
			LOG_printf("? 'SHELL/PERL' environnement not set\n");
			return(-1);
	}
	else
	{
		strncat(SHELL_FILE,shell,PATH_MAX);
	}

	cbuf[0] = 0; /* Length zero */
	cbuf[1] = 0;


	if( batchfile != NULL )
	{
		strncat( &cbuf[1], batchfile, 126 );
	}

	if( command != NULL )
	{
		strncat( &cbuf[1], " ", 126 );
		strncat( &cbuf[1], command, 126 );
	}

	cbuf[127] = 0;
	cbuf[0] = (int)strlen( &cbuf[1] );

return( (INT)Pexec(0,SHELL_FILE,cbuf,envptr) );
}

LOCAL BOOLEAN Prepare( CLIENT_HTTP *client )
{
int 	ret;		/* retour de divers fonctions */

	LOG_printf("> Prepare\n");

	client->handle_process_out	= -1;
	client->handle_process_in		= -1;

	BuildTMP(client->file_process_out, config_http.tmp_path, PATH_MAX);
	
/* Changement de r‚pertoire */

	file_splitt(client->netfile.filepath,path,name);

	if ( SetPath(path) != TRUE )
	{
		xprintf("? SetPath <%s> error\n",path);
		LOG_printf("? SetPath <%s> error\n",path);
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		UnPrepare( client );
		return( FALSE );
	}

	/* Environnement */

	if ( (envptr = NewEnv(1024L)) != NULL )
	{
	UBYTE buf[MAX_STR+1];

#ifdef notdef
		*client->software=0;
		strncat(client->software,SERVER_VERSION,MAX_STR);
#endif

		sprintf(buf,"SERVER_SOFTWARE=%s/%s", PRG_NAME, SERVER_VERSION);
		PutEnv(buf);

		sprintf(buf,"SERVER_NAME=%s",client->host_name);
		PutEnv(buf);

		PutEnv("GATEWAY_INTERFACE=CGI/1.1");

		sprintf(buf,"SERVER_PORT=%u",config_http.port);
		PutEnv(buf);

		sprintf(buf,"SERVER_PATH=%s",config_http.path_html);
		PutEnv(buf);

		if ( client->url.ptr_accept != NULL )
			sprintf(buf,"HTTP_ACCEPT=%s",client->url.ptr_accept);
		else
			sprintf(buf,"HTTP_ACCEPT=");
		PutEnv(buf);

		if ( client->url.ptr_agent != NULL )
			sprintf(buf,"HTTP_USER_AGENT=%s",client->url.ptr_agent);
		else
			sprintf(buf,"HTTP_USER_AGENT=");
		PutEnv(buf);

		if ( client->url.ptr_version != NULL )
			sprintf(buf,"SERVER_PROTOCOL=%s",client->url.ptr_version);
		else
			sprintf(buf,"SERVER_PROTOCOL=");
		PutEnv(buf);

		sprintf(buf,"REMOTE_ADDR=%s",inetoa(client->remote_addr));
		PutEnv(buf);

		sprintf(buf,"REMOTE_HOST=%s",client->remote_name);
		PutEnv(buf);

		if ( client->url.ptr_content_type != NULL )
			sprintf(buf,"CONTENT_TYPE=%s",client->url.ptr_content_type);
		else
			sprintf(buf,"CONTENT_TYPE=");
		PutEnv(buf);

		sprintf(buf,"PATH_INFO=%s",client->url.path_info);
		PutEnv(buf);

		sprintf(buf,"PATH_TRANSLATED=%s",client->url.path_translated);
		PutEnv(buf);

		if ( client->url.ptr_authorization != NULL )
			sprintf(buf,"REMOTE_IDENT=%s",client->url.ptr_authorization);
		else
			sprintf(buf,"REMOTE_IDENT=");
		PutEnv(buf);

		if ( client->url.ptr_cookie != NULL )
			sprintf(buf,"HTTP_COOKIE=%s",client->url.ptr_cookie);
		else
			sprintf(buf,"HTTP_COOKIE=");
		PutEnv(buf);

		sprintf(buf,"SCRIPT_NAME=%s",client->url.ptr_url);
/*	sprintf(buf,"SCRIPT_NAME=%s",client->netfile.filepath); */
		PutEnv(buf);

#ifdef notdef

		/* Pour Ronald Andersson */
 
		PutEnv("URL_REL_PATH=");/*    The new alias for old variable PATH_INFO */
		PutEnv("URL_ABS_PATH=");/*    The new variable for absolute path of physical HTML file. */
		PutEnv("CGI_ABS_PATH=");/*    The new alias for old variable PATH_TRANSLATED  */

#endif
		
		/* pour WebStar */
		
		PutEnv("ANNOTATION_SERVER=");
		
		/* A faire : */

		if ( client->url.ptr_http_referer != NULL )
			sprintf(buf,"HTTP_REFERER=%s",client->url.ptr_http_referer);
		else
			sprintf(buf,"HTTP_REFERER=");
		PutEnv(buf);

		PutEnv("AUTH_TYPE=");
		PutEnv("REMOTE_USER=");

		if ( client->url.request_methode == URL_METHODE_GET )
		{
			PutEnv("REQUEST_METHOD=GET");
			PutEnv("CONTENT_LENGTH="); /* on le met quand mˆme, certaines lib le r‚clame */

			if ( client->url.ptr_query_string != NULL )
			{
				if ( *client->url.ptr_query_string != 0 )
				{
				INT i;
					sprintf(buf,"QUERY_STRING=%s",client->url.ptr_query_string);
					PutEnv(buf);
						/* d‚coder le %hexa */
					unescape_url(client->url.ptr_query_string);
					/* retirer les '+' */
					for ( i=0; client->url.ptr_query_string[i] != 0;i++)
						if ( client->url.ptr_query_string[i] == '+') client->url.ptr_query_string[i]=32;
				}else PutEnv("QUERY_STRING=");
			}else PutEnv("QUERY_STRING=");
		}
		else	/* POST */
		if ( client->url.request_methode == URL_METHODE_POST )
		{
			PutEnv("REQUEST_METHOD=POST");
			sprintf(buf,"CONTENT_LENGTH=%ld",client->data_lenght);
			PutEnv(buf);
		}/* GET/POST */
	}
	else /* NewEnv */
	{
		xprintf("? L'environnement n'est pas disponible\n");
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		/* sortir avec erreur ? */
	}

/* Conservation des handles stdin et stdout */

	non_std_handleout	 = (INT)Fdup( stdout->Handle );
	non_std_handlein 	 = (INT)Fdup( stdin->Handle );
		
/* fichier de r‚cup‚ration */

	if ( (client->handle_process_out=(INT)File_create(client->file_process_out,0)) < 0 )
	{
		xprintf("? Fcreate '%s' Error = %ld\n",client->file_process_out,client->handle_process_out);
		LOG_printf("? Fcreate '%s' Error = %ld\n",client->file_process_out,client->handle_process_out);
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		UnPrepare( client );
		return( FALSE );
	}
	else LOG_printf("~ Create OutFile <%s>\n",client->file_process_out);

	if ( (ret=Fforce (stdout->Handle,(WORD)client->handle_process_out )) != 0 ) /* redirection */ 
	{
		xprintf("? Fforce '%s' stdout error (%d)\n", client->file_process_out, ret);
		LOG_printf("? Fforce '%s' stdout error (%d)\n", client->file_process_out, ret);
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		UnPrepare( client );
		return( FALSE );
	}

/* fichier des donn‚es si POST */

	if ( *client->file_process_in != 0 ) /* nom de fichier non-vide */
	{
		if ( (client->handle_process_in=(INT)File_open(client->file_process_in,FO_READ)) < 0 )
		{
			xprintf("? Fopen %s Error %d\n",client->file_process_in,(WORD)client->handle_process_in);
			LOG_printf("? Fopen %s Error %d\n",client->file_process_in,(WORD)client->handle_process_in);
			client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
			memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
			UnPrepare( client );
			return( FALSE );
		}
		else LOG_printf("~ Open InFile <%s>\n",client->file_process_in);

		if ( (ret=Fforce (stdin->Handle,(WORD)client->handle_process_in )) != 0 ) /* redirection */ 
		{
			xprintf("? Fforce '%s' stdin error (%d)\n", client->file_process_in, ret);
			LOG_printf("? Fforce '%s' stdin error (%d)\n", client->file_process_in, ret);
			client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
			memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
			UnPrepare( client );
			return( FALSE );
		}
	}
	
return( TRUE );
}

LOCAL VOID UnPrepare( CLIENT_HTTP *client )
{
int 	ret;		/* retour de divers fonctions */

	LOG_printf("> UnPrepare\n");

	/* On lib‚re l'environnement */
	
	FreeEnv();

	/* Refermer le fichier en sortie */

	if ( client->handle_process_out > 0 )
	{
		if ( (ret=Fforce(stdout->Handle,non_std_handleout)) != 0 )
			LOG_printf("? Fforce stdout error (%d)\n",ret);
		if( (ret=File_close( (WORD)client->handle_process_out )) != 0 )
			LOG_printf("? Fclose <%s> error (%d)\n",client->file_process_out,ret);
		client->handle_process_out = -1;
		client->kill_process_out = TRUE;
	}

	/* Refermer le fichier en entree */

	if ( client->handle_process_in > 0 )
	{
		if ( (ret=Fforce(stdin->Handle,non_std_handlein)) != 0 )
			LOG_printf("? Fforce stdin error (%d)\n",ret);
		if ( (ret=File_close( (WORD)client->handle_process_in )) != 0 )
			LOG_printf("? Fclose <%s> error (%d)\n",client->file_process_in,ret);
		client->handle_process_in = -1;
		client->kill_process_in = TRUE;
	}

/* retour au r‚pertoire */

	file_splitt( config_http.work_path, path, name);

	if ( SetPath(path) != TRUE )
	{
		LOG_printf("? SetPath <%s> error\n",path);
	}

}

/* ---          EOF                 --- */
