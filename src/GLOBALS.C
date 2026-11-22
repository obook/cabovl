/*
 * globals.c
 * Variables et fonctions communes … tout le module.
 *
 * Contient NettoyerChaine.
 *
 * Utilis‚ par : AUTHEN.C, B64ENCOD.C, CACHE_IP.C, CONFIG.C, COOKIE.C,
 *   ENV.C, EXEC.C, GET_INFO.C, GET_URL.C, HEADER.C, HTTP_GET.C,
 *   IF_MODIF.C, JAVA.C, LANGUAGE.C, MAILTO.C, MAIN.C, MY_HEAD.C,
 *   POST_URL.C, REPORT.C, RSC.C, SENDICON.C, SETUP.C, TCP.C, TOOLS.C
 *
 * Cr‚ation : 07/11/1997
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<portab.h>
#include	"globals.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

BOOLEAN	logfile = FALSE;	/* utilisation de la logfile */
LONG	cab_version=0L;
UBYTE cab_version_str[PATHSIZE+1];
UBYTE ovl_version_str[PATHSIZE+1];

UBYTE work_path[PATHSIZE+1];
UBYTE RSC_FILE[PATHSIZE+1];
UBYTE TMP_FILE[PATHSIZE+1];
UBYTE LOG_FILE[PATHSIZE+1];
UBYTE CONFIG_FILE[PATHSIZE+1];
UBYTE COOKIE_FILE[PATHSIZE+1];
UBYTE LANGUE_FILE[PATHSIZE+1];
UBYTE DEBUG_FILE[PATHSIZE+1];
UBYTE TMP_FILE[PATHSIZE+1];

UBYTE JAVA_PLUG_IN[PATHSIZE+1];	/* chemin complet du plug-in Java */

/*
UBYTE authentification_server[ MAX_STR +1 ];
UBYTE name_password[ MAX_STR +1 ];
UBYTE authentification_coded[ 2*( MAX_STR+1) ];
*/

/* ---         Fonctions            --- */

/*
Pour la logfile, on peut rechercher le caractŠres d‚clancheurs
et les ‚liminer

ex: \ => \\
    " => \"
    % => %%
*/
LOCAL UBYTE buffer[256];

UBYTE *NettoyerChaine( const UBYTE *string )
{
INT i = 0;
INT z = 0;
	
	*buffer = '\0';
	
	for( i = 0; (string[i] != '\0') && (z < 255); i++ )
	{
		if ( string[i] == '\\' )
		{
			buffer[z++] = '\\';
		}
		else if ( string[i] == 32 )
		{
			buffer[z++] = '\\';
		}
		else if ( string[i] == '%' )
		{
			buffer[z++] = '%';
		}
		
		buffer[z] = string[i];
		
		z++;
	}

return(buffer);
}


/* ---          EOF                 --- */
