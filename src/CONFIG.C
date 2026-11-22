/*
 * config.c
 * Chargement et sauvegarde de la configuration.
 *
 * InitConf, LoadConf et SaveConf lisent et ‚crivent les r‚glages.
 *
 * Utilise : GLOBALS.H, RSC.H
 * Utilis‚ par : COOKIE.C, HTTP_GET.C, MAILTO.C, MAIN.C, MY_HEAD.C,
 *   RSC.C, SETUP.C, TOOLS.C
 *
 * Cr‚ation : 16/05/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---			 Includes externes			 -- */

#include	<stdio.h>
#include	<string.h>
#include	<portab.h>
#include	<logfile.h>

#include	"globals.h"
#include	"rsc.h"

/* ---			 Includes internes			 -- */

#include	"config.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

CONFIG config;

/* ---		 Prototypage interne			--- */

LOCAL BOOLEAN LoadConf(UBYTE *filename);

/* ---				 Fonctions						--- */

BOOLEAN InitConf(UBYTE *filename)
{

	LOG_printf("> InitConf <%s>\n", filename);

	if ( LoadConf(filename) == FALSE )	/* on initialise … la main */
	{
		config.version = CONFIG_VERSION;
		config.logfile_open	= FALSE;
		config.debug	= FALSE;
		config.timeout	= 60;
		config.gem				= 500;
		config.timer			= 5;
		config.cookie			= CAB_COOKIE;
		config.online			= TRUE;
	}

return(TRUE);
}

LOCAL BOOLEAN LoadConf(UBYTE *filename)
{
FILE	*file_ptr;
BOOLEAN ret = TRUE;

	LOG_printf("> LoadConf <%s>\n", filename);

	if( (file_ptr=fopen(filename,"rb")) == NULL)
	{
		return(FALSE);
	}

	if( fread(&config,sizeof(CONFIG),1,file_ptr) <= 0 )
	{
		my_form_alerte( "Error", "[1][ An error occure reading config file :| %s | please re-set your CAB.OVL setup ][ OK ]", filename );
		LOG_printf("Read w%d> error, please set CAB.OVL setup\n",filename);
		ret = FALSE;
	}
	
	if ( config.version != CONFIG_VERSION )
	{
		my_form_alerte( "Error", "[1][ Your CAB.OVL config file :| %s | is changed, please re-set it ][ OK ]", filename );
		LOG_printf("Read w%d> error, please set CAB.OVL setup\n",filename);
		ret = FALSE;
	}

	fclose(file_ptr);
		
return( ret );
}

BOOLEAN SaveConf(UBYTE *filename)
{
FILE	*file_ptr;

	LOG_printf("> SaveConf <%s>\n", filename);
	
	file_ptr=fopen(filename,"wb");
	
	if( file_ptr != NULL)
	{
		config.version = CONFIG_VERSION;
		if( fwrite(&config,sizeof(CONFIG),1,file_ptr) > 0)
		{
			fclose(file_ptr);
			return(TRUE);
		}
		ERR_printf("ERR_write_file %s\n",filename);
		fclose(file_ptr);
		return(FALSE);
	}

	ERR_printf("ERR_open_write_file %s\n",filename);

return(FALSE);
}

/* ---					EOF 								--- */
