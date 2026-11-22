/*
 * main.h
 * Prototypes export‚s par MAIN.C.
 *
 * D‚clare user_agent, browser (informations fournies par CAB) et
 * les points d'entr‚e du module : restore_module, get_url_info,
 * get_version, mailto.
 *
 * Utilis‚ par : AUTHEN.C, CACHE_IP.C, COOKIE.C, HTTP_GET.C, MAILTO.C,
 *   MAIN.C, MY_HEAD.C, REPORT.C, RSC.C, SETUP.C, TOOLS.C
 *
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

EXTERN char user_agent[];
EXTERN browser_info_t *browser;

/* ---    Export Prototypage        --- */

void ___CDECL restore_module(void);
long ___CDECL get_url_info(char *url, long *timep, long *sizep, char *type);
void ___CDECL get_version(char **authorp, long *versionp, long *datep);

/* Nouveau, … faire */
long ___CDECL mailto(char *url, char *subject, char *filename);

/* ---          EOF                 --- */
