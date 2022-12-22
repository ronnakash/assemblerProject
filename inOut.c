#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "util.h"
#include "tables.h"
#include "instructions.h"
#include "secondPass.h"
#include "firstPass.h"
#include "inOut.h"
#include "assembler.h"

/* makes the .ob file from codeImage; also frees codeImage */
void makeObFileFromCodeImage(code_image *codeImage, char *fileName)
{
    int codeLineNum, dataLineNum;
    char *codeLineNumAsString, *dataLineNumAsString;
    char *tempString, tempNameString[100], *tnsp = tempNameString;
    FILE *obFile;
    code_line *temp;
    strcpy(tnsp, fileName);
    strcat(tnsp, ".ob");
    codeLineNum = codeImage->lastCodeLine->address - codeImage->firstCodeLine->address + 1;
    dataLineNum = codeImage->lastDataLine->address - codeImage->firstDataLine->address + 1;
    obFile = fopen(tnsp, "w+");
    codeLineNumAsString = intToString(codeLineNum);
    dataLineNumAsString = intToString(dataLineNum);
    fputs(codeLineNumAsString, obFile);
    fputs(" ", obFile);
    fputs(dataLineNumAsString, obFile);
    fputs("\n", obFile);
    free(codeLineNumAsString);
    free(dataLineNumAsString);
    codeImage->currCodeLine = codeImage->firstCodeLine;
    while (codeImage->currCodeLine != NULL)
    {
        tempString = intToFourDigitString(codeImage->currCodeLine->address);
        fputs(tempString, obFile);
        fputs(codeImage->currCodeLine->code->hexaCode, obFile);
        fputc(' ', obFile);
        fputc(getAREChar(codeImage->currCodeLine->code->ARE), obFile);
        fputc('\n', obFile);
        temp = codeImage->currCodeLine;
        codeImage->currCodeLine = codeImage->currCodeLine->next;
        freeCodeLine(temp);
        free(tempString);
    }
    fclose(obFile);
    free(codeImage);

    return;
}

/* add the entry to .ent file */
bool addEntrytoOutputFile(char *entryName, symbol_table *symbolTable, FILE *entFile, input_line *line)
{
    symbol_table *temp;
    char *tempString;
    temp = getEntryByName(symbolTable, entryName);
    if (temp == NULL)
    {
        printf("%d : %s\tlabel %s not found\n", line->number, line->content, entryName);
        return FALSE;
    }
    /* add to file */
    fputs(entryName, entFile);
    fputc(' ', entFile);
    tempString = intToFourDigitString(temp->value);
    fputs(tempString, entFile);
    fputc('\n', entFile);
    if (tempString != NULL)
    {
        free(tempString);
        tempString = NULL;
    }
    return TRUE;
}

/* add the entry to the external file */
void addToExternalFile(FILE *extFile, char *label, int address)
{
    char *tempString;
    fputs(label, extFile);
    fputc(' ', extFile);
    tempString = intToFourDigitString(address);
    fputs(tempString, extFile);
    fputc('\n', extFile);
    if (tempString != NULL)
    {
        free(tempString);
        tempString = NULL;
    }
}

/* remove .ext and .ent files if thy are empty at the end of processing */
void removeEmptyFiles(char *fileName)
{
    char tempNameString[100];
    strcpy(tempNameString, fileName);
    strcat(tempNameString, ".ent");
    freeFileIfEmpty(tempNameString);
    strcpy(tempNameString, fileName);
    strcat(tempNameString, ".ext");
    freeFileIfEmpty(tempNameString);
}

/* remove a file if it is empty */
void freeFileIfEmpty(char *fileName)
{
    FILE *tempFile;
    int fileSize;
    tempFile = fopen(fileName, "r+");
    fseek(tempFile, 0, SEEK_END);
    fileSize = ftell(tempFile);
    fclose(tempFile);
    if (fileSize == 0)
        remove(fileName);
}