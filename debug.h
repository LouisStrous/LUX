/* File debug.h
 "$Id"
 Replaces allocation routines with similar routines that also keep records
 of the transactions so that consistency can be checked.
 LS 31jul98 */

#define malloc	Malloc
#define calloc	Calloc
#define realloc	Realloc
#define free	Free


