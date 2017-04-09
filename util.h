#ifndef __UTIL_H_
#define __UTIL_H_

void uartEnable(int divisor);
void adcEnable();
void adcRead(int channels);
void uartSend(int c);
void uartSends(char* s);
int intDiv(int a, int b);
void uartSendHex(int x, int d);
void uartSendDec(int x);
void setupPll(int mhz);

void pinMode(int base, char num, char mode, char cnf);
void pinOutput(int base, char num, char v);
char pinInput(int base, char num);

#endif

