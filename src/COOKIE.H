/*
 * cookie.h
 * Prototypes de la gestion des cookies.
 *
 * Interface de COOKIE.C.
 *
 * Utilis‚ par : COOKIE.C, HEADER.C, HTTP_GET.C, MAIN.C, MY_HEAD.C,
 *   TOOLS.C
 *
 * Cr‚ation : 14/07/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

EXTERN	UBYTE *cookie_str;	/* Si non null, envoyer la valeur */
EXTERN	UBYTE *cookie_new_location_cookie_str;	/* s'en servir en cas de new location puis le rendre NULL */

/* ---    Export Prototypage        --- */

BOOLEAN AddCookie( REQUEST *request, UBYTE *line );
BOOLEAN SendCookie( UBYTE *domain , UBYTE *path, UBYTE *buffer, int limit);
VOID		ClearCookie( VOID );
BOOLEAN CheckCookieDate( VOID );

/* ___CDECL est dans cab/module.h */

long ___CDECL get_url_cookie(char *url, char *filename, char *cookie);
long ___CDECL post_cookie(char *url, char *content, char *enctype, char *filename, char *cookie);
long ___CDECL get_url_ifmod_cookie(char *url, char *filename, long *time, char *cookie);

/* ---          EOF                 --- */
