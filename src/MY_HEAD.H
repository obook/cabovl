/*
 * my_head.h
 * Prototype de Send_Header.
 *
 * Interface de MY_HEAD.C.
 *
 * Utilis‚ par : HTTP_GET.C, MY_HEAD.C
 *
 * Cr‚ation : 25/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

/* 

 Retourne la valeur retourn‚e par TCP_send 
 
*/

INT Send_Header( REQUEST	*request, BOOLEAN proxy );

/* ---          EOF                 --- */
