/*  */

#ifndef _SECOND__PASS__HEADER_
#define _SECOND__PASS__HEADER_


/*  process a line of input code in second pass
    return true if no errors found while processing the line */
bool secondPassProcessLine(symbol_table **symbolTable, code_image *codeImage, input_line *line, FILE *entFile, FILE *extFile);

/* process command in second pass. return wether successfull*/
bool processCodeInstructionSecondPass(char *commandName, char *operands, code_image *codeImage, symbol_table *symbolTable, FILE *extFile, input_line *line);

/* creates command from line second pass. return created command or null if failed*/
command *processCommandSecondPass(char *commandName, char *firstOp, char *secondOp, input_line *line);

/* fills undefined lines in codeImage. returns wether successfull*/
bool addCommandToImageCodeSecondPass(command *newCommand, code_image *codeImage, symbol_table *symbolTable, FILE *extFile, input_line *line);

/* process operand in second pass and adjust codeImage if nessesary. return wether successfull */
bool processOperandSecondPass(code_image *codeImage, operand_addressing_method operandAddressingMethod, char *operand, symbol_table *symbolTable, FILE *extFile, input_line *line);

#endif