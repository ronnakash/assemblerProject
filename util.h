/* utils */

#define _MAX__LINE__SIZE_ 80

#define _IC__INIT_ 100

#define _DC__INIT_ 0

#define _MIN__HEXA__INT_ -4096

#define _MAX__HEXA__INT_ 2047

#define _ILLEGAL__HEXA__INT_ -4097

#ifndef _UTIL__HEADER_
#define _UTIL__HEADER_

/* moves the index to the next place in string where the char isn't white */
#define _SKIP__TO__NON__WHITE_(string, index)                                                   \
    for (; string[(index)] && (string[(index)] == '\t' || string[(index)] == ' ' || string[(index)] == '\r' ||  string[(index)] == '\n' ); (++(index))) \
        ;

/* moves the index first white character after next word */
#define _SKIP__OVER__NEXT__WORD_(string, index)                                                 \
    for (; string[(index)] && (string[(index)] != '\t' || string[(index)] != ' ' && string[(index)] != '\r' &&  string[(index)] != '\n' ); (++(index))) \
        ;

/* moves the index to the next place in string where the char isn't white */
#define _SKIP__TO__NEXT__WORD_(string, index)                                                   \
    for (; string[(index)] && (string[(index)] != '\t' || string[(index)] != ' ' && string[(index)] != '\r' &&  string[(index)] != '\n' ); (++(index))) \
        ;                                                                                       \
    for (; string[(index)] && (string[(index)] == '\t' || string[(index)] == ' ' || string[(index)] == '\r' ||  string[(index)] == '\n' ); (++(index))) \
        ;

/* boolean enum */
typedef enum bool
{
    FALSE = 0,
    TRUE = 1
} bool;

/* struct that holds line of assembly code from input file */
typedef struct input_line
{
    /* line number in input file */
    int number;
    /* contents of the line */
    char *content;
    /* file name */
    char *file;
} input_line;

/* get next word from string 'from' starting at index i and stores it in 'to' */
void getNextWord(char *from, int *startIndex, char *to);

/* get the rest of string starting from startIndex */
void getRestOfLine(char *from, int startIndex, char *to);

/* checks if the last character is ':' and return TRUE if so */
bool isLabel(char *string);

/* instructions functions: list of functions that compare string to instructions names and return TRUE is they match*/
bool isStorageIsntruction(char *instruction);

bool isConnectionInstruction(char *instruction);

bool isEntryInstruction(char *instruction);

bool isExternInstruction(char *instruction);

bool isDataStorageInstruction(char *instruction);

bool isStringStorageInstruction(char *instruction);

bool isStringInstruction(char *instruction);

/* converts int to string of 3 hexadecimal digits */
char *intToHexaCode(int n);

/*converts int to hexadecimal char; only accepts 0-15 */
char intToHexaChar(int n);

/* converts int to four digit string for output files*/
char *intToFourDigitString(int n);

/* remove comma from string representing the first operand of line */
void removeCommaFromOperand(char *operand);

/* reads int from string starting at i 
 * return -4097 for error while parsing ; increments i to next non white after comma seperating ints;
 * also checks that after last int there are no commas*/
int getNextIntFromIntString(char *lineEnd, int *i, input_line *line);

/* gets next character from string in .string line 
 * returns EOF if error occured, \0 if done and the next char is " ,otherwise and increments to line end and checks for errors when seeing end of string (") */
char getNextcharFromDotString(char *lineEnd, int *i, input_line *line);

/* converts in to string and returns new allocated string */
char *intToString(int n);

/* takes string containing integer and return it as int; returns -4097 (out of range) if error */
int stringToInt(char *string, input_line *line);

#endif
