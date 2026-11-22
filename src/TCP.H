/*
 * tcp.h
 * Prototypes et types de l'interface TCP/IP.
 *
 * Interface de TCP.C : types entiers (int16, uint32, ...) et fonctions X_*.
 *
 * Utilis‚ par : CACHE_IP.C, GET_INFO.C, HEADER.C, HTTP_GET.C, MAILTO.C,
 *   MAIN.C, MY_HEAD.C, TCP.C, TOOLS.C
 *
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

#ifndef STING_TRANSPRT_H
typedef          char  int8;          /*   Signed  8 bit (char)             */
typedef unsigned char uint8;          /* Unsigned  8 bit (byte, octet)      */
typedef          int   int16;         /*   Signed 16 bit (int)              */
typedef unsigned int  uint16;         /* Unsigned 16 bit (word)             */
typedef          long  int32;         /*   Signed 32 bit                    */
typedef unsigned long uint32;         /* Unsigned 32 bit (longword)         */

#define  E_NORMAL         0     /* No error occured ...                     */
#define  E_OBUFFULL      -1     /* Output buffer is full                    */
#define  E_NODATA        -2     /* No data available                        */
#define  E_EOF           -3     /* EOF from remote                          */
#define  E_RRESET        -4     /* Reset received from remote               */
#define  E_UA            -5     /* Unacceptable packet received, reset      */
#define  E_NOMEM         -6     /* Something failed due to lack of memory   */
#define  E_REFUSE        -7     /* Connection refused by remote             */
#define  E_BADSYN        -8     /* A SYN was received in the window         */
#define  E_BADHANDLE     -9     /* Bad connection handle used.              */
#define  E_LISTEN        -10    /* The connection is in LISTEN state        */
#define  E_NOCCB         -11    /* No free CCB's available                  */
#define  E_NOCONNECTION  -12    /* No connection matches this packet (TCP)  */
#define  E_CONNECTFAIL   -13    /* Failure to connect to remote port (TCP)  */
#define  E_BADCLOSE      -14    /* Invalid TCP_close() requested            */
#define  E_USERTIMEOUT   -15    /* A user function timed out                */
#define  E_CNTIMEOUT     -16    /* A connection timed out                   */
#define  E_CANTRESOLVE   -17    /* Can't resolve the hostname               */
#define  E_BADDNAME      -18    /* Domain name or dotted dec. bad format    */
#define  E_LOSTCARRIER   -19    /* The modem disconnected                   */
#define  E_NOHOSTNAME    -20    /* Hostname does not exist                  */
#define  E_DNSWORKLIMIT  -21    /* Resolver Work limit reached              */
#define  E_NONAMESERVER  -22    /* No nameservers could be found for query  */
#define  E_DNSBADFORMAT  -23    /* Bad format of DS query                   */
#define  E_UNREACHABLE   -24    /* Destination unreachable                  */
#define  E_DNSNOADDR     -25    /* No address records exist for host        */
#define  E_NOROUTINE     -26    /* Routine unavailable                      */
#define  E_LOCKED        -27    /* Locked by another application            */
#define  E_FRAGMENT      -28    /* Error during fragmentation               */
#define  E_TTLEXCEED     -29    /* Time To Live of an IP packet exceeded    */
#define  E_PARAMETER     -30    /* Problem with a parameter                 */
#define  E_BIGBUF        -31    /* Input buffer is too small for data       */
#define  E_LASTERROR      31    /* ABS of last error code in this list      */
#endif

#define DRACONIS_ABSENT			 -1
#define DRACONIS_DECONNECTE		0
#define DRACONIS_EN_COURS			1
#define DRACONIS_CONNECTE			2

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

EXTERN BOOLEAN draconis;
EXTERN BOOLEAN sting;
EXTERN BOOLEAN iconnect;

EXTERN UBYTE *tcp_version;
EXTERN UBYTE *transport;

/* ---    Export Prototypage        --- */

BOOLEAN init_tcp( VOID );

int16 X_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize);
int16 X_TCP_wait_state(int16 cn, int16 state, int16 timeout );
int16 X_TCP_send(int16 cn, char *buf, int16 len);
int16 X_TCP_close(int16 cn, int16 timeout);
char  *X_get_err_text(int16 code);
int16 X_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize);
int16 X_CNbyte_count(int16 cn);
int16 X_CNget_block(int16 cn, char *blk, int16 len);
char *X_getvstr(char *variable );


/* ---          EOF                 --- */
