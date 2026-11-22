/*
 * get_url.c
 * Point d'entr‚e get_url de CAB : charge une URL dans un fichier.
 *
 * Appelle http_get avec la m‚thode GET.
 *
 * Utilise : GLOBALS.H, HTTP.H, HTTP_GET.H
 * Utilis‚ par : COOKIE.C, HTTP_GET.C, MAIN.C, POST_URL.C
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
** get_url - Fetch URL and write it as a HTML file									 ** 
**					 Returns 0 if OK, else `errno'													 **
** ----------------------------------------------------------------- */
long ___CDECL get_url(char *url, char *filename)
{
	LOG_printf("- ***************************************************\n");
	LOG_printf("> get_url\n");

return(http_get(url, "", "", filename, GET_METHOD,0L));
}

/* ---          EOF                 --- */
