/*  */

#ifndef _INSTRUCTIONS__HEADER_
#define _INSTRUCTIONS__HEADER_


/* check if operand addressing method matches commands; returns TRUE if so */
bool compatibleAddressingMethods(operand_addressing_method operandAddressingMethod,
                                 command_addressing_method commandAddressingMethod, input_line *line);
                                 
/* like previous function, but with no print for errors (for second pass) */
bool compatibleAddressingMethodsMute(operand_addressing_method operandAddressingMethod,
                                     command_addressing_method commandAddressingMethod);

/* checks operand and returns it's addressing method */
operand_addressing_method getOperandAddressingMethod(char *operand);

/* check if label is a saved keyword */
bool isKeyword(char *label);

/* returns the correct char for ARE*/
char getAREChar(ARE_property are);

#endif