/* first pass header */

#ifndef _FIRST__PASS__HEADR_
#define _FIRST__PASS__HEADR_

/* process line if first pass; return TRUE if no errors found */
bool firstPassProcessLine(symbol_table **symbolTable, code_image *codeImage, int *IC, int *DC, input_line *line);

/* check instruction type in first pass and process accordingly. returns wether successfull */
bool checkInstructionFirstPass(char *label, char *instruction, char *lineEnd, bool labelFlag, symbol_table **symbolTable, code_image *codeImage, int *IC, int *DC, input_line *line, int *j);

/* gets processed command and creates code lines, then adds them to code image*/
void addCommandToImageCodeFirstPass(command *newCommand, int *IC, code_image *codeImage, input_line *line);

/* process code instruction to command in first pass and returns wether successful*/
bool processCodeInstructionFirstPass(char *commandName, char *operands, int *IC, code_image *codeImage, input_line *line);

/* creates command from line first pass
returns the command if created successfully, otherwise returns null */
command *processCommandFirstPass(char *commandName, char *firstOp, char *secondOp, input_line *line);

#endif