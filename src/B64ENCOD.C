/*
 * b64encod.c
 * Codage d'un fichier en base64 (tobase64).
 *
 * Sert … joindre un fichier … un message.
 *
 * Utilise : GLOBALS.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 25/09/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/*------------------------------------------------
//
// Module: b64encod.c
//
// Objet : Codage de fichier attach‚ en BASE64
//
// ToDo  : 	Ajuster netmail->filename
//					Faire view
//
//
// Maintenance :
//
//	 	Auteur	: Olivier Booklage
//	 	Version : V1.0
//	 	Date		: 25/09/1996
//	 	Remarq. :
//
//
//-----------------------------------------------*/


/* ---		 Includes externes		 -- */

#include	<tos.h>
#include	<string.h>
#include	<upoint\memory.h>
#include	<portab.h>

#include	<logfile.h>
#include	"globals.h"

/* ---		 Includes internes		 -- */

#include	"b64encod.h"

/* --- Constantes globales internes --- */
/* --- Structures globales internes --- */
/* --- Variables globales  internes --- */

char base64set[64] = {

 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', 	/* x00 */
 'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f', 	/* x10 */
 'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v', 	/* x20 */
 'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/' }; /* x30 */

/* ---	 	Prototypage interne		--- */

/* ---			 Fonctions			--- */


/* CE codage est correct,son d‚codage ne l'est pas
si pas de CR (seul ) a la fin des lignes de cars */

INT tobase64 ( memdb *mem , memdb *dest )
{
int 	i = 0, j = 0;
long	s;
char *c = (char *)mem->beg;
char *e = (char *)mem->beg + mem->used;
char	out[80];

	LOG_printf("> tobase64\n");
	
	if ( e <= c ) 	return FALSE;

	while ( c < e )
	{

		memset ( out, (int)'=', sizeof (out));
		i = 0;
		j = 0;

		while ( ++j < 20 && c < e )
		{

			int k = 0;

			s	 = *c++;		/* 3 Sourcebytes */
			s <<= 8;

			if ( c < e )
			{
				s	+= *c++;
			  k++;
			}

			s <<= 8;

			if ( c < e )
			{
			  s	+= *c++;
			  k++;
			}

			out[i++] = base64set [( s & 0x00fc0000L ) >> 18];
			out[i++] = base64set [( s & 0x0003f000L ) >> 12];

			if ( k >= 1 )
			  out[i++] = base64set [( s & 0x00000fc0L ) >>  6];
			else
				out[i++] = '=';

			if ( k == 2 )
			  out[i++] = base64set [( s & 0x0000003fL )];
			else
			  out[i++] = '=';
		}

/*	out[i++] = '\n';	*/ /* <= retour a la ligne ?! */
		out[i++] = '\0';
		
		if( ( dest->used + i ) < dest->len )
		{
			memcpy( (UBYTE*)dest->beg + dest->used, out, i );
			dest->used =+ i;
/*		((UBYTE*)dest->beg)[ dest->used + 1 ] = 0; */
		}else return( FALSE );

#ifdef notdef

		if ( Fwrite ( file_handle, i, out ) != i )
/*	if ( fwrite ( out, 1, i, file ) != i ) */
			return FALSE;

#endif

	}

	return TRUE;
}
