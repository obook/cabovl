/*
 * header.h
 * Prototypes de l'analyse d'en-tˆte.
 *
 * Interface de HEADER.C.
 *
 * Utilis‚ par : HEADER.C, HTTP_GET.C
 *
 * Cr‚ation : 17/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* Valeur retourn‚ par HeaderIsReady ou status HTTP */

#define WAITING_HEADER	       			 1	/* header pas arriv‚ */
#define HEADER_EMPTY								-1	/* pas de donn‚e dans memdb */
#define HEADER_LOW_MEMORY						-2	/* pas assez de m‚moire */
#define HEADER_BAD_FORMAT						-3	/* pas de header correcte trouv‚ dans les TCP_BUFFER bytes du buffer*/

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

WORD HeaderIsReady( REQUEST *request, memdb *header);
int parse_header( REQUEST *request ,txtdb *textlst );
LONG IsHeadline ( const char *s, const txtdb *list );

/* ---          EOF                 --- */
