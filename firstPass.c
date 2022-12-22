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

/* process line if first pass; return TRUE if no errors found */
bool firstPassProcessLine(symbol_table **symbolTable, code_image *codeImage, int *IC, int *DC, input_line *line)
{
    char *label, *instruction, *lineEnd; /* strings for holding words of sentence */
    bool labelFlag, success;
    int *i = (int *)malloc(sizeof(int)), *j = (int *)malloc(sizeof(int));
    i[0] = 0;
    j[0] = 0;
    /* skip to non white character */
    _SKIP__TO__NON__WHITE_(line->content, i[0])
    /* check for comment (first non-white is ; or line is full of spaces) */
    if (line->content[i[0]] == ';' || line->content[i[0]] == '\0')
    {
        free(i);
        free(j);
        return TRUE;
    }
    label = (char *)malloc(sizeof(char) * _MAX__LINE__SIZE_ + 1);
    instruction = (char *)malloc(sizeof(char) * _MAX__LINE__SIZE_ + 1);
    lineEnd = (char *)malloc(sizeof(char) * _MAX__LINE__SIZE_ + 1);
    /* check for label */
    getNextWord(line->content, i, label);
    labelFlag = isLabel(label);
    /* get next word */
    if (labelFlag == FALSE)
        strcpy(instruction, label);
    else
    {
        getNextWord(line->content, i, instruction);
        removeColonFromLabel(label);
    }
    getRestOfLine(line->content, i[0], lineEnd);
    /* check instruction type */
    success = checkInstructionFirstPass(label, instruction, lineEnd, labelFlag, symbolTable, codeImage, IC, DC, line, j);
    /* free */
    free(label);
    free(instruction);
    free(lineEnd);
    free(i);
    free(j);
    return success;
}

/* check instruction type in first pass and process accordingly. returns wether successfull */
bool checkInstructionFirstPass(char *label, char *instruction, char *lineEnd, bool labelFlag, symbol_table **symbolTable, code_image *codeImage, int *IC, int *DC, input_line *line, int *j)
{
    bool success = TRUE;
    char *temp;
    if (isStorageIsntruction(instruction)) /* if storage instruction */
    {
        if (labelFlag) /* checks if label is valid and adds to symbol table */
        {
            if (isValidLabel(label, symbolTable[0], line) == TRUE)
                symbolTable[0] = addToSymbolTable(label, instruction, DC[0], symbolTable[0], line, DATA_ATT);
            else
                success = FALSE;
        }
        success = (addDataTocodeImage(lineEnd, instruction, DC, codeImage, line) && success);
    }
    else if (isConnectionInstruction(instruction)) /* is external or entry instruction */
    {
        if (isExternInstruction(instruction)) /* if .extern */
        {
            if (labelFlag) /* if label declared in external return error */
            {
                printf("%d : %s\tcannot declare label on .extern or .entry instructions\n", line->number, line->content);
                success = FALSE;
            }
            else
            {
                temp = malloc(sizeof(char) * (strlen(lineEnd) + 1));
                getNextWord(lineEnd, j, temp);
                if (j[0] < strlen(lineEnd))
                {
                    printf("%d : %s\texcess text after command\n", line->number, line->content);
                    success = FALSE;
                }
                if (isValidLabel(temp, symbolTable[0], line) == TRUE)
                    symbolTable[0] = addToSymbolTable(temp, instruction, 0, symbolTable[0], line, EXTERN_ATT);
                else
                    success = FALSE;
                free(temp);
            }
        }
        else /* if entry, handle on second pass*/
        {
            if (labelFlag) /* if label declared in entry return error */
            {
                printf("%d : %s\tcannot declare label on .extern or .entry instructions\n", line->number, line->content);
                success = FALSE;
            }
            else
                success = TRUE;
        }
    }
    else
    { /* is code instruction or illegal one */
        if (labelFlag == TRUE)
        {
            if (isValidLabel(label, symbolTable[0], line) == TRUE)
                symbolTable[0] = addToSymbolTable(label, instruction, IC[0], symbolTable[0], line, CODE_ATT);
            else
                success = FALSE;
        }
        success = (processCodeInstructionFirstPass(instruction, lineEnd, IC, codeImage, line) && success);
    }
    return success;
}

/* gets processed command and creates code lines, then adds them to code image*/
void addCommandToImageCodeFirstPass(command *newCommand, int *IC, code_image *codeImage, input_line *line)
{
    machine_code *line1, *line2, *line3;
    /* create lines from processed command */
    line1 = makeLineFromCommand(newCommand);
    line2 = makeLineFromOperandFirstPass(newCommand->originOperand, newCommand->originOperandAddressingMethod, line);
    line3 = makeLineFromOperandFirstPass(newCommand->destinationOperand, newCommand->destinationOperandAddressingMethod, line);
    /* add lines to codeImage */
    addCodeLineToCodeImage(line1, IC, codeImage);
    addCodeLineToCodeImage(line2, IC, codeImage);
    addCodeLineToCodeImage(line3, IC, codeImage);
}

/* process code instruction to command in first pass and returns wether successful*/
bool processCodeInstructionFirstPass(char *commandName, char *operands, int *IC, code_image *codeImage, input_line *line)
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
    if (operands[i[0]] != '\0' && operands[i[0]] != '\n' && operands[i[0]] != '\r')
    {
        printf("%d : %s\texcess text after command\n", line->number, line->content);
        success = FALSE;
    }
    newCommand = processCommandFirstPass(commandName, firstOp, secondOp, line);
    if (newCommand != NULL)
    {
        addCommandToImageCodeFirstPass(newCommand, IC, codeImage, line);
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

/* creates command from line first pass
returns the command if created successfully, otherwise returns null */
command *processCommandFirstPass(char *commandName, char *firstOp, char *secondOp, input_line *line)
{
    bool success = TRUE;
    command *newCommand = (command *)malloc(sizeof(command));
    operand_addressing_method originOperandAddressingMethod;
    operand_addressing_method destinationOperandAddressingMethod;
    int i;
    /* process operands addressing methods */
    originOperandAddressingMethod = getOperandAddressingMethod(firstOp);
    destinationOperandAddressingMethod = getOperandAddressingMethod(secondOp);
    if (originOperandAddressingMethod == ILLEGAL_SYMBOL)
    {
        printf("%d : %s\t%s is an illegal label\n", line->number, line->content, firstOp);
        success = FALSE;
    }
    if (destinationOperandAddressingMethod == ILLEGAL_SYMBOL)
    {
        printf("%d : %s\t%s is an illegal label\n", line->number, line->content, secondOp);
        success = FALSE;
    }
    /* look for command in command table */
    for (i = 0; i < 16; ++i)
    {
        if (!strcmp(commandName, commandTable.name[i]))
        {
            newCommand->opcode = commandTable.opcodes[i];
            newCommand->funct = commandTable.functs[i];
            if (compatibleAddressingMethods(originOperandAddressingMethod, commandTable.originOperandAddressingMethods[i], line) == FALSE ||
                compatibleAddressingMethods(destinationOperandAddressingMethod, commandTable.destinationOperandAddressingMethods[i], line) == FALSE)
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
