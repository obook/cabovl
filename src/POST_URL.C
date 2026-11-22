/*
 * post_url.c
 * Point d'entr‚e post de CAB : envoie un formulaire (POST).
 *
 * Appelle http_get avec la m‚thode POST.
 *
 * Utilise : GET_URL.H, GLOBALS.H, HTTP.H, HTTP_GET.H
 * Utilis‚ par : COOKIE.C, HTTP_GET.C, MAIN.C
 *
 * Cr‚ation : 11/06/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"http.h"
#include	"http_get.h"

/* ---       Includes internes       -- */

#include "get_url.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

/* ----------------------------------------------------------------- ** 
** post 	 - Post FORM, fetch result and write it as a HTML file		 ** 
**					 Returns 0 if OK, else `errno'													 **
** ----------------------------------------------------------------- */
/* URL to fetch */
/* data to post */
/* format of data */
/* file to write to */

long ___CDECL post(char *url, char *content, char *enctype, char *filename)
{
	LOG_printf("- ***************************************************\n");
	LOG_printf("> post \n");

return(http_get(url, content, enctype, filename, POST_METHOD,0L));
}


/* ---          EOF                 --- */
