/*
 * mintnet.c
 * Interface TCP/IP pour MiNTNet.
 *
 * Absent du fichier projet CABOVL.PRJ.
 *
 * Utilise : GLOBALS.H
 *
 * Cr‚ation : 10/09/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

/*
#include	<string.h>
#include	<logfile.h>
*/

#include	<stdio.h>
#include	<ext.h>
#include	<portab.h>

#include	"globals.h"
#include	"..\globals.h"
#include	"..\tools.h"

/* ---       Includes internes       -- */

typedef          char  int8;          /*   Signed  8 bit (char)             */
typedef unsigned char uint8;          /* Unsigned  8 bit (byte, octet)      */
typedef          int   int16;         /*   Signed 16 bit (int)              */
typedef unsigned int  uint16;         /* Unsigned 16 bit (word)             */
typedef          long  int32;         /*   Signed 32 bit                    */
typedef unsigned long uint32;         /* Unsigned 32 bit (longword)         */

#include "mintnet.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

typedef struct { 
  char  *s_name;      /* official service name 	*/
  char  **s_aliases;  /* alias list 						*/
  int		s_port;       /* port # 								*/
  char  *s_proto;     /* protocol to use 				*/
} DU_servent; 

/* --- Variables globales  internes --- */

BOOLEAN mintnet = FALSE;
LOCAL UBYTE mintnet_string[]="MintNet";

DU_servent *sp;			/* service spec for tcp/ftp */

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

/*

Pomp‚ sur DUFTP, a modifier !

*/


/* Initialises all the stuff to do with the FTP routines */

BOOLEAN MINTNET_init(VOID)
{

	chdir("u:/");							/* Must change to root partition here, else getservbyname will fail */
	sp=(DU_servent*)getservbyname("ftp","tcp");
	if (sp==0)
	{
		return( TRUE );
	}

return(TRUE);



#ifdef notdef

	char *cp,*home;
	struct passwd *pw = NULL;

// MiNT-NET INITIALISATION
	chdir("u:/");							// Must change to root partition here, else getservbyname will fail
	sp=(DU_servent*)getservbyname("ftp","tcp");
	if (sp==0)
	{
		form_alert(1,"[3][ ERROR: ftp/tcp: unknown service. | This probably means you haven't | installed MiNT-net correctly, as | '/etc/services' cannot be opened. ][ EXIT ]");
		close_down();
		exit(1);
	}

// Set up the home directory
	cp = getlogin();
	if (cp != NULL) {
		pw = getpwnam(cp);
	}
	if (pw == NULL)
		pw = getpwuid(getuid());
	if (pw != NULL) {
		sprintf(homedir, "%s", pw->pw_dir);
	}else{
		getcwd(homedir,0);
	}
	home = homedir;

// Set up defaults for FTP.
	type=TYPE_I;		// transferr type == binary (I know, it violates the RFC and is
						// very lazy to just use binary.....
						// **NOTE - ok, I've actually used the correct types further on.
	form = FORM_N;		// form - non-print
	mode = MODE_S;		// streaming mode
	stru = STRU_F;		// structure=file (no record or page formatting)
	bytesize = 8;		// 1 byte == 8 bits
	cpend = 0;			// no pending replies
	proxy = 0;			// proxy not active
	crflag = 1;			// strip c.r. on ascii gets
	data = -1;
	abrtflag = 0;
	ptflag = 0;
	restart_point = 0;

// Initially, we aren't connected
	connected=0;

// If we lost the ftp connection, better close down the link
	signal(SIGPIPE, lostpeer);


#endif

}

int16 MINTNET_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize)
{
}

int16 MINTNET_TCP_wait_state(int16 cn, int16 state, int16 timeout )
{
}

int16 MINTNET_TCP_send(int16 cn, char *buf, int16 len)
{
}

int16 MINTNET_TCP_close(int16 cn, int16 timeout)
{
}

char  *MINTNET_get_err_text(int16 code)
{
}

int16 MINTNET_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize)
{
}

int16 MINTNET_CNbyte_count(int16 cn)
{
}

int16 MINTNET_CNget_block(int16 cn, char *blk, int16 len)
{
}

char *MINTNET_getvstr(char *variable )
{
}


/* ---          EOF                 --- */
