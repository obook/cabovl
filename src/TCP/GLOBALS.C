/*
 * globals.c
 * Variables partag‚es entre les interfaces TCP/IP.
 *
 * D‚finit buffer et transport, d‚clar‚s dans GLOBALS.H.
 *
 * Utilis‚ par : DRACONIS.C, ICONNECT.C, MINTNET.C, MODELE.C
 *
 * Cr‚ation : 01/01/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<portab.h>


/* ---       Includes internes       -- */

#include	"globals.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

char buffer[5001];
UBYTE *transport = "";

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

VOID trash( void *fmt , ... )
{

if ( fmt )	return;

}

/* ---          EOF                 --- */
