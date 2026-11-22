/*
 * http_get.h
 * Prototype de http_get.
 *
 * Interface de HTTP_GET.C, appel‚e par les points d'entr‚e de CAB.
 *
 * Utilis‚ par : COOKIE.C, GET_INFO.C, GET_URL.C, HTTP_GET.C,
 *   IF_MODIF.C, POST_URL.C
 *
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

LONG http_get(char *URL, char *content, char *enctype, char *file, int method, long lastmodified);

/* ---          EOF                 --- */
