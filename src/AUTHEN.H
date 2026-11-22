/*
 * authen.h
 * Prototype de RequestAuthentification.
 *
 * Interface de AUTHEN.C.
 *
 * Utilis‚ par : AUTHEN.C, HTTP_GET.C, MAIN.C, MY_HEAD.C, RSC.C
 *
 * Cr‚ation : 15/10/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

EXTERN UBYTE name_password[];
EXTERN UBYTE authentification_server[];
EXTERN UBYTE authentification_coded[];	/* ? Il faudra faire + long pour le codage */

/* ---    Export Prototypage        --- */

BOOLEAN RequestAuthentification( REQUEST *request );

/* ---          EOF                 --- */
