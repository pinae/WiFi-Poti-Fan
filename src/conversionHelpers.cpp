#include <Arduino.h>
#include "debugLogger.h"

extern DebugLogger logger;

int strToInt(char* str) {
    char** conversionErrorPos = 0;
    int convertedNumber = strtoul(str, conversionErrorPos, 10);
    logger.printf("Converting \"%s\" to %d.\n", str, convertedNumber);
    if (conversionErrorPos == 0) {
        return convertedNumber;
    } else {
        logger.printf("Error converting char* to number: %s\n", str);
    }
    return -1;
}