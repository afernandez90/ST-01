#pragma once
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <string>
#include <sstream>
#define F(s) s
#define LED_BUILTIN 13
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
#define _BV(n) (1 << (n))
#define FE1 0
#define DOR1 1
#define UPE1 2
#define USBS1 3
#define UCSZ11 4
#define UCSZ10 5
#define RXEN1 6
#define RXCIE1 7
inline uint8_t UCSR1A, UCSR1B, UCSR1C, UDR1, UBRR1H, UBRR1L;
inline unsigned long reloj=0;
inline int salida=-1;
inline unsigned long millis(){return reloj;}
inline void pinMode(int,int){}
inline void digitalWrite(int pin,int v);
inline int digitalRead(int){return HIGH;}
inline void analogWrite(int pin,int v);
inline int constrain(int n,int lo,int hi){return std::min(hi,std::max(lo,n));}
struct Puerto {std::string texto; void begin(int){} void flush(){} template<class T> void print(T v){std::ostringstream s;s<<v;texto+=s.str();} void print(uint8_t v){print(int(v));} void println(){texto+='\n';} template<class T> void println(T v){print(v);println();}};
inline Puerto Serial;
#define TXC1 0
#define TXEN1 3
#define UDRIE1 4
#define TXCIE1 5
inline unsigned long esperaMicros=0;
inline void delayMicroseconds(unsigned n){esperaMicros+=n;}
#define MSBFIRST 1
inline int pinesAnalogicos[70]={};
inline int pinesDigitales[70]={};
inline uint8_t desplazados[1024]={};
inline unsigned cuantosDesplazados=0;
inline void shiftOut(int,int,int,uint8_t valor){desplazados[cuantosDesplazados++%1024]=valor;}

inline void digitalWrite(int pin,int v){pinesDigitales[pin]=v;}
inline void analogWrite(int pin,int v){pinesAnalogicos[pin]=v;if(pin==13)salida=v;}
