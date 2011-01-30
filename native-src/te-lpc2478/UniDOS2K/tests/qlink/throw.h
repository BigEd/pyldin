//#include <kernel.h>

#define DDEUtils_ThrowbackStart      0x42587
#define DDEUtils_ThrowbackSend       0x42588
#define DDEUtils_ThrowbackEnd        0x42589


#define Throwback_ReasonProcessing     0
#define Throwback_ReasonErrorDetails   1
#define Throwback_ReasonInfoDetails    2

#define ThrowbackInfo        -1
#define ThrowbackWarning      0
#define ThrowbackError        1
#define ThrowbackSeriousError 2

extern int ythrow, throwback;

//extern _kernel_oserror *ThrowbackStart(void);
//extern _kernel_oserror *ThrowbackSendStart(char *);
//extern _kernel_oserror *ThrowbackSendError(int, int, char *);
//extern _kernel_oserror *ThrowbackEnd(void);

//extern void throw_start(char *);
//extern void throw_file(char *);
//extern void throw_end(void);
//extern void throw_send(int, int, char *);
