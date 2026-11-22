/*
 * cache_ip.h
 * Prototypes du cache de r‚solution.
 *
 * Interface de CACHE_IP.C.
 *
 * Utilis‚ par : CACHE_IP.C, HTTP_GET.C
 *
 * Cr‚ation : 13/10/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

int16 my_resolver(UBYTE *server, uint32 *addr);
VOID Clear_Last_Host( VOID );

/* ---          EOF                 --- */
