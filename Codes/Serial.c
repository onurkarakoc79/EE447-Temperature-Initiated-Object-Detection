#include "Serial.h"

extern void OutStr(char *str);
#include "Serial.h"

// Function to compute powers of 10
static unsigned int power10(int exponent) {
    unsigned int result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= 10;
    }
    return result;
}

// Helper function to reverse a string
static void reverse(char *str, int length) {
    int start = 0, end = length - 1;
    char temp;
    while (start < end) {
        temp = str[start];
        str[start++] = str[end];
        str[end--] = temp;
    }
}

// Helper function to convert integer to string
static int intToStr(int number, char *str) {
    int isNegative = 0, i = 0;

    if (number < 0) {
        isNegative = 1;
        number = -number;
    }

    do {
        str[i++] = '0' + (number % 10);
        number /= 10;
    } while (number);

    if (isNegative) {
        str[i++] = '-';
    }

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Print an integer
static void printInt(int number) {
    char text[12];
    int len = intToStr(number, text);
    text[len] = 0x04; // End of transmission
    OutStr(text);
}

// Print a string
void printStr(char *str) {
    char text[256]; // Assume a reasonable maximum size
    int i = 0;

    while (str[i] != '\0') {
        text[i] = str[i];
        i++;
    }

    text[i++] = '\r'; // Carriage return
    text[i++] = 0x04; // End of transmission
    text[i] = '\0';

    OutStr(text);
}

// Print an integer with a newline
static void printlnInt(int number) {
    char text[16];
    int len = intToStr(number, text);
    text[len++] = '\r'; // Carriage return
    text[len++] = '\n'; // Newline
    text[len++] = 0x04; // End of transmission
    text[len] = '\0';

    OutStr(text);
}

// Print a string with a newline
void printlnStr(char *str) {
    char text[258]; // Assume a reasonable maximum size
    int i = 0;

    while (str[i] != '\0') {
        text[i] = str[i];
        i++;
    }

    text[i++] = '\r'; // Carriage return
    text[i++] = '\n'; // Newline
    text[i++] = 0x04; // End of transmission
    text[i] = '\0';

    OutStr(text);
}

// Convert and print a float
static void floatToStr(float number, int decimalPlaces, char *str) {
    int integerPart = (int)number;
    float fractionalPart = (number < 0) ? -(number - integerPart) : (number - integerPart);
    int fractionalDigits = (int)(fractionalPart * power10(decimalPlaces) + 0.5);

    int idx = intToStr(integerPart, str);
    str[idx++] = '.';

    char fracBuffer[12];
    int fracLen = intToStr(fractionalDigits, fracBuffer);

    // Add leading zeros if necessary
    for (int i = 0; i < decimalPlaces - fracLen; i++) {
        str[idx++] = '0';
    }

    for (int i = 0; i < fracLen; i++) {
        str[idx++] = fracBuffer[i];
    }

    str[idx] = '\0';
}

// Print a float
static void printFloat(float number, int decimalPlaces) {
    char text[20];
    floatToStr(number, decimalPlaces, text);
    int len = 0;
    while (text[len] != '\0') {
        len++;
    }
    text[len] = 0x04; // End of transmission
    text[len + 1] = '\0';
    OutStr(text);
}

// Print a float with a newline
static void printlnFloat(float number, int decimalPlaces) {
    char text[24];
    floatToStr(number, decimalPlaces, text);

    int len = 0;
    while (text[len] != '\0') {
        len++;
    }

    text[len++] = '\r'; // Carriage return
    text[len++] = '\n'; // Newline
    text[len++] = 0x04; // End of transmission
    text[len] = '\0';

    OutStr(text);
}

// SerialLib structure
SerialLib Serial = {
    .printInt = printInt,
    .printStr = printStr,
    .printlnInt = printlnInt,
    .printlnStr = printlnStr,
    .printlnFloat = printlnFloat,
    .printFloat = printFloat,
};
