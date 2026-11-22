/*
 * tools.h
 * Prototypes des fonctions utilitaires.
 *
 * Interface de TOOLS.C.
 *
 * Utilis‚ par : COOKIE.C, EXEC.C, HEADER.C, HTTP_GET.C, LANGUAGE.C,
 *   MAILTO.C, MAIN.C, MY_HEAD.C, REPORT.C, SETUP.C, TCP.C, TOOLS.C
 *
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

VOID split_URL(UBYTE *URL, REQUEST *request);
VOID clear_info(REQUEST *request);

#ifdef __HTTP__
proxy_url	*fix_proxy( proxy_url	*proxy );
#endif

BOOLEAN http_proxy_cfg(char *ps, int pslen, int *pp);
int ftp_proxy_cfg(char *ps, int pslen, int *pp);

time_t parse_date(char *date_string);
UBYTE *dans_un_ans( VOID );
UBYTE *skipwhite ( const UBYTE *src );	/* saute les espaces */
UBYTE *clearwhite ( const UBYTE *src );	/* saute espace+ enleve trailing < " " */
UBYTE *VersionCABString( LONG version );
UBYTE *VersionOVLString( LONG version );

VOID trash( void *fmt , ... );

size_t Strftime(char *s, size_t max_size, const char *format, const struct tm *timeptr);

/* ---          EOF                 --- */
