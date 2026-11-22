/*
 * if_modif.c
 * Point d'entr‚e get_url_if_modified de CAB.
 *
 * Charge une URL seulement si elle a chang‚ depuis la date
 * donn‚e (If-Modified-Since).
 *
 * Utilise : GLOBALS.H, HTTP.H, HTTP_GET.H
 * Utilis‚ par : COOKIE.C, MAIN.C
 *
 * Cr‚ation : 11/06/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<time.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"http.h"
#include	"http_get.h"

/* ---       Includes internes       -- */

#include "if_modif.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

long ___CDECL get_url_if_modified(char *url, char *filename, long *thetime)
{
/*UBYTE date[51];*/

	LOG_printf("- ***************************************************\n");
	LOG_printf("> get_url_if_modified\n");
	
	/* Sun, 06 Nov 1994 08:49:37 GMT    RFC 822, updated by RFC 1123 */
	/*strftime(date,50,"%a, %d %b %Y %X %Z",localtime(&((time_t)(*thetime))));*/
	
return(http_get(url, "", "", filename, GET_METHOD, *thetime));
}

/* ---          EOF                 --- */
