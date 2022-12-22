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

/*  process a line of input code in second pass
    return true if no errors found while processing the line */
bool secondPassProcessLine(symbol_table **symbolTable, code_image *codeImage, input_line *line, FILE *entFile, FILE *extFile)
{
    char *label, *lineEnd, *instruction, *temp;
    bool success = TRUE;
    int *i, *j;
    i = (int *)malloc(sizeof(int));
    i[0] = 0;
    j = (int *)malloc(sizeof(int));
    j[0] = 0;
    /* skip to non white character */
    _SKIP__TO__NON__WHITE_(line->content, i[0])
    /* check for comment */
    if (line->content[i[0]] == ';' || line->content[i[0]] == '\0')
    {
        free(i);
        free(j);
        return TRUE;
    }
    label = (char *)malloc(sizeof(char) * _MAX__LINE__SIZE_ + 1);
    lineEnd = (char *)malloc(sizeof(char) * _MAX__LINE__SIZE_ + 1);
    instruction = (char *)malloc(sizeof(char) * _MAX__LINE__SIZE_ + 1);
    /* check for label */
    getNextWord(line->content, i, label);
    if (isLabel(label))
        getNextWord(line->content, i, instruction);
    else
        strcpy(instruction, label);
    getRestOfLine(line->content, i[0], lineEnd);
    /* check instruction type */
    if (isExternInstruction(instruction) || isStorageIsntruction(instruction)) /* handled fully on first pass */
    {
        success = TRUE;
    }
    else if (isEntryInstruction(instruction))
    {
        temp = malloc(sizeof(char) * 70);
        getNextWord(lineEnd, j, temp);
        if (j[0] < strlen(lineEnd))
        {
            printf("%d : %s\texcess text after command\n", line->number, line->content);
            success = FALSE;
        }
        else
            success = addEntrytoOutputFile(temp, symbolTable[0], entFile, line);
        free(temp);
    }
    else
        success = processCodeInstructionSecondPass(instruction, lineEnd, codeImage, symbolTable[0], extFile, line);
    /* free */
    free(label);
    free(instruction);
    free(lineEnd);
    free(i);
    free(j);
    return success;
}

/* process command in second pass. return wether successfull*/
bool processCodeInstructionSecondPass(char *commandName, char *operands, code_image *codeImage, symbol_table *symbolTable, FILE *extFile, input_line *line)
{
    bool success = TRUE;
    char *firstOp, *secondOp;
    struct command *newCommand;
    int *i = (int *)malloc(sizeof(int));
    i[0] = 0;
    firstOp = (char *)malloc(32 * sizeof(char));
    secondOp = (char *)malloc(32 * sizeof(char));
    getNextWord(operands, i, firstOp);
    getNextWord(operands, i, secondOp);
    /* if there is only one operand, make it the secondOp
    if none, make both NULL */
    if (secondOp[0] == '\0')
    {
        free(secondOp);
        secondOp = firstOp;
        firstOp = NULL;
        if (secondOp[0] == '\0')
        {
            free(secondOp);
            secondOp = NULL;
        }
    }
    else
        removeCommaFromOperand(firstOp);
    newCommand = processCommandSecondPass(commandName, firstOp, secondOp, line);
    if (newCommand != NULL)
    {
        success = addCommandToImageCodeSecondPass(newCommand, codeImage, symbolTable, extFile, line);
        free(newCommand);
    }
    else
        success = FALSE;
    if (firstOp != NULL)
        free(firstOp);
    if (secondOp != NULL)
        free(secondOp);
    free(i);
    return success;
}

/* creates command from line second pass. return created command or null if failed*/
command *processCommandSecondPass(char *commandName, char *firstOp, char *secondOp, input_line *line)
{
    bool success = TRUE;
    struct command *newCommand = (command *)malloc(sizeof(command));
    operand_addressing_method originOperandAddressingMethod;
    operand_addressing_method destinationOperandAddressingMethod;
    int i;
    /* process operands adressing methods */
    originOperandAddressingMethod = getOperandAddressingMethod(firstOp);
    destinationOperandAddressingMethod = getOperandAddressingMethod(secondOp);
    if (originOperandAddressingMethod == ILLEGAL_SYMBOL || destinationOperandAddressingMethod == ILLEGAL_SYMBOL)
        success = FALSE;
    /* look for command in command table */
    for (i = 0; i < 16; ++i)
    {
        if (!strcmp(commandName, commandTable.name[i]))
        {
            newCommand->opcode = commandTable.opcodes[i];
            newCommand->funct = commandTable.functs[i];
            if (compatibleAddressingMethodsMute(originOperandAddressingMethod, commandTable.originOperandAddressingMethods[i]) == FALSE ||
                compatibleAddressingMethodsMute(destinationOperandAddressingMethod, commandTable.destinationOperandAddressingMethods[i]) == FALSE)
                success = FALSE;
            else
            {
                newCommand->originOperandAddressingMethod = originOperandAddressingMethod;
                newCommand->destinationOperandAddressingMethod = destinationOperandAddressingMethod;
                newCommand->originOperand = firstOp;
                newCommand->destinationOperand = secondOp;
            }
            break;
        }
    }
    if (i == 16 || success == FALSE)
    {
        free(newCommand);
        return NULL;
    }
    return newCommand;
}

/* fills undefined lines in codeImage. returns wether successfull*/
bool addCommandToImageCodeSecondPass(command *newCommand, code_image *codeImage, symbol_table *symbolTable, FILE *extFile, input_line *line)
{
    bool success = TRUE;
    /* skipping command as it should be defined */
    codeImage->currCodeLine = codeImage->currCodeLine->next;
    processOperandSecondPass(codeImage, newCommand->originOperandAddressingMethod, newCommand->originOperand, symbolTable, extFile, line);
    processOperandSecondPass(codeImage, newCommand->destinationOperandAddressingMethod, newCommand->destinationOperand, symbolTable, extFile, line);
    return success;
}

/* process operand in second pass and adjust codeImage if nessesary. return wether successfull */
bool processOperandSecondPass(code_image *codeImage, operand_addressing_method operandAddressingMethod, char *operand, symbol_table *symbolTable, FILE *extFile, input_line *line)
{
    bool success = TRUE;
    symbol_table *temp;
    if (operandAddressingMethod != NO_OPERAND)
    {
        if (operandAddressingMethod == DIRECT_ADDR || operandAddressingMethod == RELATIVE_ADDR)
        {
            /* looks for entry in entry table */
            temp = getEntryByName(symbolTable, operand);
            if (temp == NULL)
            {
                printf("%d : %s\tentry %s is undefined \n", line->number, line->content, operand);
                success = FALSE;
            }
            else
            {
                free(codeImage->currCodeLine->code->hexaCode);
                codeImage->currCodeLine->code->hexaCode = intToHexaCode(temp->value);
                /* switch */
                switch (temp->attribute)
                {
                case EXTERN_ATT:
                    addToExternalFile(extFile, operand, codeImage->currCodeLine->address);
                    codeImage->currCodeLine->code->ARE = EXTERNAL;
                    break;

                case DATA_ATT:
                    codeImage->currCodeLine->code->ARE = ABSOLUTE;
                    break;

                case CODE_ATT:
                    codeImage->currCodeLine->code->ARE = RELOCATEABLE;
                    break;
                }
            }
        }
        codeImage->currCodeLine = codeImage->currCodeLine->next;
    }
    return success;
}