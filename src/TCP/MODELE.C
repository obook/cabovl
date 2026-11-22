/*
 * modele.c
 * ModŠle de fichier pour une nouvelle interface TCP/IP.
 *
 * Absent du fichier projet CABOVL.PRJ.
 *
 * Utilise : DRACONIS.H, GLOBALS.H
 *
 * Cr‚ation : 02/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<portab.h>

#include "globals.h"	

/* ---       Includes internes       -- */

#include "draconis.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

int16 DRACONIS_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize)

int16 DRACONIS_TCP_wait_state(int16 cn, int16 state, int16 timeout )

int16 DRACONIS_TCP_send(int16 cn, char *buf, int16 len)

int16 DRACONIS_TCP_close(int16 cn, int16 timeout)

char  *DRACONIS_get_err_text(int16 code)

int16 DRACONIS_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize)

int16 DRACONIS_CNbyte_count(int16 cn)

int16 DRACONIS_CNget_block(int16 cn, char *blk, int16 len)

char *DRACONIS_getvstr(char *variable )

/* ---          EOF                 --- */
