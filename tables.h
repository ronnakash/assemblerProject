/* tables */

#ifndef _TABLES__HEADER_
#define _TABLES__HEADER_

/* struct declerations */

typedef enum symbol_attribute
{
    /*  */
    DATA_ATT,
    /*  */
    EXTERN_ATT,
    /*  */
    CODE_ATT
} symbol_attribute;

typedef struct symbol_table_t symbol_table;

/* symbol table */
typedef struct symbol_table_t
{
    /* symbol name */
    char *symbol;
    /* IC value in codeImage */
    int value;
    /* attribute (data,external,code) */
    symbol_attribute attribute;
    /* next */
    symbol_table *next;
} symbol_table_t;

typedef enum command_opcode
{
    MOV_OP = 0,
    CMP_OP = 1,
    ADD_OP = 2,
    SUB_OP = 2,
    LEA_OP = 4,
    CLR_OP = 5,
    NOT_OP = 5,
    INC_OP = 5,
    DEC_OP = 5,
    JMP_OP = 9,
    BNE_OP = 9,
    JSR_OP = 9,
    RED_OP = 12,
    PRN_OP = 13,
    RTS_OP = 14,
    STOP_OP = 15
} command_opcode;

typedef enum command_funct
{
    /* OPCODE 2 */
    ADD_FUNCT = 10,
    SUB_FUNCT = 11,

    /* OPCODE 5 */
    CLR_FUNCT = 10,
    NOT_FUNCT = 11,
    INC_FUNCT = 12,
    DEC_FUNCT = 13,

    /* OPCODE 9 */
    JMP_FUNCT = 10,
    BNE_FUNCT = 11,
    JSR_FUNCT = 12,

    /** Default (No need/Error) */
    NONE_FUNCT = 0
} command_funct;

typedef enum ARE_property
{
    /* absolute */
    ABSOLUTE,
    /* relocatatable */
    RELOCATEABLE,
    /* external */
    EXTERNAL,
    /* unknown yet(first pass) */
    UNDEFINED
} ARE_property;

typedef enum command_addressing_method
{
    /* no operand */
    NONE_ADDR,
    /* 1 */
    DIRECT_ONLY_ADDR,
    /* 0,1,3 */
    NON_RELATIVE_ADDR,
    /* 1,2 */
    DIRECT_OR_RELATIVE_ADDR,
    /* 1,3 */
    DIRECT_OR_REGISTER_ADDR
} command_addressing_method;

/* command table to process commands */

typedef struct command_table
{
    char name[16][5];
    command_opcode opcodes[16];
    command_funct functs[16];
    command_addressing_method originOperandAddressingMethods[16];
    command_addressing_method destinationOperandAddressingMethods[16];
} command_table;


typedef enum operand_addressing_method
{
    /* immediate */
    IMMEDIATE_ADDR = 0,
    /* direct */
    DIRECT_ADDR = 1,
    /* relative*/
    RELATIVE_ADDR = 2,
    /* */
    REGISTER_ADDR = 3,
    /* no operand */
    NO_OPERAND = -1,
    /* illegal symbol */
    ILLEGAL_SYMBOL = -2
} operand_addressing_method;

/* struct for analysed command */
typedef struct command
{
    command_opcode opcode;
    command_funct funct;
    operand_addressing_method originOperandAddressingMethod;
    char *originOperand;
    operand_addressing_method destinationOperandAddressingMethod;
    char *destinationOperand;
} command;

/* machine code of code image line */
typedef struct machine_code
{
    char *hexaCode;
    ARE_property ARE;

} machine_code;

typedef struct code_line_t code_line;

/* line of code for code image */
typedef struct code_line_t
{
    /* */
    int address;
    /* data */
    machine_code *code;
    /* */
    code_line *next;
} code_line_t;

/* the code image of an input file */
typedef struct code_image
{
    /* code */
    code_line *firstCodeLine;
    code_line *lastCodeLine;
    /* data */
    code_line *firstDataLine;
    code_line *lastDataLine;
    /* pointer for second pass traversal; TODO: set to firstCodeLine in assembler func between first and second pass */
    code_line *currCodeLine;
} code_image;

/* functions */


/* already checked label is valid and colon removed */
symbol_table *addToSymbolTable(char *label, char *instruction, int value, symbol_table *symbolTable, input_line *line, symbol_attribute attributeSymbol);

/* gets the appropriate attribute for given instruction */
symbol_attribute getSymbolAttribute(char *instruction);

/* remove : from end of label string */
void removeColonFromLabel(char *label);

/* check if label is valid and not already defined*/
bool isValidLabel(char *label, symbol_table *symbolTable, input_line *line);

/* adds a line of code to the codeImage */
void addCodeLineToCodeImage(machine_code *codeLine, int *IC, code_image *codeImage);

/* gets data instruction line and process it and add to codeImage;
 * returns TRUE if succsessfull*/
bool addDataTocodeImage(char *lineEnd, char *instruction, int *DC, code_image *codeImage, input_line *line);

/* gets integers from .data line, makes code lines. returns wether errors occured */
bool addDataIntsToCodeImage(char *lineEnd, int *i, input_line *line, int *DC, code_line **firstDataLine, code_line *nextDataLine);

/* gets chars from .string line, makes code lines. returns wether errors occured */
bool getCodeLinesFromString(char *lineEnd, int *i, input_line *line, int *DC, code_line **firstDataLine, code_line *nextDataLine);

/* adds machine code lines to code image. makes sure last pointer is correct */
void addDataLinesTocodeImage(code_line *newDataLines, code_image *codeImage);

/* make machine code line from operand in first pass and return the line */
machine_code *makeLineFromOperandFirstPass(char *operand, operand_addressing_method addressingMethod, input_line *line);

/* makes codeline from command based on opcode funct and addressing methods */
machine_code *makeLineFromCommand(command *newCommand);

/* frees a line of code */
void freeCodeLine(code_line *codeLine);

/* gets symbol table entry by name and returns it. if it doesn't exist, returns null */
symbol_table *getEntryByName(symbol_table *symbolTable, char *label);

/* frees code image */
void freeCodeImage(code_image *codeImage);

/* frees symbol table */
void freeSymbolTable(symbol_table *symbolTable);

/* adds the final IC to symbol table entries with data attribute */
void addICFToSymbolTableDataEntries(int ICF, symbol_table *symbolTable);

/* adds the final IC to codeImage data lines */
void addICFToCodeImageLines(int ICF, code_image *codeImage);

/* check is a label exists in symbol table and return true if so */
bool containsSymbol(char *label, symbol_table *symbolTable);

#endif