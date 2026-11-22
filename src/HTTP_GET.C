/*
 * http_get.c
 * Routine g‚n‚rale des requˆtes HTTP (GET, POST, HEAD).
 *
 * http_get ouvre la connexion, envoie la requˆte, lit l'en-tˆte et
 * le corps de la r‚ponse, puis suit les redirections. Elle d‚lŠgue
 * l'authentification, les cookies, les pages internes (setup,
 * ic“nes), les rapports d'erreur et mailto aux autres modules.
 * C'est le fichier central du module.
 *
 * Utilise : AUTHEN.H, B64ENCOD.H, CACHE_IP.H, CONFIG.H, COOKIE.H,
 *   GET_URL.H, GLOBALS.H, HEADER.H, HTTP.H, JAVA.H, MAILTO.H, MAIN.H,
 *   MY_HEAD.H, POST_URL.H, REPORT.H, RSC.H, SENDICON.H, SETUP.H, TCP.H,
 *   TOOLS.H
 * Utilis‚ par : COOKIE.C, GET_INFO.C, GET_URL.C, IF_MODIF.C, POST_URL.C
 *
 * Cr‚ation : 11/06/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/*#define _RUN_JAVA_ */
/*#define WATCH_EXPECTED	*/	/* regarder le content-lenght */
#define NO_WATCH_HEADER		/* laisser passer si pas de header */
#define NO_ERASE_CACHE		/* Rapport d'erreur si le cache est vide */
/* #define USE_TCP_WAIT_STATE */
/*------------------------------------------------
//
// Module: http_get.c
//
// Objet : routine generale HTTP
//
// ToDo  : REVOIR LE CODAGE DES CARACTERES DE L'URI COMMME VCF X.gif = VCF%20X.gif ???
//
//
//
// Maintenance :
//
//     Auteur  : Olivier Booklage
//     Version : V1.0
//     Date    : 11/06/1998
//		 Remarq. :
//
//								bug:si pas connection, ecrase le cache :(
//
// http_get: File <G:\WWWCACHE\a\00000702.html>=<-32> is not cached
// sous TOS 4.0 il est evident que *.html soit -32 !!
//
	if ( file == 0L )					> a r‚gler !!!!!!!!!!!!!!!!!!!!!!!!
		file = TMP_FILE;	+ ligne 565 (Fcreate)
//
//								Le source est encore trop gros, = d‚couper.
//
//
//-----------------------------------------------*/


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<ext.h>
#include	<atarierr.h>
#include	<tos.h>	/* Fdatime() */
#include	<portab.h>
#include	<logfile.h>
#include	<upoint\memory.h>
#include	<upoint\struct.h>
#include	<tos.h>
#include	<cab\module.h>

#include	<sting\transprt.h>		/* Pour  TESTABLISH */

#include	"globals.h"
#include	"config.h"
#include	"tcp.h"
#include	"cache_ip.h"
#include	"http.h"
#include	"main.h"
#include	"get_url.h"
#include	"post_url.h"
#include	"authen.h"
#include	"my_head.h"
#include	"header.h"
#include	"cookie.h"
#include	"mailto.h"


#include	"rsc.h"
#include	"setup.h"
#include	"report.h"

#include	"sendicon.h"
#include	"b64encod.h"
#include	"tools.h"
#include	"java.h"

/* ---       Includes internes       -- */

#include "http_get.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

REQUEST	request;

/* --- Variables globales  internes --- */

LOCAL	INT timeouts;						/* 3 valeurs, 1, 2 et 3 */

/* Eviter de resoudre : Il faudrait faire un tableau avec 3 ou 4 adresse car sans proxy
certaines pages utilisent quelques renvois sur d'autres sites */
/*
LOCAL UBYTE last_server[256];
LOCAL uint32 last_addr;
*/
/* message d'alerte offline */

LOCAL BOOLEAN alert_offline = TRUE;

/* Certaines choses a initialiser */

LOCAL BOOLEAN premier_lancement = TRUE;

/* Authentification */

/*
LOCAL UBYTE *name_ptr;
LOCAL UBYTE *password_ptr;
*/

LOCAL UBYTE http_proxy[256];
LOCAL INT		http_proxy_port;

/* Locations */

/* static	UBYTE		new_location[256]; */
/* static  UBYTE		new_file[1024]; */

/* ---     Prototypage interne      --- */

LOCAL int16 open_connection(char *server, int port, int TOS);
LOCAL WORD HeaderIsReady( REQUEST *request, memdb *header);
LOCAL long MakeHeadlist ( char *start, txtdb *lst, memdb *mem );
LOCAL UBYTE *new_file_ptr;

EXTERN UBYTE *inetoa( uint32 ip );

/* ---         Fonctions            --- */

/* Rapide patch pour pallier aux erreures de redirection */

typedef struct Retour
{
	UBYTE		location[256];		/* URL suivante si redirection */
	UBYTE		file[1024];				/* filename du cache */
	BOOLEAN redirection;			/* Si TRUE a rediriger */
	LONG		value;						/* A retourner a CAB */
	LONG		bytes_read;				/* Pour info */
}RETOUR;

LOCAL RETOUR	retour;

LOCAL LONG	Reception(char *URL, char *content, char *enctype, char *file, int method, long lastmodified);

LONG http_get(char *URL, char *content, char *enctype, char *file, int method, long lastmodified)
{
LONG ret;
INT loop;

	retour.redirection = TRUE;
	retour.value = 0L;
	retour.bytes_read = 0L;
	*retour.location ='\0';

	/* On elimine les CR et LF ‚ventuels */
	
	/*
	Ca suffit pas !!! c'est pas la le probleme des retours a la ligne
	dans le fichier log, il me faudrait la page HTML contenant l'URL
	qui pose probleme....ou bug d'une version de CAB ?
	
	*/
	
	loop = 0;

	while( *URL != '\0' )
	{
/*	if ( (*URL!=0xA) && (*URL!=0xD) ) test */
		{
			retour.location[loop++] = *URL;
		}
		if( loop > 255 ) break;
		URL++;
	}
	
	retour.location[loop] = '\0';

/*strncat( retour.location, URL, 256 );*/

	*retour.file ='\0';
	strncat( retour.file, file, 1024);

	while( retour.redirection == TRUE )
	{

LOG_printf("~ Call Reception for <%s>\n", retour.location);

		ret = Reception( retour.location, content, enctype, retour.file, method, lastmodified);
		
		/* En cas de redirection, on passe en GET sans parametre */

		content = "";
		enctype = "";
		if ( method == POST_METHOD )
			method = GET_METHOD;

	}

	LOG_printf("~ END : return_code (%ld) for file contain (%ld) bytes\n", retour.value, retour.bytes_read);

/*Fdelete(TMP_FILE);	*//* C'est vraiment sale... */

 if ( retour.bytes_read > 0L )
 	browser->msg_status( STATUS_FORMATTING_TEXT , 0L);
	
return( ret );
}

LOCAL LONG	Reception(char *URL, char *content, char *enctype, char *file, int method, long lastmodified)
{

int16 x;

INT		readed=0;
INT		ret;
INT		bytes_available;
INT		eof=0;
INT		no_data;

LONG	bytes_read=0L;
LONG	old_bytes_read=0L;
LONG	return_code=0L;
LONG	lret=0L;

time_t timeout;
time_t call_cab;

DOSTIME filetime;

struct tm *urltime;

BOOLEAN proxy;

memdb header=_MEMDB;
/* memdb name_pass=_MEMDB; */
/* memdb authentification=_MEMDB; */

XATTR xattr;
LONG test;

UBYTE *p;


							if ( retour.redirection == FALSE )
							{
								*retour.file = '\0';
								strncat(retour.file, file, 1023);
							}
							retour.redirection = FALSE;


/*

- 02 Apr 20:20:42 OVL  URL   : <javascript:benoitlive()>
est tres mal d‚coup‚ par split_url !

*/

	LOG_printf("- URL   : <");
	LOG_puts("-",URL);
	LOG_puts("-",">\n");


/*

 Test car parfois URL tronqu‚e, par une redirection ??
 ou bien si elle contient des % les LOG_printf ou printf les
 d‚truisent ???? non, dans les forms avec parametre de plus de 80 car#
 CAB transmet tronqu‚ ? ( a verifier avec weblight )
 
 toujours des problemes avec VCF X.gif faut-t'il coder en 
 
 VCF%20X.gif => fait plus bas !!

*/

/*	mailto ----------------------------------------------------------- */

/* Il faudrait transmettre le type */

	if ( strnicmp( URL, "mailto:", 7 ) == SUCCESS )
	{
		return( my_mailto( skipwhite(&URL[7]), enctype, content) );
	}

	/* s‚curit‚ ? mais en fait il faudra trouver une solution
	   car file = 0L est vraiment utilis‚ par get_info() donc
	   ce fichier est vraiment cr‚‚ !! ( mais ou ? )
	*/
	
/*if ( file == 0L ) */
	if ( *file == '\0' )
		file = TMP_FILE;

	request.tcp_handle = -666;
	request.URL = URL;
	request.content = content;
	request.enctype = enctype;
	request.filename = file;
	request.methode = method;
	request.cab_date = lastmodified;
	
	if ( premier_lancement == TRUE )
	{
		*request.stock_URL = '\0';
		*request.last_URL = '\0';
		premier_lancement = FALSE;
	}
	else
	{
		*request.last_URL = '\0';
		strncat( request.last_URL, request.stock_URL, 1024 );
	}
	
	/* On stocke */

	*request.stock_URL = '\0';
	strncat( request.stock_URL, request.URL, 1024);
	p = strchr( request.stock_URL, '?');


	if ( p != NULL )	*p='\0';

	/*
	p = strrchr( last_URL, '/');
	if ( p != NULL )	*(p+1)='\0';
	*/


	LOG_printf("~ Write last URL <%s>.\n", request.stock_URL);
	LOG_printf("~ Read last URL  <%s>.\n", request.last_URL);

	/*
	
	 Sous TOS wwwcache/c/001244.html retourne une erreure, normal,
	 mais pas sous MagiC si wwwcache/c/001244.htm existe.
	 Donc il est pas si simple de savoir si le fichier est d‚j…
	 dans le cache.
	 
	*/

	if ( (test=Fxattr( 0, file, &xattr )) < 0L )
	{
		request.cache_exist = FALSE;
		LOG_printf("~ http_get: File <%s>=<%ld> is not cached\n", file, test);
	}
	else
	{
		request.cache_exist = TRUE;
		LOG_printf("~ http_get: File <%s>=<%ld> is cached\n", file, test);
	}


/*	internal message ----------------------------------------------------------- */

	if ( strnicmp( request.URL, "http://cab.ovl", 14 ) == SUCCESS )
	{
		return_code = SendSetup( &request, &request.URL[15]);
		goto end_no_free_memory;
	}

/*	on/offline ----------------------------------------------------------- */

	if ( config.online == FALSE )
	{
		browser->msg_error(NETWORK_DOWN);
		if ( alert_offline == TRUE )
		{
			ret =  my_form_alerte("Offline", ChaineAlerte(A_OFFLINE) );
		}
		else ret = 1; /* offline */

		if ( ret == 2 )	/* online */
		{
			config.online = TRUE;
		}
		else
		{
			if ( ret == 3 )	alert_offline = FALSE; /* nevermind */
/*		return_code = NETWORK_DOWN;	*//* c'est une valeur de : 	browser->msg_error(); !!! */
			return_code = NETWORK_DOWN;	/* c'est une valeur de : 	browser->msg_error(); !!! */
			goto end_no_free_memory;
		}

	}

/*	split URL ----------------------------------------------------------- */

	return_code = 0L;

	split_URL(URL, &request);
	*request.true_server=0;
	strncat( request.true_server, request.server,255);

	LOG_printf("- Server: <%s>\n", request.true_server);
	LOG_printf("- URI   : <%s>\n", request.URI);

/*	internal icones ----------------------------------------------------------- */

	if ( (bytes_read=SendIcon(request.URI,file)) > 0L )
	{
		return_code = 0;
		goto end_no_free_memory;
	}

/*	header memory ----------------------------------------------------------- */

	/* Pas de m‚moire pour le header */
	
	if( !NewMemBlk(&header,TCP_BUFFER+1,sizeof(UBYTE)) ) /* STiK : 1024 bytes */
	{
		browser->msg_error(NO_MEMORY);
		ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_LOW_MEMORY);
		goto end_no_free_memory;
	}


/*	----------------------------------------------------------- */

	header.used = 0L;

/*	

	name_pass.beg		= name_password;
	name_pass.len		=	MAX_STR;
	name_pass.used	=	strlen( name_password );
	
	authentification.beg=	authentification_coded;
	authentification.len=	2*(MAX_STR);
	authentification.used= strlen( authentification_coded );

*/

Start:

/*	proxy ----------------------------------------------------------- */

	proxy=http_proxy_cfg(http_proxy, 256, &http_proxy_port);

	if (proxy==TRUE)
	{
		LOG_printf("- Proxy : <%s>:(%d)\n", http_proxy, http_proxy_port);
		*request.server=0;
		strncat(request.server, http_proxy, 256);
		request.port=http_proxy_port;
	}

/*

Le probleme : si resolve (open_connection) prend du temps, on se trouve
apres deja en timeout juste apres,
il faudrait le fixer apres le resolve (?)

*/

	timeouts = 1;
	timeout	=clock()+config.timeout*CLK_TCK;
	
	request.tcp_handle = open_connection(request.server, request.port, 0);

	if ( request.tcp_handle < 0 )
	{
		if ( proxy == TRUE )
		{
			split_URL(URL, &request);
			request.tcp_handle = open_connection(request.server, request.port, 0);
			proxy = FALSE;
		}
		
		if ( request.tcp_handle < 0 )
		{
			/* Il faudrait faire en fonction de l'erreure */
			browser->msg_error(CANNOT_RESOLVE);

			/* On ne fait un rapport que si le cache est vide !!! */
			/* En fait le rapport doit etre fait pour presque toutes */
			/* les erreures, sauf can't resolve (deconnect‚) et */
			/* quelques autres... */

#ifdef NOTDEF


	#ifndef NO_ERASE_CACHE
		if ( request.cache_exist == FALSE )
			ReportRequest( &request, X_get_err_text(request.tcp_handle) );
	#endif
			if ( (request.tcp_handle != E_UNREACHABLE) && (request.tcp_handle !=  E_CANTRESOLVE) )
			{
				ReportRequestText( &request, X_get_err_text(request.tcp_handle) );
			}

#endif

retour.value = return_code = EFILNF;	/*EACCDN pas marche?*/
retour.bytes_read = bytes_read = 0L;


			goto end_free_memory;
		}
	}

	ret = Send_Header( &request, proxy );

	if ( ret != 0 )
	{
		browser->msg_error(NETWORK_ERROR);
		ReportRequestText( &request, X_get_err_text( ret ) );
		goto end_free_memory;
	}
		
/*	R‚ception ----------------------------------------------------------- */

	call_cab=clock()+(config.gem/1000)*CLK_TCK;		/* appel CAB toutes les secondes */

	/* ---------------------------------------------------------------------*/
	/*  R‚ception  */
	/* ---------------------------------------------------------------------*/

	request.status = WAITING_HEADER;
	header.used = 0L;
	bytes_read = 0L;
	old_bytes_read = 0L;
	readed = 0;
	no_data = 3;	/* d‚compte imm‚diat pendant l'attente du header */
	request.file_handle = -1;
	eof = 0;
	
	while ( eof == 0 )
	{

		bytes_available=X_CNbyte_count(request.tcp_handle);

		if ( bytes_available < 0  )
			LOG_printf("~ CNbyte_count() returns <%s>\n", X_get_err_text(bytes_available));
	
	/* ---------------------------------------------------------------------*/
	/*  Octets disponibles */
	/* ---------------------------------------------------------------------*/

		if (bytes_available > 0)
		{
		INT take = min ( bytes_available, (INT)(header.len - header.used -1) );

			timeout=clock()+config.timeout*CLK_TCK; /* timeout 60s after last char */

			no_data = 0;

			LOG_printf("~ bytes_available = <%d>, take = <%d> (header.len - header.used) = <%ld>\n", bytes_available, take, header.len - header.used);
			
			if ( (header.used + take) <= TCP_BUFFER ) /* securit‚ */
			{
				readed=X_CNget_block(request.tcp_handle, (UBYTE*)header.beg+header.used, take );
			}
			
			/* Header trop gros pour memdb header */
			
			else
			{
				browser->msg_error(BAD_PACKET);
				ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_BIG_HEADER);
				eof = 2;
			}

			if ( readed != 0 )
		 		LOG_printf("~ X_CNget_block = <%d> bytes read\n", readed);
			
	/* ---------------------------------------------------------------------*/
	/*  Octets lus  */
	/* ---------------------------------------------------------------------*/

			if( readed > 0)
			{

				header.used += readed;
				bytes_read += header.used;

	/* ---------------------------------------------------------------------*/
	/*  R‚ception Entete */
	/* ---------------------------------------------------------------------*/

				if( request.status == WAITING_HEADER )
				{

					request.status = HeaderIsReady( &request, &header);	/* Status HTTP ou perso */

SmallLoop :

					if ( (request.status == 200)
					 || (request.status == 201)
					 || (request.status == 301)
					 || (request.status == 302)
					  )
					{

						/* vider l'ancien fichier cache */

						/* BuG Fcreate converti en lowercase ? */

						request.file_handle=(INT)Fcreate(request.filename, 0);
						
						/* On ne peut pas creer le fichier pour CAB */

						if (request.file_handle < 0 )
						{
							LOG_printf("! Error, request.file_handle = <%d>\n", request.file_handle );
						 /*
						  On essaye EINTRN bien qu'il se fasse pas partie des
						 	erreures. ( il est dans atarierr.h )
						 	A noter que -33 marche.
					 	 */
							browser->msg_error(EINTRN);
							ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_CACHE_FILE);
							eof = 2; /* ? au pif */
						}

						bytes_read = header.used;

						if ( request.content_length > 0L )
						{

/* Pour cabdload faut-t'il le mettre ici ou attendre les premiers bytes ? */

							browser->msg_status(STATUS_DATALENGTH, request.content_length);
							/* Nouveau : pour cabdload ?? */
/*						browser->aes_events(50); */
						}


/*

 dans quel ordre le mettre : ?
 ici c'est pas mal, ca commence tjrs par 0 byte/sec, donc
 pas tres joli non ? 
 
*/
						if ( header.used != 0L )
						{
							/* On ne peut pas ‚crire dans le fichier pour CAB */
							if( (lret=Fwrite(request.file_handle, header.used, (UBYTE*)header.beg)) < 0 )
							{
								LOG_printf("! Fwrite error <%ld>\n", lret);
								browser->msg_error(EINTRN);
								ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_CACHE_WRITE);
								header.used = 0L;
								eof = 2; /* ? au pif */
							}
							header.used = 0L;
						}

					} /* HeaderIsReady WAITING_DATA*/


					else if ( request.status == 304 )
					{
						LOG_printf("~ File not modified\n");
						return_code = 0L;
						eof = 2;
					}

					/* Demande d'authentification */
						
					else if ( request.status == 401 )
					{
						LOG_printf("~ Document protected\n");
						if ( RequestAuthentification( &request ) == TRUE )
						{
							if ( request.file_handle > 0 )
							{
								Fclose(request.file_handle);
								request.file_handle = -1;
							}
							Fdelete(request.filename);
							X_TCP_close(request.tcp_handle, TCP_IMMEDIATE);
							timeouts = 1;
							timeout	=clock()+config.timeout*CLK_TCK;
							goto Start;
						}

						LOG_printf("~ Authentification aborted\n");
						Fdelete( request.filename );
						return_code = EACCDN;
						eof = 2;
					
					}
					
					/* low memory for header */
					
					else if ( request.status == HEADER_LOW_MEMORY )
					{
						browser->msg_error(NO_MEMORY);
						ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_LOW_MEMORY);
						eof = 2;
					}
					
					/* bad header format */
					/*

						Certains serveurs ne renvoi pas de header HTTP :(
					
					*/
					
					else if ( request.status == HEADER_BAD_FORMAT )
					{
						browser->msg_error(USERBREAK);
#ifdef NO_WATCH_HEADER
/* de tres longues URL peuvent crasher la boite de dialogue : */
/*					my_form_alerte( "Protocol ERROR","[1][%s|The document do no contain HTTP header | is very bad server error.|Please contact is administartor.][ Will do ]", URL); */
						LOG_printf("! HTTP missed by server administrator, get raw data.\n");
#else
						ReportRequestIndex( request.filename, &request.file_handle, REPO_BAD_HEADER);
						eof = 2;
#endif
						request.status = 200;
						goto SmallLoop;
					}
					
					/* Autres erreures HTTP */
					
					else if ( request.status != WAITING_HEADER ) /* erreur HTTP */
					{
						if ( *request.content_type == '\0' )
						{
							browser->msg_error(EFILNF);
							ReportRequestText( &request, request.http_line);
							eof = 2;
						}
						else
						{
							request.status = 200;
							goto SmallLoop;
						}
						
					}
				}

	/* ---------------------------------------------------------------------*/
	/*  R‚ception donn‚es  */
	/* ---------------------------------------------------------------------*/

				/* Header pass‚, des donn‚es arrivent */
				
				else
				{
					/* erreur ‚criture fichier temporaire */
					
					if ( (lret=Fwrite(request.file_handle, header.used, (UBYTE*)header.beg)) < 0L )
					{
						LOG_printf("! Fwrite error <%ld>\n", lret);
						browser->msg_error(EINTRN);
						ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_TMP_WRITE);
						eof = 2; /* ? au pif */
					}
					header.used=0L;	/* pret a recevoir */

					if ( ( bytes_read > 0) &&	( (request.status == 200) || (request.status == 201) )	 )
					{
						old_bytes_read = bytes_read;
						if( request.status != WAITING_HEADER  )
						{
							browser->msg_status(STATUS_RECEIVING_DATA, bytes_read);
						}
					}
				}
			}

	/* ---------------------------------------------------------------------*/
	/*  Erreur sur le nombre d'octets lus (readed)  */
	/* ---------------------------------------------------------------------*/

			else if ( readed == E_EOF )	/* 1er test, en fait le EOF n'est pas lu ici  mais plus bas ! */
			{

				LOG_printf("~ X_CNget_block:EOF\n");
#ifdef WATCH_EXPECTED
				if ( ( request.content_length > 0L) && ( bytes_read > 0L ) )
				{
					if ( request.content_length != bytes_read )
					{
						LOG_printf("! error expect <%ld> received <%ld> bytes\n", request.content_length, bytes_read );
						browser->msg_error(BAD_PACKET);
						ReportRequestIndex( request.filename, &request.file_handle, REPO_BAD_NUMBER);
					}
				}
#endif
				eof=1;
			}

			/* STinG retourne une erreur */
			
			else if ( readed < 0 )
			{
				browser->msg_error(NETWORK_ERROR);
				ReportRequestText( &request, X_get_err_text(readed) );
				eof=3;
			}

		}	/* bytes_available > 0 */
		

	/* ---------------------------------------------------------------------*/
	/*  Erreur sur le nombre d'octets disponibles (bytes_available) */
	/* ---------------------------------------------------------------------*/

		else if ( bytes_available == E_RRESET )
		{
			LOG_printf("~ bytes_available==E_RRESET, do nothing..\n");

				/*
				 => ca ne resoud pas le probleme sur certains
				 serveurs, mais c'est bien de ne rien faire.
			 */
		}
		else if ( bytes_available == E_EOF )	/* Le test est toujours ici ! */
		{
			LOG_printf("~ bytes_available:EOF\n");

			if ( (bytes_read > old_bytes_read) && ( bytes_read > 0) && ((request.status == 200) || (request.status == 201)) )
			{
				browser->msg_status(STATUS_RECEIVING_DATA, bytes_read);
			}

#ifdef WATCH_EXPECTED
			if ( ( request.content_length > 0L) && ( bytes_read > 0L ) )
			{
				if ( request.content_length != bytes_read )
				{
					LOG_printf("! error expect <%ld> received <%ld> bytes\n", request.content_length, bytes_read );
					browser->msg_error(BAD_PACKET);
					ReportRequestIndex( request.filename, &request.file_handle, REPO_BAD_NUMBER);
				}
			}
#endif
			eof=1;
		}

		/* Erreur sur la connexion ex:refus‚ */
		
		else if ( (bytes_available != E_NODATA) && ( bytes_available != E_NORMAL ))
		{
			browser->msg_error(NETWORK_ERROR);
			bytes_available=ReportRequestText( &request, X_get_err_text(bytes_available) );
			eof=3;
		}


	/* ---------------------------------------------------------------------*/
	/*  Proc‚dures de timer */
	/* ---------------------------------------------------------------------*/

	/* ************ Temps pour CAB + touche STOP + touche ESC ************* */

		if ( clock() > call_cab )
		{

			if( (sting == TRUE) && (bytes_available != 0) )
				goto Step1;

			if ( browser->aes_events(config.timer) == -1L )	/* bouton STOP ou touche Esc */
			{
				LOG_printf("! aes_events : STOP icon or Esc key pressed\n");
				/* s'affiche pas :( (toujours ) */
				browser->msg_error(USERBREAK); /* histoire de donner quelque chose */
				if ( request.file_handle > 0 )
				{
					Fclose( request.file_handle );
					request.file_handle = -1;
				}
				Fdelete( request.filename );
				bytes_read = 0;
				return_code = ERROR;
				eof=4;
			}

			call_cab=clock()+(config.gem/1000)*CLK_TCK;		/* appel CAB toutes les secondes */

			if ( (no_data == 3) && (eof==0) )
			{
				browser->msg_status(STATUS_WAITINGFORDATA,(timeout-clock())/200);
			}
			else if ( no_data < 3 )				no_data ++;

		}

Step1:

	/* ***************************** Timeout ****************************** */
		
		if ( clock() > timeout )
		{
			LOG_printf("~ Timeout!\n");
			browser->msg_error(CONNECTION_FAILED);
			ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_NO_REPONSE);
			eof=4;
		}

	} /* while eof == 0 */


/*	TCP_close ----------------------------------------------------------- */


	/* ---------------------------------------------------------------------*/
	/*  Fin de boucle de r‚ception  */
	/* ---------------------------------------------------------------------*/

	/* ----- Fermeture de la connexion ----- */
	
	/* timeout or worse*/
	/*	if (eof>1) on ferme quand meme ! */
	
	if ( request.tcp_handle != -666 )
	{
		x = (int16)X_TCP_close(request.tcp_handle, TCP_IMMEDIATE);
		LOG_printf("~ TCP_close() returns <%s>\n", X_get_err_text(x));
	}

/*	File close ----------------------------------------------------------- */

	/* fixer le date du fichier NE RIEN ECRIRE APRES !! */

	if( request.file_handle > 0 )
	{
		if (request.last_modified > 0	)
		{

		/* Parceque GEMDOS ne gere que des nombre paire de secondes
		on incremente d'une seconde la date du fichier*/

			request.last_modified++;	/* TOS is 2 sec based */
			
			urltime=localtime(&request.last_modified);/* gmtime corrige GMT (-1h) */
		}
		else
			urltime=localtime(&request.date);
	
		filetime.time=(urltime->tm_sec/2);
		filetime.time+=(urltime->tm_min)<<5;
		filetime.time+=(urltime->tm_hour)<<11;
		filetime.date=(urltime->tm_mday);
		filetime.date+=((urltime->tm_mon)+1)<<5;
		filetime.date+=((urltime->tm_year)-80)<<9;
							
		Fdatime(&filetime, request.file_handle, 1);
		
		x = Fclose(request.file_handle);
		
		if( x < 0 )
		{
			/* Normalement impossible car erreur donc on essaye: */
			request.file_handle = -1;
			browser->msg_error(EINTRN);
			ReportRequestIndex( &request, request.filename, &request.file_handle, REPO_TMP_CLOSE);
		}
		else /* Bien ferm‚ */
		{
			request.file_handle = -1;
		}
	}

/*	Free memory ----------------------------------------------------------- */

end_free_memory:

	FreeMemBlk(&header);

/*	Java ----------------------------------------------------------- */

#ifdef _RUN_JAVA_

	RunJavaPlugIn( request.filename );

#endif

/*	Location ----------------------------------------------------------- */

	/* --------------------------------------------------------------------------*/
	/* Lancer une autre location si l'url est renvoy‚e !! tout lib‚rer avant ici */
	/* --------------------------------------------------------------------------*/

/*
 !! Si request.location ne commence pas par http:// il faut l'ajouter
*/
	
	if ( (strlen(request.location)>0) && (eof == 1))	/* Pas d'erreur */
	{
		*retour.location = '\0';
		if ( strnicmp( request.location, "http://", 7) != SUCCESS )
		{
			strncat ( retour.location, "http://", 255);
			strncat ( retour.location, request.true_server, 255);
			if ( request.location[0] != '/' )
				strncat ( retour.location, "/", 255);
		}
		strncat ( retour.location, request.location, 255);
		LOG_printf("~ New Location <");
		LOG_puts("~", retour.location);
		LOG_puts("~", ">\n");

		/* Il faut faire quelque chose si CAB ne donne pas de fichier ! */

		if (browser->new_url( retour.location, &new_file_ptr)==0)
		{

			*retour.file = '\0';
			strncat(retour.file, new_file_ptr,1023);

			if ( *retour.file == '\0' )	/* S‚curit‚ */
			{
				strncat(retour.file, TMP_FILE,1023);			
			}
		}
		else
		{
			LOG_printf("! browser can not give a new filename, set as old <%s>\n", retour.file);
			Fdelete(request.filename);
		}

		retour.redirection = TRUE;
		
	}
	else
	{
		retour.redirection = FALSE;
	}

/*	End ----------------------------------------------------------- */
	
end_no_free_memory:

	if ( return_code != 0 ) /* si il y a eu probleme, on efface le cache */
		Clear_Last_Host();

	ClearCookie();

/*

Avec une redirection on repasse par la....hummm pas bien !!

ex : 

! 15 Nov 02:33:58 OVL  Run PLUG-IN <U:\INTERNET\CAB\modules\plug-in\java.ovl> error <-34>
> 15 Nov 02:33:58 OVL  ClearCookie
~ 15 Nov 02:33:58 OVL  END : return_code (0) for file contain (1331) bytes
> 15 Nov 02:33:58 OVL  ClearCookie
~ 15 Nov 02:33:58 OVL  END : return_code (0) for file contain (302) bytes
> 15 Nov 02:33:58 OVL  ClearCookie
~ 15 Nov 02:33:58 OVL  END : return_code (0) for file contain (275) bytes

Ca peut etre un danger sur deux redirection avec browser->new_url() ??

=> OUI si le fichier est deja dans le cache

*/


	retour.value = return_code;
	retour.bytes_read = bytes_read;

/* 0 si ok, sinon errno */

return( return_code );
}

/* ---------------------------------------------------------------------- */



/** Resolves server into an IP address, then opens a connection to port
	of server, with TOS (magic variable, which should always be 0, ask
	Steve Adam about it). It returns a connection handle for following
	calls. If it's smaller than 0 then it's an error. */

LOCAL int16 open_connection(char *server, int port, int TOS)
{
int16 x, cn;
uint32 addr;
	
	LOG_printf("> open_connection server <%s> port <%i> TOS <%i>\n", server, port, TOS);

	x = my_resolver( server, &addr );
	if (x<0)
	{
		LOG_printf("! Resolve <%s> returns <%s>\n", server, X_get_err_text(x));
		return(x);
	}

	browser->msg_status(STATUS_CONNECTING_HOST, 0);
	
	cn=X_TCP_open(addr, port, TOS, TCP_BUFFER);

	if ( iconnect == TRUE )
		browser->msg_status(STATUS_CONNECTING_HOST, 0);

	if (cn<0)
	{
		LOG_printf("! TCP_open <%s> returns <%s>\n", server, X_get_err_text(cn));
		Clear_Last_Host();
		return(cn);
	}

#ifdef USE_TCP_WAIT_STATE	/* on a des blocages */

	x = X_TCP_wait_state(cn, TESTABLISH, 1);
	if (x<0)
	{
		LOG_printf("~ TCP_wait_state() returns: %s\n", X_get_err_text(x));
		X_TCP_close(cn, TCP_IMMEDIATE);
		return(x);
	}

#endif

return(cn);
}

/* ---          EOF                 --- */
