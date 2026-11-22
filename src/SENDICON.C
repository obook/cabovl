/*
 * sendicon.c
 * Envoi des ic“nes internes … CAB.
 *
 * R‚pond aux URL des ic“nes internes (back, text, image, ...)
 * avec les donn‚es compil‚es dans ICONS.LIB.
 *
 * Utilise : GLOBALS.H, ICONS.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 23/03/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/*------------------------------------------------
//
// Module: sendicon.c
//
// Objet : envoi de fichiers internes (icones)
//
// ToDo  : faire last-modified, puis assurer le HEAD
//
//
//
// Maintenance :
//
//     Auteur  : Olivier Booklage
//     Version : V1.0
//     Date    : 23/03/1998
//     Remarq. :
//
//
//
//-----------------------------------------------*/


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>

#include	"globals.h"
#include	"icons.h"

/* ---       Includes internes       -- */

#include "sendicon.h"	

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */
														
/* ---     Prototypage interne      --- */

LOCAL LONG GetIcon( UBYTE *filename, UBYTE *data, INT size);

/* ---         Fonctions            --- */

ULONG SendIcon(UBYTE *url, UBYTE *file)
{

	LOG_printf("> SendIcon : Scan for <%s>\n",url);

	/* faut-t'il comparer la fin ? */

	if ( strcmp(url,"/internal-gopher-back") == SUCCESS )
		return( GetIcon( file, back_xbm, size_back_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-binary") == SUCCESS )
		return( GetIcon( file, binary_xbm, size_binary_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-blank") == SUCCESS )
		return( GetIcon( file, blank_xbm, size_blank_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-image") == SUCCESS )
		return( GetIcon( file, image_xbm, size_image_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-index") == SUCCESS )
		return( GetIcon( file, index_xbm, size_index_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-menu") == SUCCESS )
		return( GetIcon( file, menu_xbm, size_menu_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-movie") == SUCCESS )
		return( GetIcon( file, movie_xbm, size_movie_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-sound") == SUCCESS )
		return( GetIcon( file, sound_xbm, size_sound_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-text") == SUCCESS )
		return( GetIcon( file, text_xbm, size_text_xbm) );

	else
	if ( strcmp(url,"/internal-gopher-unknown") == SUCCESS )
		return( GetIcon( file, unknown_xbm, size_unknown_xbm) );


return(0L);	/* rien trouv‚ */
}

LOCAL LONG GetIcon( UBYTE *filename, UBYTE *data, INT size)
{
FILE *file_ptr;

	LOG_printf("> GetIcon : Send %s\n",filename);
	
	file_ptr = fopen( filename,"w");
	
	if ( file_ptr == NULL )
	{
		LOG_printf("! GetIcon : Can not fopen <%s>\n",filename);
	 return(-1L);
	}
	
	fwrite( data, sizeof(UBYTE), size, file_ptr);	
	
	fclose(file_ptr);

return((LONG)size);
}

/* ---          EOF                 --- */
