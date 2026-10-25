#pragma once
#include <string>
struct hd44780_I2Cexp {bool iluminada=true;void backlight(){iluminada=true;}void noBacklight(){iluminada=false;}int fila=0;std::string lineas[2];hd44780_I2Cexp(int){} int begin(int,int){return 0;} void setCursor(int,int y){fila=y;} void print(const char* s){lineas[fila]=s;} };
