/*
 * globals.h
 * Variables partag‚es entre les interfaces TCP/IP.
 *
 * D‚clare buffer et transport.
 *
 * Utilis‚ par : DRACONIS.C, GLOBALS.C, ICONNECT.C, MINTNET.C, MODELE.C
 *
 * Cr‚ation : 01/01/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

EXTERN UBYTE buffer[];
EXTERN UBYTE *transport;

/* ---    Export Prototypage        --- */

VOID trash( VOID *fmt , ... );

/* ---          EOF                 --- */
