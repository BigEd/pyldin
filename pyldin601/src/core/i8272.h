#ifndef _I8272_H_
#define _I8272_H_

void i8272_init(void);

void i8272_write(byte a, byte d);
byte i8272_read(byte a);

#endif
