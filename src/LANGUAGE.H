/*
 * language.h
 * Prototypes du chargement de langue.
 *
 * Interface de LANGUAGE.C.
 *
 * Utilis‚ par : LANGUAGE.C, MAIN.C
 *
 * Cr‚ation : 05/08/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

WORD	load_language ( UBYTE *name );
WORD	Give_languages ( UBYTE *name , UBYTE *buffer, INT limit);

/* ---          EOF                 --- */
