/*------------------------------------------------
//
// Module: memory.h
//
// Objet : Exportation
//
// ToDo  :
//
//
//
// Maintenance :
//
//		 Auteur  : Olivier Booklage
//		 Version : V1.0
//		 Date 	 : 23/03/1999
//		 Remarq. :
//
//
//
//-----------------------------------------------*/

#ifndef __MEMORY_H_
#define __MEMORY_H_

/* --- Export Constantes globales 	--- */

/* --- Export Structures globales 	--- */

/* eigener Memory Description Block */

typedef struct
{
	void		*beg; 			/* Begin benutzter Speicher 		*/
	char		*end; 			/* Zeiger auf letztes Byte			*/
	long		len;	 			/* LÑnge in Byte					*/
	long		used;				/* Anzahl benutzter Elemente! 	*/
	const 	char  * fl;	/* aufrufendes Sourcefile 		*/
	long		ln;					/* Zeilennummer Sourcefile			*/
}memdb;

	/* for initialisation */

#define _MEMDB { NULL, NULL, 0L, 0L, NULL, 0L }

/* --- Export Variables globales		--- */

/* ---		Export Prototypage				--- */

/* liefert den benutzten Speicher */

long Inet_memused ( void );

/* fÅr atexit am Programmende 	*/

void Inet_memcheck ( void );



/*
	allocate - Speicher reservieren

	öbergabe : memdb
						 Anzahl der Elemente
						 Grîûe eines Elementes
						 Sourcefile
						 Zeile im Sourcefile

	RÅckgabe : TRUE, wenn Speicher reserviert werden konnte
*/

int  Inet_allocate 	( memdb *mem, long count, long size, const char *, long );
int  Inet_reallocate ( memdb *mem, long count, long size, const char *, long );

/*
	release - Speicher freigeben

	öbergabe : memdb
					 Sourcefile
					 Zeile im Sourcefile
*/

void Inet_release	( memdb *mem, const char *, long );


/*--- for use --------------------------------------------------------------*/

/*
	 GetMemBlk()

	 öbergabe : memdb
							Anzahl Elemente
							Grîûe der Elemente in Bytes
	 RÅckgabe : TRUE, wenn ok, sonst FALSE
*/

#ifdef __FILE_

 #define NewMemBlk(a,b,c) Inet_allocate(a,b,c, __FILE_, __LINE__ )
 #define GetMemBlk(a,b,c) Inet_reallocate(a,b,c, __FILE_, __LINE__ )
 #define FreeMemBlk(a)		Inet_release(a, __FILE_, __LINE__ )

#else

 #define NewMemBlk(a,b,c) Inet_allocate(a,b,c, __FILE__, __LINE__ )
 #define GetMemBlk(a,b,c) Inet_reallocate(a,b,c, __FILE__, __LINE__ )
 #define FreeMemBlk(a)		Inet_release(a, __FILE__, __LINE__ )

#endif


#endif /* __MEMORY_H_ */

/* ---					EOF 								--- */
