/*
 * mime.h
 * Prototypes de la table des types MIME.
 *
 * Chargement du fichier MIME et recherche par extension.
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

WORD		parser_mime_description(BYTE *filename,BYTE *ext,BYTE *type,BYTE *description);
BOOLEAN LoadMIME(UBYTE *filename);
VOID		UnLoadMIME(VOID);
BYTE		*MimeDescription(UBYTE *ext);
UBYTE		*MimeContent_type(UBYTE *ext);
UBYTE		*GetFirstMIME(UBYTE *string);
UBYTE		*GetNextMIME(UBYTE *string);

/* Outils */

void ProcessLine(BYTE *line, BYTE *s, BYTE *p1, BYTE *p2, LONG limit);

/* ---          EOF                 --- */
