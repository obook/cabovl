/*
 * get_info.c
 * Point d'entr‚e get_url_info de CAB : informations sur une URL.
 *
 * Renvoie la date, la taille et le type d'un document (requˆte HEAD).
 *
 * Utilise : GLOBALS.H, HTTP.H, HTTP_GET.H, TCP.H
 * Utilis‚ par : MAIN.C
 *
 * Cr‚ation : 12/08/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/*------------------------------------------------
//
// Module: get_info.c
//
// Objet : 
//
// ToDo  : plante avec mon nouvel OVL...
//
//
//
// Maintenance :
//
//     Auteur  : Olivier Booklage
//     Version : V1.0
//     Date    : 12/08/1999
//     Remarq. :
//
//
//
//-----------------------------------------------*/


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"tcp.h"
#include	"http.h"
#include	"http_get.h"

/* ---       Includes internes       -- */

#include "get_info.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

EXTERN REQUEST	request;

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

/* ----------------------------------------------------------------- **
** get_url_info - Retreive infos for an URL 												 **
**								Returns 0 if OK, else `errno' 										 **
** ----------------------------------------------------------------- */

/* URL */
/* UNIX time */
/* size of data */
/* mime type (max len = 250), empty string if unknow */

long ___CDECL get_url_info(char *url, long *timep, long *sizep, char *type)
{
LONG ret=-1L;

	LOG_printf("- ***************************************************\n");
	LOG_printf("> get_url_info\n",url,timep,sizep,type);

#ifdef notdef


/*ret = http_get( url, "", "", 0L, HEAD_METHOD, 0L);*/
	ret = http_get( url, "", "", "", HEAD_METHOD, 0L);
	if ( ret == 0L )	/* pas d'erreur */
	{
		memcpy( &timep, &request.last_modified, sizeof(time_t) );
		memcpy( &sizep, &request.content_length, sizeof(LONG) );
		*type = '\0';
		strncat( type, request.content_type, 249);
	}

/*alerte(1,ALERTE_FUNC_NOT_IMPLEMENTED);*/

#endif

return (ret);
}
/* ---          EOF                 --- */
