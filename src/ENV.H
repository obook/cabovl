/*
 * env.h
 * Prototypes de ENV.C.
 *
 * Interface de ENV.C.
 *
 * Utilis‚ par : ENV.C, EXEC.C
 *
 * Cr‚ation : 01/01/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

UBYTE * NewEnv(LONG size);
BOOLEAN PutEnv(UBYTE *buffer);
BOOLEAN FreeEnv(VOID);

/* ---          EOF                 --- */
