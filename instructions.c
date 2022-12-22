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

extern command_table commandTable;

/* check if operand addressing method matches commands; returns TRUE if so */
bool compatibleAddressingMethods(operand_addressing_method operandAddressingMethod,
                                 command_addressing_method commandAddressingMethod, input_line *line)
{
    switch (operandAddressingMethod)
    {
    case IMMEDIATE_ADDR:
        if (commandAddressingMethod != NON_RELATIVE_ADDR)
        {
            printf("%d : %s\tcommand does not accept immediate addressing for this operand\n", line->number, line->content);
            return FALSE;
        }
        return TRUE;
    case DIRECT_ADDR:
        if (commandAddressingMethod == NONE_ADDR)
        {
            printf("%d : %s\textra text after command\n", line->number, line->content);
            return FALSE;
        }
        return TRUE;
    case RELATIVE_ADDR:
        if (commandAddressingMethod != DIRECT_OR_RELATIVE_ADDR)
        {
            printf("%d : %s\tcommand does not accept relative addressing for this operand\n", line->number, line->content);
            return FALSE;
        }
        return TRUE;
    case REGISTER_ADDR:
        if (commandAddressingMethod != DIRECT_OR_REGISTER_ADDR && commandAddressingMethod != NON_RELATIVE_ADDR)
        {
            printf("%d : %s\tcommand does not accept register as operand\n", line->number, line->content);
            return FALSE;
        }
        return TRUE;
    case NO_OPERAND:
        return (commandAddressingMethod == NONE_ADDR) ? TRUE : FALSE;
    case ILLEGAL_SYMBOL:
        printf("%d : %s\toperand is illegal symbol\n", line->number, line->content);
        return FALSE;
    default:
        printf("%d : %s\tunexpected symbol reached while parsing addressing method for operand\n", line->number, line->content);
        return FALSE;
    }
}

/* like previous function, but with no print for errors (for second pass) */
bool compatibleAddressingMethodsMute(operand_addressing_method operandAddressingMethod,
                                     command_addressing_method commandAddressingMethod)
{
    switch (operandAddressingMethod)
    {
    case IMMEDIATE_ADDR:
        return (commandAddressingMethod == NON_RELATIVE_ADDR) ? TRUE : FALSE;
    case DIRECT_ADDR:
        return (commandAddressingMethod != NONE_ADDR) ? TRUE : FALSE;
    case RELATIVE_ADDR:
        return (commandAddressingMethod == DIRECT_OR_RELATIVE_ADDR) ? TRUE : FALSE;
    case REGISTER_ADDR:
        return (commandAddressingMethod == DIRECT_OR_REGISTER_ADDR || commandAddressingMethod == NON_RELATIVE_ADDR) ? TRUE : FALSE;
    case NO_OPERAND:
        return (commandAddressingMethod == NONE_ADDR) ? TRUE : FALSE;
    case ILLEGAL_SYMBOL:
        return FALSE;
    default:
        return FALSE;
    }
}

/* checks operand and returns it's addressing method */
operand_addressing_method getOperandAddressingMethod(char *operand) /*TODO: make func for each addressing */
{
    /* if null, no operand */
    int i, length;
    if (operand == NULL || operand[0] == '\0')
        return NO_OPERAND;
    length = strlen(operand);
    /* if is register (direct register addressing) */
    if (operand[0] == 'r' && (operand[1] >= '0' && operand[1] <= '7' && operand[2] == '\0'))
        return REGISTER_ADDR;
    /* if starts with # then a number (immediate addressing) */
    /* also removes # if immediate addressing */
    if (operand[0] == '#' && (isdigit(operand[1]) || operand[1] == '-'))
    {
        for (i = 2; i < length; ++i)
        {
            if (!isdigit(operand[i]))
                return ILLEGAL_SYMBOL;
        }
        /* remove # */
        for (i = 0; i < length - 1; ++i)
        {
            operand[i] = operand[i + 1];
        }
        operand[i] = '\0';
        return IMMEDIATE_ADDR;
    }
    /* if is valid label (direct addressing) */
    if (isalpha(operand[0]))
    {
        i = 1;
        while (1)
        {
            if (operand[i] == '\0')
                return DIRECT_ADDR;
            if (!isalnum(operand[i]))
                return ILLEGAL_SYMBOL;
            ++i;
        }
    }
    /* if starts with % then valid label (relative addressing) */
    /*  also removes % if relative addressing*/
    if (operand[0] == '%' && isalpha(operand[1]))
    {
        for (i = 2; i < length; ++i)
        {
            if (!isalnum(operand[i]))
                return ILLEGAL_SYMBOL;
        }
        /* remove % */
        for (i = 0; i < length; ++i)
        {
            operand[i] = operand[i + 1];
        }
        operand[i] = '\0';
        return RELATIVE_ADDR;
    }
    return ILLEGAL_SYMBOL;
}

/* check if label is a saved keyword */
bool isKeyword(char *label)
{
    int i;
    for (i = 0; i < 16; ++i)
    {
        if (!strcmp(label, commandTable.name[i]))
            return TRUE;
    }
    if (((isEntryInstruction(label)) == TRUE || (strlen(label) == 2 && (label[0] == 'r' && (label[1] >= '0' && label[1] <= '7') && label[2] == '\0')))) /* if register */
        return TRUE;
    return FALSE;
}

/* returns the correct char for ARE*/
char getAREChar(ARE_property are)
{
    switch (are)
    {
    case ABSOLUTE:
        return 'A';
    case RELOCATEABLE:
        return 'R';
    default:
        return 'E';
    }
}