#ifndef _UNLZMA_H_
#define _UNLZMA_H_

#define USE_ALLOCA

#if !defined(USE_MALLOC) && !defined(USE_ALLOCA)
extern void *large_malloc(size_t size);
extern void *large_free(void *ptr);
#endif

int unlzma(unsigned char *, int,
	   int(*fill)(void*, unsigned int),
	   int(*flush)(void*, unsigned int),
	   unsigned char *output,
	   int *posp,
	   void(*error)(char *x)
	);

#endif
