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

#ifndef _COMMAND__TABLE_
#define _COMMAND__TABLE_

struct command_table commandTable =
    {{"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "prn", "rts", "stop"},
     {MOV_OP, CMP_OP, ADD_OP, SUB_OP, LEA_OP, CLR_OP, NOT_OP, INC_OP, DEC_OP,
      JMP_OP, BNE_OP, JSR_OP, RED_OP, PRN_OP, RTS_OP, STOP_OP},
     {NONE_FUNCT, NONE_FUNCT, ADD_FUNCT, SUB_FUNCT, NONE_FUNCT, CLR_FUNCT,
      NOT_FUNCT, INC_FUNCT, DEC_FUNCT, JMP_FUNCT, BNE_FUNCT, JSR_FUNCT, NONE_FUNCT, NONE_FUNCT, NONE_FUNCT, NONE_FUNCT},
     {NON_RELATIVE_ADDR, NON_RELATIVE_ADDR, NON_RELATIVE_ADDR, NON_RELATIVE_ADDR, DIRECT_ONLY_ADDR, NONE_ADDR, NONE_ADDR,
      NONE_ADDR, NONE_ADDR, NONE_ADDR, NONE_ADDR, NONE_ADDR, NONE_ADDR, NONE_ADDR, NONE_ADDR, NONE_ADDR},
     {DIRECT_OR_REGISTER_ADDR, NON_RELATIVE_ADDR, DIRECT_OR_REGISTER_ADDR, DIRECT_OR_REGISTER_ADDR, DIRECT_OR_REGISTER_ADDR,
      DIRECT_OR_REGISTER_ADDR, DIRECT_OR_REGISTER_ADDR, DIRECT_OR_REGISTER_ADDR, DIRECT_OR_REGISTER_ADDR, DIRECT_OR_RELATIVE_ADDR,
      DIRECT_OR_RELATIVE_ADDR, DIRECT_OR_RELATIVE_ADDR, DIRECT_OR_REGISTER_ADDR, NON_RELATIVE_ADDR, NONE_ADDR, NONE_ADDR}};

#endif

/* already checked label is valid and colon removed */
symbol_table *addToSymbolTable(char *label, char *instruction, int value, symbol_table *symbolTable, input_line *line, symbol_attribute attributeSymbol)
{
    symbol_table *newLabel;
    char *tempString = malloc(sizeof(char) * (strlen(label) + 1));
    strcpy(tempString, label);
    /* create new entry */
    newLabel = (symbol_table *)malloc(sizeof(symbol_table));
    newLabel->attribute = attributeSymbol;
    newLabel->symbol = tempString;
    newLabel->next = symbolTable;
    newLabel->value = value;
    return newLabel;
}

/* gets the appropriate attribute for given instruction */
symbol_attribute getSymbolAttribute(char *instruction)
{
    if (isStorageIsntruction(instruction))
        return DATA_ATT;
    else if (isExternInstruction(instruction))
        return EXTERN_ATT;
    else
        return CODE_ATT;
}

/* remove : from end of label string */
void removeColonFromLabel(char *label)
{
    int i;
    for (i = 0; label[i] != ':' && label[i] != '\0'; ++i)
        ;
    label[i] = '\0';
    return;
}

/* check if label is valid and not already defined*/
bool isValidLabel(char *label, symbol_table *symbolTable, input_line *line)
{
    int i;
    if (isKeyword(label) == TRUE) /* check if keyword */
    {
        printf("%d : %s\tillegal label name: %s is a saved keyword\n", line->number, line->content, label);
        return FALSE;
    }
    if (containsSymbol(label, symbolTable)) /* check if already defined */
    {
        printf("%d : %s\tlabel %s is already defined\n", line->number, line->content, label);
        return FALSE;
    }
    /* check if valid name */
    if (!isalpha(label[0]))
    {
        printf("%d : %s\tillegal label name: %s first character of label must be alphabetic letter\n", line->number, line->content, label);
        return FALSE;
    }
    for (i = 1; label[i] != '\0'; ++i)
    {
        if (!isalnum(label[i]))
        {
            printf("%d : %s\tillegal label name: %s contains illegal character \n", line->number, line->content, label);
            return FALSE;
        }
    }
    return TRUE;
}

/* adds a line of code to the codeImage */
void addCodeLineToCodeImage(machine_code *codeLine, int *IC, code_image *codeImage)
{
    code_line *newCodeLine;
    if (codeLine == NULL)
        return;
    newCodeLine = (code_line *)malloc(sizeof(code_line));
    newCodeLine->address = IC[0]++;
    newCodeLine->code = codeLine;
    newCodeLine->next = codeImage->firstDataLine;
    if (codeImage->firstCodeLine == NULL) /* check if codeImage is initialised and initialise if needed*/
    {
        codeImage->firstCodeLine = newCodeLine;
        codeImage->lastCodeLine = newCodeLine;
    }
    else /* add after last and change last pointer */
    {
        codeImage->lastCodeLine->next = newCodeLine;
        codeImage->lastCodeLine = newCodeLine;
    }
    /* set next as first data line */
    newCodeLine->next = codeImage->firstDataLine;
    return;
}

/* gets data instruction line and process it and add to codeImage;
 * returns TRUE if succsessfull*/
bool addDataTocodeImage(char *lineEnd, char *instruction, int *DC, code_image *codeImage, input_line *line)
{
    code_line **firstDataLine = (code_line **)malloc(sizeof(code_line *)), *nextDataLine = NULL; /* DEBUG */
    bool success = TRUE;
    int *i = (int *)malloc(sizeof(int));
    i[0] = 0;
    firstDataLine[0] = NULL;
    if (isDataStorageInstruction(instruction)) /* is .data */
        success = addDataIntsToCodeImage(lineEnd, i, line, DC, firstDataLine, nextDataLine);
    else /* is .string */
        success = getCodeLinesFromString(lineEnd, i, line, DC, firstDataLine, nextDataLine);
    /* add to code image */
    if (success)
        addDataLinesTocodeImage(firstDataLine[0], codeImage);
    /* free */
    free(i);
    free(firstDataLine);
    return success;
}

/* gets integers from .data line, makes code lines. returns wether errors occured */
bool addDataIntsToCodeImage(char *lineEnd, int *i, input_line *line, int *DC, code_line **firstDataLine, code_line *nextDataLine)
{
    code_line *tempLine;
    int tempInt;
    do
    {
        tempInt = getNextIntFromIntString(lineEnd, i, line);
        /*check that int is legal ( not _ILLEGAL__HEXA__INT_ = -4097) */
        if (tempInt == _ILLEGAL__HEXA__INT_)
            return FALSE;
        tempLine = (code_line *)malloc(sizeof(code_line));
        tempLine->address = (DC[0])++;
        tempLine->code = (machine_code *)malloc(sizeof(machine_code));
        tempLine->code->hexaCode = intToHexaCode(tempInt);
        tempLine->code->ARE = ABSOLUTE;
        tempLine->next = NULL;
        if (firstDataLine[0] == NULL) /* if first one undefined */
            firstDataLine[0] = tempLine;
        else if (nextDataLine == NULL) /* if second one undefind */
        {
            nextDataLine = tempLine;
            firstDataLine[0]->next = nextDataLine;
        }
        else /* if first 2 lines already defined */
        {
            nextDataLine->next = tempLine;
            nextDataLine = tempLine;
        }
    } while (i[0] < strlen(lineEnd));
    return TRUE;
}

/* gets chars from .string line, makes code lines. returns wether errors occured */
bool getCodeLinesFromString(char *lineEnd, int *i, input_line *line, int *DC, code_line **firstDataLine, code_line *nextDataLine)
{
    char tempChar;
    code_line *tempLine;
    while ((tempChar = getNextcharFromDotString(lineEnd, i, line)) != EOF)
    {
        tempLine = (code_line *)malloc(sizeof(code_line));
        tempLine->address = (DC[0])++;
        tempLine->code = (machine_code *)malloc(sizeof(machine_code));
        tempLine->code->hexaCode = intToHexaCode((int)tempChar);
        tempLine->code->ARE = ABSOLUTE;
        tempLine->next = NULL;
        if (firstDataLine[0] == NULL) /* if first one defined */
            firstDataLine[0] = tempLine;
        else if (nextDataLine == NULL) /* if second one defind */
        {
            nextDataLine = tempLine;
            firstDataLine[0]->next = nextDataLine;
        }
        else /* if first 2 lines already defined */
        {
            nextDataLine->next = tempLine;
            nextDataLine = tempLine;
        }
        if (tempChar == '\0')
            break;
    }
    return (tempChar == EOF) ? FALSE : TRUE;
}

/* adds machine code lines to code image. makes sure last pointer is correct */
void addDataLinesTocodeImage(code_line *newDataLines, code_image *codeImage)
{
    code_line *temp;

    if (codeImage->firstDataLine == NULL) /* check if first entry is not null */
    {
        codeImage->firstDataLine = newDataLines;
        codeImage->lastDataLine = newDataLines;
    }
    else /* add after last */
        codeImage->lastDataLine->next = newDataLines;
    /* increment last pointer */
    for (temp = codeImage->lastDataLine; temp != NULL && temp->next != NULL; temp = temp->next)
        ;
    codeImage->lastDataLine = temp;
}

/* make machine code line from operand in first pass and return the line */
machine_code *makeLineFromOperandFirstPass(char *operand, operand_addressing_method addressingMethod, input_line *line)
{
    machine_code *codeLine;
    char *temp;
    if (addressingMethod == NO_OPERAND)
        return NULL;
    codeLine = (machine_code *)malloc(sizeof(machine_code));
    switch (addressingMethod)
    {
    case IMMEDIATE_ADDR: /* immediate addressing */
        codeLine->hexaCode = intToHexaCode(stringToInt(operand, line));
        codeLine->ARE = ABSOLUTE;
        break;
    case REGISTER_ADDR: /* direct register addressing */
        codeLine->hexaCode = intToHexaCode(1 << ((int)operand[1] - '0'));
        codeLine->ARE = ABSOLUTE;
        break;
    default: /* if relative or direct addressing, will be defined in second pass
              * meanwhile, stores the name of label to be replaced with its hexaCode*/
        temp = (char *)malloc(sizeof(char) * (strlen(operand) + 1));
        strcpy(temp, operand);
        codeLine->hexaCode = temp;
        codeLine->ARE = UNDEFINED;
        break;
    }
    return codeLine;
}

/* makes codeline from command based on opcode funct and addressing methods */
machine_code *makeLineFromCommand(command *newCommand)
{
    int n, k, m, l;
    machine_code *codeLine;
    if (newCommand->destinationOperandAddressingMethod == NO_OPERAND)
        n = 0;
    else
        n = newCommand->destinationOperandAddressingMethod;
    if (newCommand->originOperandAddressingMethod == NO_OPERAND)
        k = 0;
    else
        k = newCommand->originOperandAddressingMethod;
    m = newCommand->funct;
    l = newCommand->opcode;
    codeLine = (machine_code *)malloc(sizeof(machine_code));
    codeLine->hexaCode = intToHexaCode(n + (k * 4) + (m * 16) + (l * 256));
    codeLine->ARE = ABSOLUTE;
    return codeLine;
}

/* frees a line of code */
void freeCodeLine(code_line *codeLine)
{
    if (codeLine == NULL) /* no need to free NULL */
        return;
    if (codeLine->code->hexaCode != NULL)
    {
        free(codeLine->code->hexaCode);
        codeLine->code->hexaCode = NULL;
    }
    if (codeLine->code != NULL)
    {

        free(codeLine->code);
        codeLine->code = NULL;
    }
    if (codeLine != NULL)
    {
        free(codeLine);
        codeLine = NULL;
    }
}

/* gets symbol table entry by name and returns it. if it doesn't exist, returns null */
symbol_table *getEntryByName(symbol_table *symbolTable, char *label)
{
    symbol_table *entry = symbolTable;
    while (entry != NULL)
    {
        if (!strcmp(label, entry->symbol))
            return entry;
        entry = entry->next;
    }
    return NULL;
}

/* frees code image */
void freeCodeImage(code_image *codeImage)
{
    code_line *temp;
    if (codeImage == NULL)
        return;
    codeImage->currCodeLine = codeImage->firstCodeLine;
    while (codeImage->currCodeLine != NULL)
    {
        temp = codeImage->currCodeLine->next;
        freeCodeLine(codeImage->currCodeLine);
        codeImage->currCodeLine = temp;
    }
    free(codeImage);
}

/* frees symbol table */
void freeSymbolTable(symbol_table *symbolTable)
{
    symbol_table *temp;
    while (symbolTable != NULL)
    {
        temp = symbolTable->next;
        free(symbolTable->symbol);
        free(symbolTable);
        symbolTable = temp;
    }
}

/* adds the final IC to symbol table entries with data attribute */
void addICFToSymbolTableDataEntries(int ICF, symbol_table *symbolTable)
{
    symbol_table *temp;
    for (temp = symbolTable; temp != NULL && temp->next != NULL; temp = temp->next)
    {
        if (temp->attribute == DATA_ATT)
            temp->value = temp->value + ICF;
    }
}

/* adds the final IC to codeImage data lines */
void addICFToCodeImageLines(int ICF, code_image *codeImage)
{
    /* connect code and data lines */
    codeImage->lastCodeLine->next = codeImage->firstDataLine;
    /* add ICF to each data line */
    for (codeImage->currCodeLine = codeImage->firstDataLine; codeImage->currCodeLine->next != NULL; codeImage->currCodeLine = codeImage->currCodeLine->next)
        codeImage->currCodeLine->address += ICF;
    codeImage->currCodeLine->address += ICF;
    /* reset current code line for traversing in second pass */
    codeImage->currCodeLine = codeImage->firstCodeLine;
}

/* check is a label exists in symbol table and return true if so */
bool containsSymbol(char *label, symbol_table *symbolTable)
{
    return getEntryByName(symbolTable, label) == NULL ? FALSE : TRUE;
}
