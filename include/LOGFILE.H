/*

HEADER File pour logfile.c (C) Olivier Booklage 1996
Les messages ne doivent pas d‚passer 125 caracteres ! 
pour la version logfile3.c (Fopen)

*/

/* modes : */

/* Controle de la taille et type de cession */

#define	LOG_CESSION			0x000L	/* logfile par cession,pas de	controle de la taille */
#define	LOG_NOTSIZED		0x001L	/* accumulation de la log sans controle de la taille */
#define	LOG_SIZED				0x002L	/* accumulation de la log avec controle de la taille */

/* type d'ouverture */

#define	LOG_FREE				0x000L	/* ouverte en permanence */
#define	LOG_DISCRET			0x010L	/* ouverte puis referm‚e */

/* pr‚sentation  */

#define	LOG_NOSTAMP			0x000L	/*	Rien  */
#define	LOG_TIMESTAMP		0x100L	/*	Ajout de l'heure … chaque ligne */
#define	LOG_DATESTAMP		0x200L	/*	Ajout de la date … chaque ligne */
#define	LOG_CLOCKSTAMP	0x300L	/*	Ajout de date+heure … chaque ligne */

/*	---------------------- Ouverture --------------------------------

	Entr‚es :

		file			= Chaine contenant le chemin+nom de la logfile d‚sir‚e
		max_size	= Taille maxi souhait‚e ou 0
		mode			= Voir les modes plus haut

	Sortie :

		normale		=	TRUE
		erreure		=	FALSE
-------------------------------------------------------------------- */

BOOLEAN LOG_open( UBYTE *file ,long	max_size, LONG	mode, UBYTE	*name,LONG	debug_level);

BOOLEAN	LOG_welcome(VOID);

WORD		STD_printf(UBYTE *fmt,...);
WORD		ALL_printf(UBYTE *fmt,...);
WORD		LOG_printf(UBYTE *fmt,...);
WORD		ERR_printf(UBYTE *fmt,...);

WORD		LOG_bye(VOID);
WORD		LOG_close(VOID);

WORD		LOG_lf(VOID);	/* simple retour … la ligne */

/* R‚glage de la validit‚ d'inscription :

	exemple : LOG_debug("SEMPE",DEBUG_0|DEBUG_6);
	permet l'affichage en log des messages commencant par "- coucou..
	et "# re-coucou..

	Rem :Si le message d‚bute par un double espace : "  re-re-coucou..
	il sera dans la logfile quelque soit son niveau de debug
	
*/

#define	DEBUG_0					0x0001L
#define	DEBUG_1					0x0002L
#define	DEBUG_2					0x0004L
#define	DEBUG_3					0x0008L
#define	DEBUG_4					0x0010L
#define	DEBUG_5					0x0020L
#define	DEBUG_6					0x0040L
#define	DEBUG_7					0x0080L
#define	DEBUG_8					0x0100L
#define	DEBUG_9					0x0200L
#define	DEBUG_10				0x0400L
#define	DEBUG_11				0x0800L
#define	DEBUG_12				0x1000L
#define	DEBUG_13				0x2000L

/*

DEBUG_0		-
DEBUG_1		~
DEBUG_2		!
DEBUG_3		?
DEBUG_4		+
DEBUG_5		:
DEBUG_6		#
DEBUG_7		*
DEBUG_8		r
DEBUG_9		w
DEBUG_10	$
DEBUG_11	&
DEBUG_12	>
DEBUG_13	/

*/
