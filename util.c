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

/* get next word from string 'from' starting at index i and stores it in 'to' */
void getNextWord(char *from, int *startIndex, char *to)
{
    int j;
    for (j = 0; from[startIndex[0] + j] != '\0'; ++j)
    {
        if (from[startIndex[0] + j] == ' ' || from[startIndex[0] + j] == '\t' || from[startIndex[0] + j] == '\n' || from[startIndex[0] + j] == '\r')
            break;
        else
            to[j] = from[startIndex[0] + j];
    }
    to[j] = '\0';
    startIndex[0] = startIndex[0] + (j);
    _SKIP__TO__NON__WHITE_(from, startIndex[0])
}

/* get the rest of string starting from startIndex */
void getRestOfLine(char *from, int startIndex, char *to)
{
    int j;
    for (j = 0; from[startIndex + j] != '\0'; ++j)
    {
        if (from[startIndex + j] == '\0' || (from[startIndex + j] == '\r' && from[startIndex + j + 1] == '\n'))
        {
            to[j] = '\0';
            return;
        }
        else
            to[j] = from[startIndex + j];
    }
    to[j] = '\0';
}

/* checks if the last character is ':' and return TRUE if so */
bool isLabel(char *string)
{
    return string[strlen(string) - 1] == ':' ? TRUE : FALSE;
}

/* instructions functions: list of functions that compare string to instructions names and return TRUE is they match*/

bool isStorageIsntruction(char *instruction)
{
    return (isDataStorageInstruction(instruction) || isStringStorageInstruction(instruction)) ? TRUE : FALSE;
}

bool isConnectionInstruction(char *instruction)
{
    return (isEntryInstruction(instruction) || isExternInstruction(instruction)) ? TRUE : FALSE;
}

bool isEntryInstruction(char *instruction)
{
    return strcmp(instruction, ".entry") ? FALSE : TRUE;
}

bool isExternInstruction(char *instruction)
{
    return strcmp(instruction, ".extern") ? FALSE : TRUE;
}

bool isDataStorageInstruction(char *instruction)
{
    return strcmp(instruction, ".data") ? FALSE : TRUE;
}

bool isStringStorageInstruction(char *instruction)
{
    return strcmp(instruction, ".string") ? FALSE : TRUE;
}

bool isStringInstruction(char *instruction)
{
    return (isStorageIsntruction(instruction) || isConnectionInstruction(instruction)) ? TRUE : FALSE;
}

/* converts int to string of 3 hexadecimal digits */
char *intToHexaCode(int n)
{
    char *hexa = (char *)malloc(sizeof(char) * 4);
    strcpy(hexa, "000");
    if (n < 0)
        n += 4096;
    hexa[2] = intToHexaChar(n % 16);
    n = n / 16;
    hexa[1] = intToHexaChar(n % 16);
    n = n / 16;
    hexa[0] = intToHexaChar(n);
    return hexa;
}
/*converts int to hexadecimal char; only accepts 0-15 */
char intToHexaChar(int n)
{
    return ((n < 10) ? ('0' + n) : ('A' + n - 10));
}

/* converts int to four digit string for output files*/
char *intToFourDigitString(int n)
{
    char *intString = (char *)malloc(sizeof(char) * 6); /* freed in addEntrytoOutputFile,makeObFileFromCodeImage */
    int i;
    for (i = 3; i > -1; --i)
    {
        intString[i] = ('0' + (n % 10));
        n /= 10;
    }
    intString[4] = ' ';
    intString[5] = '\0';
    return intString;
}
/* remove comma from string representing the first operand of line */
void removeCommaFromOperand(char *operand)
{
    int i;
    if (operand == NULL)
        return;
    for (i = 0; operand[i] != '\0' && i < strlen(operand); ++i)
    {
        if (operand[i] == ',' && operand[i + 1] == '\0')
        {
            operand[i] = '\0';
            break;
        }
    }
}

/* reads int from string starting at i 
 * return -4097 for error while parsing ; increments i to next non white after comma seperating ints;
 * also checks that after last int there are no commas*/
int getNextIntFromIntString(char *lineEnd, int *i, input_line *line)
{
    char *intString = malloc((strlen(lineEnd) + 1) * sizeof(char));
    int j = 0, num = 0;
    if (i[0] == 0)
    { /* first int read from lineEnd; make sure it start with number and not comma */
        _SKIP__TO__NON__WHITE_(lineEnd, i[0])
        if (lineEnd[i[0]] == ',')
        {
            printf("%d : %s\tlist of ints %s starts with comma \n", line->number, line->content, lineEnd);
            j = _ILLEGAL__HEXA__INT_;
        }
    }
    while (lineEnd[i[0]] != ' ' && lineEnd[i[0]] != '\t' && lineEnd[i[0]] != ',' && lineEnd[i[0]] != '\0' && lineEnd[i[0]] != '\n' && lineEnd[i[0]] != '\r' && j < strlen(lineEnd))
        intString[j++] = lineEnd[i[0]++];
    intString[j] = '\0';
    _SKIP__TO__NON__WHITE_(lineEnd, i[0])
    if (lineEnd[i[0]] != ',')
    {
        if (lineEnd[i[0]] != '\0' && lineEnd[i[0]] != '\r' && lineEnd[i[0]] != '\n')
        {
            printf("%d : %s\tmissing comma between ints %s  \n", line->number, line->content, lineEnd);
            num = _ILLEGAL__HEXA__INT_;
        }
    }
    else
    {
        ++(i[0]);
        _SKIP__TO__NON__WHITE_(lineEnd, i[0])
    }
    if (num != _ILLEGAL__HEXA__INT_)
        num = stringToInt(intString, line);
    if (intString != NULL)
    {
        free(intString);
        intString = NULL;
    }
    return num;
}

/* gets next character from string in .string line 
 * returns EOF if error occured, \0 if done and the next char is " ,otherwise and increments to line end and checks for errors when seeing end of string (") */
char getNextcharFromDotString(char *lineEnd, int *i, input_line *line)
{
    if (i[0] == 0)
    {
        _SKIP__TO__NON__WHITE_(lineEnd, i[0])
        if (lineEnd[i[0]] != '\"' || strchr(lineEnd + i[0] + 1, '\"') == NULL)
        { /* if first char of string, check that it starts with " and return next*/
            printf("%d : %s\t%s expected string \n", line->number, line->content, lineEnd);
            return EOF;
        }
        else
        {
            ++(i[0]);
            return lineEnd[i[0]++];
        }
    }
    else /* if not first char */
    {
        if (lineEnd[i[0]] == '\"') /* if is end of string ("), check that rest of line is clear */
        {
            ++i[0];
            _SKIP__TO__NON__WHITE_(lineEnd, i[0])
            if (lineEnd[i[0]] != '\0' && lineEnd[i[0]] != '\r' && lineEnd[i[0]] != '\n')
            {
                printf("%d : %s\t%s extra text after string definition \n", line->number, line->content, lineEnd);
                return EOF;
            }
            else
                return '\0';
        }
        else /* some char in the middle */
            return lineEnd[i[0]++];
    }
}

/* converts in to string and returns new allocated string */
char *intToString(int n)
{
    char *nReverseString = (char *)malloc(sizeof(char) * 10);
    char *nString = (char *)malloc(sizeof(char) * 10);
    int current;
    /* create number as string in reverse */
    int len = 0;
    if (n == 0)
    {
        nReverseString[len] = '\0';
        len++;
    }
    while (n > 0)
    {
        nReverseString[len] = '0' + (n % 10);
        n = n / 10;
        len++;
    }
    /* reverse the string */
    current = len - 1;
    while (current >= 0)
    {
        nString[len - current - 1] = nReverseString[current];
        current--;
    }
    nString[len] = '\0';
    free(nReverseString);
    return nString;
}

/* takes string containing integer and return it as int; returns -4097 (out of range) if error */
int stringToInt(char *string, input_line *line)
{
    bool negative;
    int n = 0, i = 0;
    if ((negative = string[0] == '-')) /* checks if negative */
        ++i;
    n = (string[i] - '0');
    if (n == 0)
    {
        printf("%d : %s\tnumber %s starts with 0\n", line->number, line->content, string);
        return _ILLEGAL__HEXA__INT_;
    }
    ++i;
    while (string[i] != '\0')
    {
        if (!isdigit(string[i])) /* check if next char is num */
        {
            printf("%d : %s\t%s is not a number 0\n", line->number, line->content, string);
            return _ILLEGAL__HEXA__INT_;
        }
        n *= 10;
        n += (string[i++] - '0');
    }
    if (negative)
        n *= (-1);
    if (n > _MAX__HEXA__INT_ || n < _MIN__HEXA__INT_)
    { /* if too big for 12 bit representation */
        printf("%d : %s\tnumber %s is too big to be represented by 12 bit signed int\n", line->number, line->content, string);
        return _ILLEGAL__HEXA__INT_;
    }
    return n;
}