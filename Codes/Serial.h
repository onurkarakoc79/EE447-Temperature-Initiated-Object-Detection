#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>



typedef struct{
	void (*printInt)(int number);
	void (*printStr)(char str[]);
	void (*printlnInt) (int number);
	void (*printlnStr) (char* str);
	void (*printlnFloat)(float number,int);
	void (*printFloat) (float number,int);
}SerialLib;

extern SerialLib Serial;



#endif	//SERIAL_H