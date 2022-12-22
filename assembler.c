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

/* assembler c project - Ron Nakash */

/* assembler main */
int main(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; ++i)
    {
        if (assemblerProcessFile(argv[i]) == FALSE)
            printf("failed file %s\n", argv[i]);
    }
    return 0;
}

/* process assembly file; return TRUE if assembly file was successfully processed*/

bool assemblerProcessFile(char *fileName)
{
    int *IC, *DC; /* instruction and data counters */
    bool success = TRUE, done = FALSE;
    input_line *line;                    /* stores input line */
    FILE *inputFile, *entFile, *extFile; /* stores output files */
    symbol_table **symbolTable;          /* symbol table*/
    code_image *codeImage;               /* code image */
    char *temp = (char *)malloc(strlen(fileName) + 5);
    /* try to open file; also add .as to file name */
    strcpy(temp, fileName);
    strcat(temp, ".as");
    inputFile = fopen(temp, "r");
    if (inputFile == NULL)
    {
        printf("can't open %s. skipping it\n", temp);
        free(temp);
        return FALSE;
    }
    /* initialise stuff before first pass */
    IC = (int *)malloc(sizeof(int));
    DC = (int *)malloc(sizeof(int));
    line = (input_line *)malloc(sizeof(input_line));
    IC[0] = _IC__INIT_;
    DC[0] = _DC__INIT_;
    line->number = 1;
    line->file = fileName;
    line->content = (char *)malloc(_MAX__LINE__SIZE_ + 1);
    codeImage = (code_image *)malloc(sizeof(code_image));
    symbolTable = (symbol_table **)malloc(sizeof(symbol_table *));
    symbolTable[0] = NULL;
    codeImage->currCodeLine = NULL;
    codeImage->firstCodeLine = NULL;
    codeImage->firstDataLine = NULL;
    codeImage->lastCodeLine = NULL;
    codeImage->lastDataLine = NULL;
    /* first pass */
    while (!done)
    {
        fgets(line->content, _MAX__LINE__SIZE_ + 2, inputFile);
        /* check if line too long (doesn't contain \n and EOF not reached yet) */
        if (strchr(line->content, '\n') == NULL && !feof(inputFile))
        {
            printf("line %d is too long\n", line->number);
            do
            { /* skip until new line or EOF */
                fgets(line->content, _MAX__LINE__SIZE_ + 2, inputFile);
            } while (strchr(line->content, '\n') == NULL && !feof(inputFile));
        }
        if (feof(inputFile))
            done = TRUE;
        success = (firstPassProcessLine(symbolTable, codeImage, IC, DC, line) && success);
        line->number += 1;
    }
    /* initialise stuff before second pass */
    done = FALSE;
    addICFToSymbolTableDataEntries(IC[0], symbolTable[0]);
    addICFToCodeImageLines(IC[0], codeImage);
    line->number = 1;
    rewind(inputFile);
    strcpy(temp, fileName);
    strcat(temp, ".ent");
    entFile = fopen(temp, "w+");
    strcpy(temp, fileName);
    strcat(temp, ".ext");
    extFile = fopen(temp, "w+");
    /* start of second pass */
    while (!done)
    {
        fgets(line->content, _MAX__LINE__SIZE_ + 2, inputFile);
        /* check if line too long (doesn't contain \n and EOF not reached yet) */
        if (strchr(line->content, '\n') == NULL && !feof(inputFile))
        {
            do
            { /* skip until new line or EOF */
                fgets(line->content, _MAX__LINE__SIZE_ + 2, inputFile);
            } while (strchr(line->content, '\n') == NULL && !feof(inputFile));
        }
        if (feof(inputFile))
            done = TRUE;
        success = (secondPassProcessLine(symbolTable, codeImage, line, entFile, extFile) && success);
        line->number += 1;
    }
    /* done with processing input file */
    /* free after second pass */
    freeAfterSecondPass(symbolTable, line, inputFile, extFile, entFile);
    if (success)
    {
        /* make .ob output file; also frees codeimage*/
        makeObFileFromCodeImage(codeImage, fileName);
        codeImage = NULL;
    }
    else /* delete .ent and .ext files and free FILE pointers*/
        freeFilesIfUnsuccessfull(temp, fileName);
    /* free att end */
    freeAtEnd(temp, IC, DC, codeImage);
    if (success == TRUE)
        removeEmptyFiles(fileName);
    return success;
}

/* free after second pass */
void freeAfterSecondPass(symbol_table **symbolTable, input_line *line, FILE *inputFile, FILE *extFile, FILE *entFile)
{
    freeSymbolTable(symbolTable[0]);
    free(symbolTable);
    free(line->content);
    free(line);
    fclose(inputFile);
    fclose(entFile);
    fclose(extFile);
}

/* free if unsuccessfull */
void freeFilesIfUnsuccessfull(char *temp, char *fileName)
{
    strcpy(temp, fileName);
    strcat(temp, ".ent");
    remove(temp);
    strcpy(temp, fileName);
    strcat(temp, ".ext");
    remove(temp);
}

/* free at end */
void freeAtEnd(char *temp, int *IC, int *DC, code_image *codeImage)
{
    free(temp);
    free(IC);
    free(DC);
    freeCodeImage(codeImage);
}
