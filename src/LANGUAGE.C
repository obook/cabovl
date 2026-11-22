/*
 * language.c
 * Chargement d'un fichier de langue.
 *
 * load_language lit le fichier, Give_languages liste les langues.
 *
 * Utilise : GLOBALS.H, RSC.H, TOOLS.H
 * Utilis‚ par : MAIN.C
 *
 * Cr‚ation : 01/01/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<aes.h>
#include	<portab.h>
#include	<logfile.h>

#include	"globals.h"
#include	"tools.h"
#include	"rsc.h"

/* ---       Includes internes       -- */

#include "language.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL BYTE line[256],flag[MAX_STR],value[MAX_STR];

/* ---     Prototypage interne      --- */

LOCAL VOID ProcessLine(BYTE *line, BYTE *f, BYTE *v, int limit);

/* ---         Fonctions            --- */

WORD	load_language ( UBYTE *name )
{
FILE *file_handle;
BOOLEAN ok = FALSE;

	LOG_printf("> load_language <%s>\n", name);

	file_handle = fopen(LANGUE_FILE,"r");
	
	if (file_handle == NULL)
	{
		LOG_printf("! LANGUE_FILE file <%s> not found\n",LANGUE_FILE);
		return(-33);
	}
	
	while ( fgets(line, 127, file_handle) != NULL )
	{
		if ( (*line != ';') && (*line != '#') )
		{
			ProcessLine(line, flag, value, MAX_STR);
			if ( (*flag != EOS) && (*value != EOS) )
			{

				if ( stricmp( flag , "00" ) == SUCCESS)
				{
					if ( (ok == FALSE) && ( stricmp(value,name) == SUCCESS ) )
					{
						ok = TRUE;
					}
					else
					{
						ok = FALSE;
					}
				}
				
				if ( ok == TRUE )
				{
					if ( stricmp( flag , "0A" ) == SUCCESS )	/* Ok */
					{
						*err_form[ERR_OK].ob_spec.tedinfo->te_ptext = '\0';
						strncat( err_form[ERR_OK].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
					}
					else if ( stricmp( flag , "0B" ) == SUCCESS )	/* Abort */
					{
						*adr_form[DIAL_ABORT].ob_spec.tedinfo->te_ptext = '\0';
						strncat( adr_form[DIAL_ABORT].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
						*err_form[ERR_ABORT].ob_spec.tedinfo->te_ptext = '\0';
						strncat( err_form[ERR_ABORT].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
					}
					else if ( stricmp( flag , "0C" ) == SUCCESS )	/* Noticed */
					{
						*adr_form[DIAL_NOTICED].ob_spec.tedinfo->te_ptext = '\0';
						strncat( adr_form[DIAL_NOTICED].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
						*err_form[ERR_NOTICED].ob_spec.tedinfo->te_ptext = '\0';
						strncat( err_form[ERR_NOTICED].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
					}
					else if ( stricmp( flag , "0D" ) == SUCCESS )	/* Debug */
					{
						*adr_form[DIAL_DEBUG].ob_spec.tedinfo->te_ptext = '\0';
						strncat( adr_form[DIAL_DEBUG].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
						*err_form[ERR_DEBUG].ob_spec.tedinfo->te_ptext = '\0';
						strncat( err_form[ERR_DEBUG].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
					}
					else if ( stricmp( flag , "0E" ) == SUCCESS )	/* Never Mind */
					{
						*adr_form[DIAL_MIND].ob_spec.tedinfo->te_ptext = '\0';
						strncat( adr_form[DIAL_MIND].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
						*err_form[ERR_NEVER].ob_spec.tedinfo->te_ptext = '\0';
						strncat( err_form[ERR_NEVER].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
					}
					else if ( stricmp( flag , "0F" ) == SUCCESS )	/* Retry */
					{
						*adr_form[DIAL_RETRY].ob_spec.tedinfo->te_ptext = '\0';
						strncat( adr_form[DIAL_RETRY].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
						*err_form[ERR_RETRY].ob_spec.tedinfo->te_ptext = '\0';
						strncat( err_form[ERR_RETRY].ob_spec.tedinfo->te_ptext , value, MAX_BUTTON);
					}
				}
			}
		}
	}
	fclose(file_handle);
return(FALSE);
}

WORD	Give_languages ( UBYTE *name , UBYTE *buffer, INT limit)
{
FILE *file_handle;
BOOLEAN ok = FALSE;

	LOG_printf("> Give_languages <%s>\n", name);

	file_handle = fopen(LANGUE_FILE,"r");
	
	if (file_handle == NULL)
	{
		LOG_printf("! LANGUE_FILE file <%s> not found\n",LANGUE_FILE);
		return(-33);
	}
	
	*buffer = '\0';
	
	while ( fgets(line, 127, file_handle) != NULL )
	{
		if ( (*line != ';') && (*line != '#') )
		{
			ProcessLine(line, flag, value, MAX_STR);
			if ( (*flag != EOS) && (*value != EOS) )
			{

				if ( stricmp( flag , "00" ) == SUCCESS)
				{
					ok = TRUE;
					strncat( buffer, value, limit);
					strncat( buffer, " ", limit);
				}
			}
		}
	}
	fclose(file_handle);

return( ok );
}

/* Extraction des elements (tokens) d'une chaine */

LOCAL VOID ProcessLine(BYTE *line, BYTE *f, BYTE *v, int limit)
{
char	*pos,*token;
long	i;	/* OB */
	
	LOG_printf("* ProcessLine %s\n",line);
	
	for(i=0;line[i]!=0;i++)	if(line[i]==9)	line[i]=32;	/* Allow TAB */

	pos = strchr(line, ';');
	if (pos) *pos = EOS;
	
	pos = strchr(line, '\n');
	if (pos) *pos = EOS;
	
	token = strtok(line, " ");
	
	*f = *v = EOS;
	
	if (token)
	{
		*f=0;
		strncat(f, token, limit);
		
		token = strtok(NULL, "\0");
		
		if (token)
		{
			*v=0;
			strncat(v, skipwhite(token),limit);
		}
	}
}

/* ---          EOF                 --- */
