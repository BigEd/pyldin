
#ifndef __TYPE_H__
#define __TYPE_H__

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef __FALSE
#define __FALSE   (0)
#define FALSE   (0)
#endif

#ifndef __TRUE
#define __TRUE    (1)
#define TRUE    (1)
#endif

typedef char               S8;
typedef short              S16;
typedef int                S32;
typedef long long          S64;
typedef unsigned char      U8;
typedef unsigned short     U16;
typedef unsigned int       U32;
typedef unsigned long long U64;
typedef unsigned char      BIT;
typedef unsigned int       BOOL;

#define	Bit(x)	((U32) 1<<x)


#endif  /* __TYPE_H__ */
