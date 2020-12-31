#include "utils.h"

#include <stdio.h>

int mepaLabelCounter = 0;

int countDigits(int number);

char* nextMEPALabel(){
    mepaLabelCounter++;

    int digitsCount = countDigits(mepaLabelCounter);
    char* mepaLabel = malloc(sizeof(char)*(digitsCount+1)); // TODO free this (memory leak!)
    sprintf(mepaLabel, "L%d", mepaLabelCounter);

    return mepaLabel;
}

int countDigits(int number) {
    int count = 0;

    do {
        count++;
        number /= 10;
    } while (number != 0);

    return count;
}

void initializeSymbolTable() {
    // TODO types integer and boolean
    // TODO constants true and false
    // TODO pseudo functions read and write
}
