/*
 * main.c
 * Point d'entr‚e du module CAB.OVL (protocole HTTP pour CAB).
 *
 * CAB charge ce module depuis son dossier MODULES. Ce fichier
 * fournit … CAB les fonctions du module : init_module, get_version,
 * restore_module et mailto. init_module renseigne la table des
 * m‚thodes (get_url, post, get_url_info, ...) et initialise la
 * configuration, la ressource, la langue et la pile TCP/IP.
 *
 * Utilise : AUTHEN.H, CONFIG.H, COOKIE.H, GET_INFO.H, GET_URL.H,
 *   GLOBALS.H, HTTP.H, IF_MODIF.H, LANGUAGE.H, POST_URL.H, RSC.H,
 *   TCP.H, TOOLS.H, VERSION.H
 * Utilis‚ par : AUTHEN.C, CACHE_IP.C, COOKIE.C, HTTP_GET.C, MAILTO.C,
 *   MY_HEAD.C, REPORT.C, RSC.C, SETUP.C, TOOLS.C
 *
 * Cr‚ation : 07/11/1997
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/* #define _USE_FTP_ */
/*------------------------------------------------
//
// Module: main.c
//
// Objet : programme principal de cab.ovl
//
// ToDo  :
//
//
//
// Maintenance :
//
//		 Auteur  : Olivier Booklage
//		 Version : V1.0
//		 Date 	 : 07/11/1997
//		 Remarq. :
//
~ 10 Apr 01:21:14 WWW  MakeHeadlist Line 5:0
<User-Agent: Mozilla/4.0 (compatible; MSIE 4.5; Mac_PowerPC)>
//
~ 10 Apr 01:49:36 WWW  MakeHeadlist Line 3:
<User-Agent: Mozilla/4.5 [en] (WinNT; I)>
//
//-----------------------------------------------*/


/* ---			 Includes externes			 -- */

#include	<stdio.h>
#include	<string.h>
#include	<tos.h>			/* BASPAG = > TEST !!! */
#include	<atarierr.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>

#include	"http.h"
#include	"globals.h"
#include	"version.h"
#include	"tools.h"

#include	"get_info.h"
#include	"get_url.h"
#include	"post_url.h"
#include	"if_modif.h"
#include	"cookie.h"
#include	"authen.h"
#include	"config.h"
#include	"tcp.h"
#include	"rsc.h"
#include	"language.h"

/* ---			 Includes internes			 -- */

#include	"main.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */
/* --- Structures globales internes --- */

proxy_url	proxy;

/* --- Variables globales  internes --- */

/* For Cab */

browser_info_t *browser;

/* For me */
/* static char user_agent[256]; */

char user_agent[256];

/* ---		 Prototypage interne			--- */

/* For Cab */

long ___CDECL init_module(url_methods_t *out, browser_info_t *in, char *path);

/* ---				 Fonctions						--- */

/* ----------------------------------------------------------------- **
** main - Doesn't useful, but take care that others functions don't  **
**				get `swallowed' by compiler optimizations 								 **
** ----------------------------------------------------------------- */

EXTERN INT form_alert( INT fo_adefbttn, const UBYTE *fo_astring );

int main()
{

static void *array[] =
{
	"Needs CAB.APP to run\r\n",
	(void *)BROWSER_MAGIC1,
	(void *)BROWSER_MAGIC2,
	(void *)BROWSER_MAGIC3,
	(void *)BROWSER_MAGIC4,
	init_module
};

	write(1, array[0], strlen(array[0]));

return(1);
}

long ___CDECL init_module(url_methods_t *out, browser_info_t *in, char *path)
{
int ftp_proxy_port;
char ftp_proxy[256];

	*work_path=0;
	strncat( work_path, path, PATHSIZE);

	/* ----- Fixation Ressource ----- */
	
 	*RSC_FILE = '\0';
 	strncat(RSC_FILE,work_path,PATHSIZE);
 	strncat(RSC_FILE,"cab_ovl.rsc",PATHSIZE);

	if ( rsc_load(RSC_FILE) != TRUE )
	{
		form_alert(1, "[3][CAB.OVL can not found | is resource file][ Abort ]");
		return (0);
	}

 	*TMP_FILE = '\0';
 	strncat(TMP_FILE,work_path,PATHSIZE);
 	strncat(TMP_FILE,"cabovl.tmp",PATHSIZE);
	
/*TestRsc();*/
		
	/*--- Set browser info variable ---*/

	browser = in;

	/*--- Fill URL methods structure ---*/

	out->restore = restore_module;
	out->get_url = get_url;
	out->get_url_info = get_url_info;
	out->get_version = get_version;
	out->post = post;
	out->get_url_if_modified = get_url_if_modified;

 	*LOG_FILE = '\0';
 	strncat(LOG_FILE,work_path,PATHSIZE);
 	strncat(LOG_FILE,"cabovl.log",PATHSIZE);

 	*CONFIG_FILE = '\0';
 	strncat(CONFIG_FILE,work_path,PATHSIZE);
 	strncat(CONFIG_FILE,"cabovl.set",PATHSIZE);

	*COOKIE_FILE 	 = '\0';
 	strncat(COOKIE_FILE,work_path,PATHSIZE);
 	strncat(COOKIE_FILE,"cookie.set",PATHSIZE);

	*LANGUE_FILE 	 = '\0';
 	strncat(LANGUE_FILE,work_path,PATHSIZE);
 	strncat(LANGUE_FILE,"language.str",PATHSIZE);

	*DEBUG_FILE 	 = '\0';
 	strncat(DEBUG_FILE,work_path,PATHSIZE);
 	strncat(DEBUG_FILE,"debug.txt",PATHSIZE);

	*TMP_FILE 	 = '\0';
 	strncat(TMP_FILE,work_path,PATHSIZE);
 	strncat(TMP_FILE,"cabovl.tmp",PATHSIZE);

	*JAVA_PLUG_IN 	 = '\0';
 	strncat(JAVA_PLUG_IN,work_path,PATHSIZE);
 	strncat(JAVA_PLUG_IN,"plug-in\\",PATHSIZE);
 	strncat(JAVA_PLUG_IN,"java.ovl",PATHSIZE);

	*authentification_server	=	'\0';
	*authentification_coded		= '\0';

	cab_version = in->version;
	*cab_version_str					=	'\0';
	strncat( cab_version_str, VersionCABString( cab_version ), MAX_STR );

	*ovl_version_str					=	'\0';
	strncat( ovl_version_str, VersionOVLString( CAB_OVL_VERSION ), MAX_STR );
	
	InitConf(CONFIG_FILE);

	if ( config.logfile_open == TRUE )
	{
	BOOLEAN ret;
	INT loop = 0;
	INT len = (INT)strlen(LOG_FILE);

#ifdef notdef
/*BASPAG	*baspag = ((BASPAG	*)in->basepage)->p_parent; => NULL */
	BASPAG	*baspag = (BASPAG	*)in->basepage; /*Only valid, if module with pexec(3) => ne donne pas d'info :( */
#endif
	
		do
		{
						/*																				                           		 rien 	 !			 ? */
			ret = LOG_open( LOG_FILE, 500L, LOG_CESSION|LOG_FREE|LOG_CLOCKSTAMP, "OVL ", DEBUG_0|DEBUG_2|DEBUG_3);
			if ( ret == FALSE )
			{
				if ( loop == 0 )	/* premiŠre erreure */
				{
					LOG_FILE[len-1] = '1';
				}
				else
				{
					LOG_FILE[len-1] = LOG_FILE[len-1] + 1 ;
				}
			}
			loop++;
		}while ( (ret != TRUE) && ( loop < 9 ) );
		
		LOG_welcome();
		if ( config.debug == TRUE )
		{
				/* 			 rien 	 ~			 !				?				>			 */
			LOG_level(DEBUG_0|DEBUG_1|DEBUG_2|DEBUG_3|DEBUG_12);
		}

/* printf("TEST p_cmdlin = <%s>\n", baspag->p_cmdlin ); vide :( */

#ifdef notdef

if ( baspag != NULL )
{
	if ( baspag->p_cmdlin != NULL )
		printf("TEST p_cmdlin = <%s>\n", baspag->p_cmdlin );
	else
		printf("TEST p_cmdlin = NULL\n");

	if ( baspag->p_dta != NULL )
	{
		if ( baspag->p_dta->d_fname != NULL )
			printf("TEST d_fname = <%s>\n", baspag->p_dta->d_fname );
		else
			printf("TEST baspag->p_dta->d_fname = NULL\n" );
	}
	else
	{
		printf("TEST p_dta = NULL\n");
	}
}
else
{
	printf("TEST baspag = NULL\n");
}

#endif

		LOG_printf("~ CAB version <V%s>\n", cab_version_str );
		LOG_printf("~ OVL version <V%s>\n", ovl_version_str );

	}

	/* Langue */

#ifdef _RSC_INT_
	load_language( "Francais" );
#endif

	/* Test stack TCP/IP */
	
	if ( init_tcp() != TRUE )
	{
		my_form_alerte( "NetWorkError", ChaineAlerte(A_STACK));
		LOG_printf("! Network Error: no StiK/STinK/Draconis\n");
		LOG_close();
		rsc_free();
		LOG_bye();
		LOG_close();
		return(0);
	}

	LOG_printf("~ Transport <%s> detected version <%s>\n", transport, tcp_version);

/*sprintf(user_agent, "Mozilla/2.0 (compatible; Cab/2.5 ; %s/%s; Atari)", transport, tcp_version); */
/*sprintf(user_agent, "Cab%s/%s (compatible; Mozilla/2.0; %s/%s; Atari)", cab_version, CAB_OVL_VERSION, transport, tcp_version); */
/*
D'aprŠs Martin-Eric Racine il faudrait le system multitache ou TOS
… la place de 'Atari', toujours sous la forme :

 ps : "Mozilla/4.5 [en] (WinNT; I)" donne rien avec Atari; CAB%s/%s

*/
	
/* On m'a dit que 5.0 etait pas compatible ( reserv‚ a DHTML ) */
/*sprintf(user_agent, "Mozilla/5.0 (compatible; CAB%s/%s; Atari)", cab_version_str, ovl_version_str);*/

	sprintf(user_agent, "Mozilla/3.0 (compatible; CAB%s/%s; Atari)", cab_version_str, ovl_version_str);

/* test : pas bien */
/*sprintf(user_agent, "Mozilla/4.5 [en] (WinNT; I)", cab_version_str, ovl_version_str); */

	if ( cab_version >= 808597296L )	/* 0x30323730L */
	{
  	out->get_url_cookie = get_url_cookie;
  	out->post_cookie = post_cookie;
  	out->get_url_ifmod_cookie = get_url_ifmod_cookie;
	}
	
	cookie_str = NULL;
	
	/* ----- Reglage du proxy ----- */

	in->proxy 	=	fix_proxy( in->proxy );

	/*--- Return support ---*/

#ifdef NOTDEF

	if (ftp_proxy_cfg(ftp_proxy, 256, &ftp_proxy_port)==TRUE)
#ifdef _USE_FTP_
		return(SUPPORT_HTTP|SUPPORT_FTP|SUPPORT_PEXEC|SUPPORT_MAILTO);
#else
		return(SUPPORT_HTTP|SUPPORT_PEXEC|SUPPORT_MAILTO); /* SUPPORT_MAILTO obligatoire pour les url "mailto:" */
#endif

#ifdef _USE_FTP_
return(SUPPORT_HTTP|SUPPORT_PEXEC|SUPPORT_FTP|SUPPORT_MAILTO);
#else
return(SUPPORT_HTTP|SUPPORT_PEXEC|SUPPORT_MAILTO); /* SUPPORT_MAILTO obligatoire pour les url "mailto:" */
#endif

#endif



#ifdef _USE_FTP_
return(SUPPORT_HTTP|SUPPORT_PEXEC|SUPPORT_FTP|SUPPORT_MAILTO);
#else
return(SUPPORT_HTTP|SUPPORT_PEXEC|SUPPORT_MAILTO); /* SUPPORT_MAILTO obligatoire pour les url "mailto:" */
#endif

}

/* ----------------------------------------------------------------- **
** get_version - Returns infos about module 												 **
** ----------------------------------------------------------------- */

void ___CDECL get_version(authorp, versionp, datep)
char **authorp;	/* 4x30 chars separated by '|' */
long *versionp;	/* Version number in BCD format (V1.15 -> 0x00011500) */
long *datep;	/* Date in BCD format (0xYYYYMMDD) */
{
	*versionp = CAB_OVL_VERSION;
	*datep = CAB_OVL_DATE;

	*authorp =
		"Olivier Booklage|Fast cab.ovl beta|xxx@xxx.xxx|http://xxx.xxx.xxx/";
}


/* ----------------------------------------------------------------- **
** restore_module - De-initialization 															 **
**									(freeing memory, closing files, etc...) 				 **
** ----------------------------------------------------------------- */

void ___CDECL restore_module()
{
	LOG_printf("- ***************************************************\n");
	LOG_printf("> restore_module\n");
	browser->proxy = &proxy;
	cookie_str = NULL;
	CheckCookieDate();
	rsc_free();
	LOG_bye();
	LOG_close();
	memcheck();
}

/*
url=email
subject=enctype
filename=content
*/

long ___CDECL mailto(char *url, char *subject, char *filename)
{

	LOG_printf("- ***************************************************\n");

	LOG_printf("> mailto <%s>\n", url, subject, filename);
	
/* filename peut etre le contenu du courrier a envoyer */

my_form_alerte( "CAB.OVL: mailto", ChaineAlerte(A_IMPLEMENT));

/* return( my_mailto( url, subject, filename) ); */
return( -1L );
}

/* ---					EOF 								--- */
