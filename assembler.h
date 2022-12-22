/* assembler header */

#ifndef _ASSEMBLER__HEADER_
#define _ASSEMBLER__HEEADR_

/* process assembly file; return TRUE if assembly file was successfully processed*/
bool assemblerProcessFile(char *fileName);

/* free after second pass */
void freeAfterSecondPass(symbol_table **symbolTable, input_line *line, FILE *inputFile, FILE *extFile, FILE *entFile);

/* free if unsuccessfull */
void freeFilesIfUnsuccessfull(char *temp, char *fileName);

/* free at end */
void freeAtEnd(char *temp, int *IC, int *DC, code_image *codeImage);

#endif