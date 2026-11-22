/*
 * authen.c
 * Authentification HTTP (RequestAuthentification).
 *
 * Demande l'identifiant et le mot de passe quand le serveur
 * r‚clame une authentification (boŒte de dialogue de RSC.C) et
 * les encode en base64 pour l'en-tˆte de la requˆte.
 *
 * Utilise : GLOBALS.H, MAIN.H, RSC.H
 * Utilis‚ par : HTTP_GET.C, MAIN.C, MY_HEAD.C, RSC.C
 *
 * Cr‚ation : 15/10/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* #define	__USE_CAB_AUTH__ */


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"globals.h"
#include	"main.h"
#include	"rsc.h"

/* ---       Includes internes       -- */

#include	"authen.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

UBYTE name_password[ MAX_STR +1 ];
UBYTE authentification_server[ MAX_STR +1 ];
UBYTE authentification_coded[ 2*( MAX_STR+1) ];	/* Il faudra faire + long pour le codage */

LOCAL UBYTE base64set[64] = {

 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', 	/* x00 */
 'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f', 	/* x10 */
 'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v', 	/* x20 */
 'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/' }; /* x30 */

/* ---     Prototypage interne      --- */

LOCAL BOOLEAN Codebase64 ( UBYTE *origine, UBYTE *destination, INT limite);

/* ---         Fonctions            --- */

BOOLEAN RequestAuthentification( REQUEST *request)
{
UBYTE *name_ptr;
UBYTE *password_ptr;

	*name_password = '\0';
	*authentification_coded = '\0';
	*authentification_server = '\0';

#ifdef __USE_CAB_AUTH__

	if ( !browser->ask_user(REQUEST_NAME, &name_ptr) ) /* ok */
		return(FALSE);

	strncat( name_password, name_ptr, MAX_STR );
	strncat( name_password, ":", MAX_STR );

	if ( !browser->ask_user(REQUEST_PASSWORD, &password_ptr) ) /* ok */
		return(FALSE);
	
	strncat( name_password, password_ptr, MAX_STR );

#else

	if ( ask_authentification( request->true_server, &name_ptr, &password_ptr) != AUTH_VALID )
	{
		LOG_printf("~ User do not use AUTH_VALID button\n");
		return(FALSE);
	}

#endif
		
	if ( Codebase64 ( name_password, authentification_coded , 2*( MAX_STR ) ) == FALSE )
	{
		LOG_printf("! ERROR in Codebase64\n");
		return(FALSE);
	}

	/* M‚morisation du serveur */
	
	strncat( authentification_server ,request->true_server, 126);

	LOG_printf("~ Restricted access on  = <%s>\n", authentification_server );
	LOG_printf("~ Authentification      = <%s>\n", name_password );
	LOG_printf("~ Coded on base64       = <%s>\n", authentification_coded );

return( TRUE );
}

LOCAL BOOLEAN Codebase64 ( UBYTE *origine, UBYTE *destination, INT limite)
{
int 	i = 0, j = 0;
long	s;
char	*c = origine;
char	*e = origine + strlen(origine);
char	out[80];
INT		len = 0;

	LOG_printf("> tobase64 <%s>\n", origine);
	
	if ( e <= c )
	{
		LOG_printf("~ la fin est plus que le debut\n");
		return(FALSE);
	}

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

		out[i++] = '\0';
		
		if( ( len + i ) < limite )
		{
			memcpy( destination + len, out, i );
			len =+ i;
		}
		else
		{
			LOG_printf("~ buffer de reception <%d> trop petit pour <%d> \n", limite, len+i);
			return(FALSE);
		}

	}

return(TRUE);
}

/* ---          EOF                 --- */
