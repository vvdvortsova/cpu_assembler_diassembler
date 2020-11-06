#ifndef CPU_ASSEMBLER_DISASSEMBLER_DISASM_H
#define CPU_ASSEMBLER_DISASSEMBLER_DISASM_H
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "asm_utils.h"


/**
 * Method translates binary file with byteCode into file.txt with mnemonics
 * @param fileWithByteCode
 * @param fileWithMnemonics
 * @return the success of function
 */
int disAssembler(const char* fileWithByteCode, const char* fileWithMnemonics);

/**
 * Method translates byteCodes into mnemonics
 * @param byteCodes
 * @param size
 * @param file
 */
void disassemblerToFile(char* byteCodes, size_t size, FILE* file);
int writeMnemonicsToFile(const char* instruction, byte reg, FILE* file);
void disassembleFirstWayToReadTags(char* byteCodes, int size, vector* tags, int* countOfFunction, int* countOfBytes);
void disassembleSecondWayToWriteTags(char* byteCodes, int size, vector* tags, FILE* file, int* countOfBytes);

#endif //CPU_ASSEMBLER_DISASSEMBLER_DISASM_H