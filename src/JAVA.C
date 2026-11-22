/*
 * java.c
 * Appel du module Java (RunJavaPlugIn).
 *
 * Positionne le chemin du fichier puis lance le greffon Java.
 *
 * Utilise : GLOBALS.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 01/08/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<tos.h>
#include	<portab.h>
#include	<logfile.h>
#include	<nkcc/nkcc.h>	/* v2.93 */
#include	"globals.h"


/* ---       Includes internes       -- */

#include	"java.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL UBYTE path[PATHSIZE+1];
LOCAL UBYTE name[PATHSIZE+1];

/* ---     Prototypage interne      --- */

LOCAL VOID file_splitt(const UBYTE *fullname, UBYTE *path, UBYTE *name);
LOCAL BOOLEAN SetPath(UBYTE *unixpath);

/* ---         Fonctions            --- */

VOID RunJavaPlugIn( UBYTE *filename )
{
UBYTE commande[128];
INT retour;

	*commande = '\0';

#ifdef notdef
	strncat( commande, "XCPI1", 0x7E );
	strncat( commande, " ", 0x7E );
#endif
	strncat( commande, "X", 0x7E );
	strncat( commande, filename, 0x7E );
	commande[127] = 0;
	commande[0] = (int)strlen( &commande[1] );

/* Changement de r‚pertoire */

	file_splitt(JAVA_PLUG_IN,path,name);

	if ( SetPath(path) != TRUE )
	{
/*
		xprintf('term',"? SetPath <%s> error\n",path);
		LOG_printf("? SetPath <%s> error\n",path);
		client->headerbuf.used = min(strlen(MSG_INTERN_ERR),client->headerbuf.len);
		memcpy(client->headerbuf.beg,MSG_INTERN_ERR,client->headerbuf.used);
		UnPrepare( client );
		return( FALSE );
*/
	}

	retour = (int)Pexec(0,JAVA_PLUG_IN, commande, 0L);
	
	if ( retour < 0 )
		LOG_printf("! Run PLUG-IN <%s> error <%d>\n", JAVA_PLUG_IN, retour);
}

LOCAL VOID file_splitt(const UBYTE *fullname, UBYTE *path, UBYTE *name)
{
	UBYTE	 *str;

	str = strrchr (fullname, '\\');

	if (path != NULL)
		path[0] = EOS;		/* schadet nix */
	if (name != NULL)
		name[0] = EOS;		/* ditto */

	if (str != NULL)
	{
		/* Dateinamen holen */
		if (name != NULL)
		{
			*name=0;
			strcat(name, str+1);
		}

		/* Pfad mit Laufwerk bestimmen */
		if (path != NULL)
		{
			WORD len =  (short)( str - (UBYTE *)fullname );
			strncpy(path, fullname, len);
			path[len] = EOS;
		}
	}
} /* file_split */

/* Changement de r‚pertoire , path est SANS \\ */

LOCAL BOOLEAN SetPath(UBYTE *unixpath)
{
WORD	drive;

	LOG_printf("> SetPath <%s>\n",unixpath);
	
	/* Vient de QED souces (globals.c) : */

		unixpath[0] = nkc_toupper(unixpath[0]);
		if (unixpath[0] >= '1' && unixpath[0] <= '6')
			drive = unixpath[0] - '1' + 26;
		else
			drive = unixpath[0] - 'A';
		if ( Dsetdrv(drive) < 0 )			return(FALSE);
		if ( Dsetpath(unixpath + 2) ) return(FALSE);

return(TRUE);
}

/* ---          EOF                 --- */
