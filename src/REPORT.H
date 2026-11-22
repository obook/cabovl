/*
 * report.h
 * Prototypes des rapports d'erreur.
 *
 * Interface de REPORT.C.
 *
 * Utilis‚ par : HTTP_GET.C, MAILTO.C, REPORT.C, SETUP.C
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

INT ReportRequestIndex( REQUEST *request, UBYTE *file_name, INT *file_handle, INT rsc_index);
INT ReportRequestText( REQUEST *request, UBYTE *texte);
INT ReportRequestCode( REQUEST *request, INT code);

INT ReportRscMessage( UBYTE *file_name, INT *file_handle, INT index);
INT ReportText( UBYTE *URL, UBYTE *file_name, INT *file_handle, UBYTE *texte);

/* ---          EOF                 --- */
