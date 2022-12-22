/* input output header */

#ifndef _IO__HEADER_
#define _IO__HEADER_

/* makes the .ob file from codeImage; also frees codeImage */
void makeObFileFromCodeImage(code_image *codeImage, char *fileName);

/* add the entry to .ent file */
bool addEntrytoOutputFile(char *entryName, symbol_table *symbolTable, FILE *entFile, input_line *line);

/* add the entry to the external file */
void addToExternalFile(FILE *extFile, char *label, int address);

/* remove .ext and .ent files if thy are empty at the end of processing */
void removeEmptyFiles(char *fileName);

/* remove a file if it is empty */
void freeFileIfEmpty(char *fileName);

#endif
