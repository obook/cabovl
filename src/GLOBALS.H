/*
 * globals.h
 * Constantes, types et macros communs … tout le module.
 *
 * Tailles de tampons, valeurs des cookies, codes d'erreur et
 * structure REQUEST. Inclus par presque tous les fichiers.
 *
 * Utilis‚ par : AUTHEN.C, B64ENCOD.C, CACHE_IP.C, CONFIG.C, COOKIE.C,
 *   ENV.C, EXEC.C, GET_INFO.C, GET_URL.C, GLOBALS.C, HEADER.C,
 *   HTTP_GET.C, IF_MODIF.C, JAVA.C, LANGUAGE.C, MAILTO.C, MAIN.C,
 *   MY_HEAD.C, POST_URL.C, REPORT.C, RSC.C, SENDICON.C, SETUP.C, TCP.C,
 *   TOOLS.C
 *
 * Cr‚ation : 07/11/1997
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

#define MDEBUG
/* #define _RSC_INT_ */ /* Ressource incluse */

#include	<time.h>
#include	<portab.h>

/* --- Export Constantes globales   --- */

#define PATHSIZE				128
#define MAX_STR					128
#define MAX_RSC					60
#define MAX_BUTTON			10

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))

/* TCP buffer */

#define TCP_BUFFER				8192 	/* 4096 */

/* cookies */

#define ACCEPT_COOKIE	1
#define REFUSE_COOKIE	2
#define STORE_COOKIE	3
#define ASK_COOKIE		4
#define CAB_COOKIE		5

/* Valeur pour browser_error */

#define NO_CONNECTION			-1
#define CONNECTION_FAILED	-36
#define NO_MEMORY					-39
#define BAD_SYN						-65
#define BAD_HANDLE				-300
#define NO_NAME_SERVER		-307
#define BAD_DNS_FORMAT		-309
#define NETWORK_ERROR			-311
#define NETWORK_DOWN			-312
#define HOST_UNREACHABLE	-313
#define	BAD_PACKET				-314
#define	USERBREAK					-315
#define	CONNECTION_RESET	-316
#define	SOCKET_CLOSE			-318
#define	CONNECTION_CLOSE	-320	/* connexion trop lente */
#define ACCESS_DENIED			-321
#define FAR_DNS						-323
#define	CANNOT_RESOLVE		-600

#ifndef _TOOLS_INTERNAL_
#define Fopen(a,b) X_File_open(a,b, __FILE__, __LINE__ )
#define Fcreate(a,b) X_File_create(a,b, __FILE__, __LINE__ )
#define Fclose(a) X_File_close(a, __FILE__, __LINE__ )
#define Fdelete(a) X_Fdelete(a, __FILE__, __LINE__ )
#define fopen(a,b) X_file_open(a,b, __FILE__, __LINE__ )
#define fclose(a) X_file_close(a, __FILE__, __LINE__ )
#define remove(a) X_remove(a, __FILE__, __LINE__ )
#endif

long  X_File_open( const char *fname, int mode , const char *file, long line );
long  X_File_create( const char *fname, int attr , const char *file, long line );
WORD  X_File_close ( WORD handle , const char *file, long line );
int		X_Fdelete( const char *fname , const char *file, long line );
FILE  *X_file_open( const char *filename, const char *mode , const char *file, long line );
WORD X_file_close ( FILE *stream , const char *file, long line );
int		X_remove( const char *fname , const char *file, long line );

/* --- Export Structures globales   --- */

#ifndef	__MEMORY_H_
#include	<upoint\memory.h>
#endif

#ifndef	__STRUCT_H_
#include	<upoint\struct.h>
#endif

typedef struct Request
{
	/* Internet */
	
	/* STinG */
	INT			tcp_handle;				/* Handle de la connexion TCP */	/* ok */

	/* TOS */
	UBYTE			*filename;				/* Pointeur sur le nom du fichier cache */
	BOOLEAN		cache_exist;			/* Le fichier est d‚j… dans le cache */
	INT				file_handle;			/* handle du fichier */	/* ok */

	/* Buffers */

	memdb		mem;										/* m‚moire de cette structure */
	memdb		netbuf;									/* buffer entr‚-sortie */
	txtdb		textlst;								/* tableau des pointeur sur les lignes du header recu */

	/* Network */
	UBYTE protocol[10];				/* ftp, http, gopher, news, mailto */
	UBYTE server[256];				/* serveur contact‚: true server ou proxy */
	UBYTE true_server[256];		/* toujours vrai serveur */
	INT port;

	UBYTE 	*proxy_server;		/* pointeur sur le serveur proxy */
	INT			proxy_port;				/* port du serveur proxy */

	/* URL */
		/* CAB donne : */
	UBYTE		*URL;							/* pointeur sur l'URL complete */
	UBYTE *content;
	UBYTE *enctype;
	INT		methode;
	LONG cab_date;
	
	/* ca pourrait etre un pointeur ? */
	
	UBYTE URI[1024];						/* uri, ex '/download/dumy.lzh'*/
	UBYTE stock_URL[1024];			/* utilis‚ pour stocker la current URL */
	UBYTE last_URL[1024];				/* utilis‚ par "Referer:" */

	/* Info */
	
	INT status;											/* http status, 200, 201 .. etc */
	UBYTE		content_encoding[256];  /* x-gzip or x-compress */
	LONG		content_length;         /* length of the object */
	UBYTE		content_type[256];      /* MIME type of the object */
	time_t	date;                 /* creation date of the object */
	time_t	expires;              /* expiration date of the object */
	time_t	last_modified;        /* date the object was last modified */
	UBYTE		location[256];          /* real location of the document; go here */
	double	mime_version;         /* MIME version used for this document;
	                                indicates a MIME compliant message */
	UBYTE		pragma[256];            /* message for every gateway etc */
	UBYTE		host_server[256];         /* server used by the host */
	UBYTE		www_authenticate[256];  /* authentication challenge */
	UBYTE		http_line[256];					/* PremiŠre line du header termin‚e par z‚ro */

}REQUEST;

/* --- Export Variables globales    --- */

EXTERN	BOOLEAN	logfile;			/* TRUE = en fonction */
EXTERN	LONG		cab_version;
EXTERN	UBYTE cab_version_str[];
EXTERN	UBYTE ovl_version_str[];

EXTERN	UBYTE work_path[];		/* chemin de cab.ovl termin‚ par '\' */
EXTERN	UBYTE RSC_FILE[];			/* chemin du fichier ressource */
EXTERN	UBYTE TMP_FILE[];			/* chemin du fichier temporaire, pour travaux divers */
EXTERN	UBYTE LOG_FILE[];			/* chemin du fichier logfile */
EXTERN	UBYTE CONFIG_FILE[];	/* chemin du fichier cabovl.set */
EXTERN	UBYTE COOKIE_FILE[];	/* chemin du fichier des cookie.set */
EXTERN	UBYTE LANGUE_FILE[];	/* chemin du fichier des languages */
EXTERN	UBYTE DEBUG_FILE[];		/* chemin du fichier des debug.txt */
EXTERN	UBYTE TMP_FILE[];			/* chemin du fichier temporaire pour travaux */

EXTERN UBYTE JAVA_PLUG_IN[];	/* chemin complet du plug-in Java */

EXTERN	INT	app_id;						/* defini dans windforx.lib */

/*
EXTERN UBYTE authentification_coded[];
EXTERN UBYTE name_password[];
EXTERN UBYTE authentification_server[];
*/

/* ---    Export Prototypage        --- */

/*

Retourne le pointeur d'une chaine ( max 255 ) avec inhibition
des caractŠres de controle \ % et "

*/

UBYTE *NettoyerChaine( const UBYTE *string );

/* ---          EOF                 --- */
