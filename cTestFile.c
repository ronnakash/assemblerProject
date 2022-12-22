#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int main(void)
{
    char fileName[100];
    FILE *newFile;

    newFile =fopen(fileName, "r+");
    fclose(newFile);
    free(fileName);
    return 0;
}