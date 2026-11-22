/*
 * my_head.c
 * Construction de l'en-tàte de la requàte HTTP (Send_Header).
 *
 * êcrit la ligne de requàte et les champs Host, User-Agent,
 * Accept, Referer, Cookie et Authorization, avec ou sans proxy.
 *
 * Utilise : AUTHEN.H, CONFIG.H, COOKIE.H, GLOBALS.H, HTTP.H, MAIN.H,
 *   RSC.H, TCP.H, TOOLS.H
 * UtilisÇ par : HTTP_GET.C
 *
 * CrÇation : 25/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ultÇrieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<errno.h>
#include	<portab.h>
#include	<upoint\memory.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"main.h"
#include	"authen.h"
#include	"config.h"
#include	"http.h"
#include	"tcp.h"
#include	"cookie.h"
#include	"rsc.h"
#include	"tools.h"	/* Strftime */

/* ---       Includes internes       -- */

#include "my_head.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL UBYTE my_header[2048];

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */


INT Send_Header( REQUEST	*request, BOOLEAN proxy )
{
INT retour = 0;
UBYTE date[51];

	LOG_printf("~ Creating header...\n");
	
	Strftime(date,50,"%a, %d %b %Y %X GMT",localtime(&((time_t)(request->cab_date))));

	*my_header = '\0';

	if (request->methode == GET_METHOD)
		strncat( my_header, "GET ", 2047 );
	else if (request->methode == POST_METHOD)
		strncat( my_header, "POST ", 2047 );
	else
		strncat( my_header, "HEAD ", 2047 );
	
/*

Nouveau : transcodages des espages, mais on DETRUIT URL !!
Il va falloir trouver une autre methode

*/

	if (proxy==TRUE)	/* ?? SUR OUI !! */
	{
	UBYTE *token = strtok(request->URL, " ");
		while(token)
		{
			strncat(my_header, token, 2047);
			token = strtok(NULL, " ");
			if ( token != NULL ) /* pas fin de chaine */
				strncat(my_header, "%20", 2047);
		}
	}
	else
	{
	UBYTE *token = strtok(request->URI, " ");
		while(token)
		{
			strncat(my_header, token, 2047);
			token = strtok(NULL, " ");
			if ( token != NULL ) /* pas fin de chaine */
				strncat(my_header, "%20", 2047);
		}
	}

	strncat( my_header, " HTTP/1.0\r\n", 2047 );

	strncat( my_header, "Host: ", 2047 );
	strncat( my_header, request->true_server, 2047 );
	strncat( my_header, "\r\n", 2047 );

	strncat( my_header, "Pragma: no-cache\r\n", 2047 );
	strncat( my_header, "Cache-Control: no-cache\r\n", 2047 );

	strncat( my_header, "User-Agent: ", 2047 );
	strncat( my_header, user_agent, 2047 );
	strncat( my_header, "\r\n", 2047 );

	strncat( my_header, "Accept: */*\r\n", 2047 );

	if ( ( request->cab_date > 42L ) && ( *(browser->reloadflag) == 0 ) )
	{
		strncat( my_header, "If-Modified-Since: ", 2047 );
		strncat( my_header, date, 2047 );
		strncat( my_header, "\r\n", 2047 );
	}

/*
 C'est ici que l'on devrait envoyer le "Referer:" ?
 si le serveur est le màme => stocker last URL
 
 Ca marche pour free
 
*/
	if (request->methode == POST_METHOD)
	{
	UBYTE buffer[10];

		strncat( my_header, "Content-Length: ", 2047 );
		strncat( my_header, ltoa(strlen(request->content), buffer,10), 2047 );
		strncat( my_header, "\r\n", 2047 );

		strncat( my_header, "Content-type: ", 2047 );
		strncat( my_header, request->enctype, 2047 );
		strncat( my_header, "\r\n", 2047 );
	}

	/* Nouveau, Referer est valable tout le temps */

	if( *request->last_URL != '\0' )
	{
		strncat( my_header, "Referer: ", 2047 );
		strncat( my_header, request->last_URL, 2047 );
		strncat( my_header, "\r\n", 2047 );
	}

	if ( strcmp( authentification_server, request->true_server) == SUCCESS )
	{
	/*
	Le module de Dan donne le màme codage, seul de basic est avec
	une majuscule... c'est ce qui pose probleme ?! ca m'Çtonnerait.
	*/
/*	strncat( my_header, "Authorization: basic ", 2047 ); */
		strncat( my_header, "Authorization: Basic ", 2047 );
		strncat( my_header, authentification_coded, 2047 );
		strncat( my_header, "\r\n", 2047 );
	}

	/* Recherche et envoi de cookie si cab_version < 0x30323730L */
	
	SendCookie( request->true_server , request->URI, my_header, 2047);

	strncat( my_header, "\r\n", 2047 );	/* Ligne Vide */
		
	LOG_printf("~ Send my header: <\n");
	
	/* erreur Çcriture LOG ici avec colorado : trop long ?*/

	/*
		OUI mais pour CAB, l'URL fait plus de 81 caractere a cause des
		paramätres, il faut prendre une version supÇrieure de CAB
		pour rÇsoudre le probläme, mais le max == ??
	*/
	
	LOG_puts("~", my_header);
	LOG_puts("~", ">\n");

/* A partir d'ici il faut refermer TCP */

	if ( iconnect == TRUE )
		browser->msg_status(STATUS_SENDING_REQUEST,0);

/*browser->aes_events(50);*/

	retour = X_TCP_send( request->tcp_handle, my_header, (int16) strlen(my_header) );

	if ( iconnect == TRUE )
		browser->msg_status(STATUS_WAITING_RESPONSE,0);

/*browser->msg_status(STATUS_WAITINGFORDATA,(timeout-clock())/200); */

	if ( retour != E_NORMAL )
	{
		LOG_printf("! TCP_send() to <%s> returns: <%s>\n", request->server, X_get_err_text(retour));
		X_TCP_close(request->tcp_handle,1);
		return(retour);
	}

	if (request->methode == POST_METHOD)
	{
	int16 len = (int16) strlen(request->content);
	
		LOG_printf("~ Sending form <%s>, <%d> bytes using the POST method : <\n%s>\n", NettoyerChaine(request->enctype), len, request->content);

		if( (retour=X_TCP_send(request->tcp_handle, request->content, len) ) < 0 )
		{
			LOG_printf("! TCP_send() to <%s> returns: <%s>\n", request->server, X_get_err_text(retour));
			X_TCP_close(request->tcp_handle,1);
			return(retour);
		}
	}

return( retour );
}

/* ---          EOF                 --- */
