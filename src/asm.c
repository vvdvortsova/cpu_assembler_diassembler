/**
* @file         asm.c
* @brief        Definitions of methods for assembler
* @author       Dvortsova Varvara BSE182 HSE
* @include
*/
#include "asm.h"

byte getOpCodeWithStringOfCode(const char* code, size_t len) {
    assert(code != NULL);
    if (strncmp(code, "push", len) == 0) return PUSH;
    if (strncmp(code, "pop", len) == 0)  return POP;
    if (strncmp(code, "add", len) == 0)  return ADD;
    if (strncmp(code, "sub", len) == 0)  return SUB;
    if (strncmp(code, "mul", len) == 0)  return MUL;
    if (strncmp(code, "div", len) == 0)  return DIV;
    if (strncmp(code, "sqrt", len) == 0) return SQRT;
    if (strncmp(code, "hlt", len) == 0)  return HLT;
    if (strncmp(code, "in",len) == 0)    return IN;
    if (strncmp(code, "out",len) == 0)   return OUT;
    if (strncmp(code, "ret",len) == 0)   return RET;
    if (strncmp(code, "call",len) == 0)  return CALL;
    if (strncmp(code, "f",len) == 0)     return F;
    if (strncmp(code, "jmp",len) == 0)   return JMP;
    if (strncmp(code, "je",len) == 0)    return JE;
    if (strncmp(code, "jne",len) == 0)   return JNE;
    if (strncmp(code, "jl",len) == 0)    return JL;
    if (strncmp(code, "jle",len) == 0)   return JLE;
    if (strncmp(code, "jg",len) == 0)    return JG;
    if (strncmp(code, "jge",len) == 0)   return JGE;
    if (strncmp(code, "t",len) == 0)     return TAG;
    if (strncmp(code, "mov",len) == 0)   return MOV;
    return INVALID_OP_ERROR;
}

byte getRegistersByMnemonic(const char* code, size_t len) {
    assert(code != NULL);
    if (strncmp(code, "rax", len) == 0) return RAX;
    if (strncmp(code, "rbx", len) == 0) return RBX;
    return INVALID_REG_ERROR;
}

int assembler(char* fileWithMnemonics, char* fileWithByteCode) {
    assert(fileWithByteCode != NULL);
    assert(fileWithMnemonics != NULL);

    //vector for saving tag-pos
    vector* tags = calloc(1, sizeof(vector));
    vectorInit(tags);

    int size = 0;
    //get array with disasmCode and args
    char* disasmCode = getBuffer(fileWithMnemonics, &size, "r");
    //open binary file to write opcodes
    FILE* fileByte = fopen(fileWithByteCode, "wb");
    if (fileByte == NULL) {
        fprintf(stderr, "Do not open = %s\n", fileWithByteCode);
        exit(EXIT_FAILURE);
    }

    char* pointToTheEnd = disasmCode + size;
    char* mnemonicBegin = disasmCode;
    char* mnemonicEnd   = disasmCode;
    size_t countsOfBytes = 0;

    //first iteration
    for (; mnemonicEnd < pointToTheEnd;) {
        while (isspace(*mnemonicEnd) == 0) {
            mnemonicEnd++;
            if (mnemonicEnd >= pointToTheEnd)
                break;
        }
        if (firstWayWithoutWritingInFile(&mnemonicBegin, &mnemonicEnd, pointToTheEnd, tags, &countsOfBytes) != 0) {
            fprintf(stderr, "Invalid instruction in assembler function!\n");
            fprintf(stderr, "%s\n",mnemonicBegin);
            exit(EXIT_FAILURE);
        }
        while (isspace(*mnemonicEnd) != 0)
            mnemonicEnd++;
        mnemonicBegin = mnemonicEnd;
    }

    pointToTheEnd = disasmCode + size;
    mnemonicBegin = disasmCode;
    mnemonicEnd   = disasmCode;

    //second iteration and writing
    for (; mnemonicEnd < pointToTheEnd;) {
        while (isspace(*mnemonicEnd) == 0) {
            mnemonicEnd++;
            if (mnemonicEnd >= pointToTheEnd)
                break;
        }
        if (secondWayWithWritingToFile(&mnemonicBegin, &mnemonicEnd, pointToTheEnd, fileByte, tags)!= 0) {
            fprintf(stderr, "Invalid instruction in assembler function!\n");
            fprintf(stderr, "%s\n",mnemonicBegin);
            exit(EXIT_FAILURE);
        }
        while (isspace(*mnemonicEnd) != 0)
            mnemonicEnd++;
        mnemonicBegin = mnemonicEnd;
    }


    vectorFree(tags);
    free(disasmCode);
    fclose(fileByte);
    return EXIT_SUCCESS;
}

void getNextMnemonic(char** MnemonicStart, char** mnemonicEnd, const char* endOfFile) {
    while (isspace(**mnemonicEnd) != 0)
        (*mnemonicEnd)++;

    *MnemonicStart = *mnemonicEnd;
    while (isspace(**mnemonicEnd) == 0) {
        (*mnemonicEnd)++;
        if (*mnemonicEnd >= endOfFile)
            break;
    }
}

bool getDoubleNumber(char* mnemonicStart, double* number) {
    char* endPtr = NULL;
    //cast string to double
    *number = strtod(mnemonicStart, &endPtr);
    if (endPtr == mnemonicStart) {
        return false;
    }
    return true;
}

bool getIntNumber(char* mnemonicStart, int* number) {
    char* endPtr = NULL;
    *number = (int)strtol(mnemonicStart, &endPtr, 10);
    if (endPtr == mnemonicStart) {
        return false;
    }
    return true;
}

int firstWayWithoutWritingInFile(char** mnemonicBegin, char** mnemonicEnd,
        char* endOfFile, vector* tags, size_t* countsOfBytes) {
    size_t lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
    byte opCode = getOpCodeWithStringOfCode(*mnemonicBegin, lenOfMnemonic);
    byte rgCode;
    if (opCode != INVALID_OP_ERROR) {
        switch (opCode) {
            case MOV:
                // findReg
                getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                lenOfMnemonic = *mnemonicEnd - *mnemonicBegin - 1;// because we do not calculate ","
                rgCode = getRegistersByMnemonic(*mnemonicBegin, lenOfMnemonic);
                if (rgCode != INVALID_REG_ERROR) {
                    *countsOfBytes += sizeof(byte);//opCode
                    *countsOfBytes += sizeof(byte);//regCode
                    getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                    if (strncmp(mnemonicBegin[0],"[",1) == 0) {//call ram
                        (*mnemonicBegin)++;
                        int addr = 0;
                        if (getIntNumber(*mnemonicBegin, &addr)) {
                            *countsOfBytes += sizeof(byte);//opCode
                            *countsOfBytes += sizeof(addr);//number
                            return EXIT_SUCCESS;
                        }
                    }
                }
                fprintf(stderr,"Unexpected register in MOV\nMnemonic = %s\n", *mnemonicBegin);
                exit(EXIT_FAILURE);
            case PUSH:
                getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                double number;
                if (getDoubleNumber(*mnemonicBegin, &number)) {
                    *countsOfBytes += sizeof(byte);
                    *countsOfBytes += sizeof(number);
                    return EXIT_SUCCESS;
                } else {
                    lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
                    rgCode = getRegistersByMnemonic(*mnemonicBegin, lenOfMnemonic);
                    if (rgCode != INVALID_REG_ERROR) {
                        *countsOfBytes += sizeof(byte);//opCode
                        *countsOfBytes += sizeof(byte);//regCode
                        return EXIT_SUCCESS;
                    }

                    if (strncmp(mnemonicBegin[0],"[",1) == 0) {//call ram
                        (*mnemonicBegin)++;
                        int addr = 0;
                        if (getIntNumber(*mnemonicBegin, &addr)) {
                            *countsOfBytes += sizeof(byte);//opCode
                            *countsOfBytes += sizeof(addr);//number
                            return EXIT_SUCCESS;
                        }
                    }
                    fprintf(stderr, "Something went wrong with push!\n");
                    fprintf(stderr, "or error when parsing double value!\nMnemonic = %s\n", *mnemonicBegin);
                    exit(EXIT_FAILURE);
                }
            case JE:
            case JNE:
            case JL:
            case JLE:
            case JG:
            case JGE:
            case JMP:
            case CALL:
                *countsOfBytes += sizeof(byte);//opCode
                getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);//get the name of tag function
                *countsOfBytes += sizeof(int);//number of bytes
                return EXIT_SUCCESS;
            case POP:
                if (strncmp(mnemonicEnd[0], "\n", 1) == 0) {
                    *countsOfBytes += sizeof(byte);//opCode
                    return EXIT_SUCCESS;
                }
                if (strncmp(mnemonicEnd[0], "\n", 1) != 0) {//means that is register
                    getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                    lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
                    rgCode = getRegistersByMnemonic(*mnemonicBegin, lenOfMnemonic);
                    if (rgCode != INVALID_REG_ERROR) {
                        *countsOfBytes += sizeof(byte);//opCode
                        *countsOfBytes += sizeof(byte);//regCode
                        return EXIT_SUCCESS;
                    }
                    if (strncmp(mnemonicBegin[0],"[",1) == 0){//call ram
                        (*mnemonicBegin)++;
                        int addr = 0;
                        if (getIntNumber(*mnemonicBegin, &addr)) {
                            *countsOfBytes += sizeof(byte);//opCode
                            *countsOfBytes += sizeof(addr);//number
                            return EXIT_SUCCESS;
                        }
                    }
                    fprintf(stderr, "Can't identify the current register!\nMnemonic = %s\n", *mnemonicBegin);
                    return EXIT_FAILURE;
                }
            default:
                *countsOfBytes += sizeof(byte);//opCode
                return EXIT_SUCCESS;
        }
    }else{
        if(strncmp(mnemonicBegin[0],"f",1) == 0 || strncmp(mnemonicBegin[0],"t",1) == 0) {
            lenOfMnemonic = (*mnemonicEnd - *mnemonicBegin);
            char* temp = calloc(1, sizeof(temp));
            strncpy(temp, *mnemonicBegin,lenOfMnemonic-1);
            struct tag* item = calloc(1, sizeof(struct tag));
            item->name = temp;
            *countsOfBytes += sizeof(byte);
            item->position = *countsOfBytes;
            vectorAdd(tags, item);
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Can't identify the current instruction!\nMnemonic = %s\n", *mnemonicBegin);
        exit(EXIT_FAILURE);
    }
}

int secondWayWithWritingToFile(char** mnemonicBegin, char** mnemonicEnd, char* endOfFile, FILE *file, vector* tags) {
    size_t lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
    byte opCode = getOpCodeWithStringOfCode(*mnemonicBegin, lenOfMnemonic);
    byte rgCode;
    checkType isCorrectWrite;
    if (opCode != INVALID_OP_ERROR) {
        switch (opCode) {
            case MOV:
                // findReg
                getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                lenOfMnemonic = *mnemonicEnd - *mnemonicBegin - 1;// because we do not calculate ","
                rgCode = getRegistersByMnemonic(*mnemonicBegin, lenOfMnemonic);
                if (rgCode != INVALID_REG_ERROR) {
                    if (writeToFileTWOValues(MOV, rgCode, file) != EXIT_SUCCESS) {
                        fprintf(stderr, "Something went wrong with writing MOV!\n");
                        exit(EXIT_FAILURE);
                    }

                    getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                    (*mnemonicBegin)++;
                    int addr = 0;
                    if (getIntNumber(*mnemonicBegin, &addr)) {
                        isCorrectWrite = fwrite(&addr, sizeof(addr), 1, file);
                        assert(isCorrectWrite == 1);
                        return EXIT_SUCCESS;
                    }
                }
                fprintf(stderr,"Unexpected register in MOV\nMnemonic = %s\n", *mnemonicBegin);
                exit(EXIT_FAILURE);
            case PUSH:
                getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                double number;
                if (getDoubleNumber(*mnemonicBegin, &number)) {
                    isCorrectWrite = fwrite(&opCode, sizeof(byte), 1, file);
                    assert(isCorrectWrite == 1);
                    isCorrectWrite = fwrite(&number, sizeof(number), 1, file);
                    assert(isCorrectWrite == 1);
                    return EXIT_SUCCESS;
                } else {
                    lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
                    rgCode = getRegistersByMnemonic(*mnemonicBegin, lenOfMnemonic);
                    if (rgCode != INVALID_REG_ERROR) {
                        if (writeToFileTWOValues(PUSHR, rgCode, file) != EXIT_SUCCESS) {
                            fprintf(stderr, "Something went wrong with writing pop!\n");
                            exit(EXIT_FAILURE);
                        }
                        return EXIT_SUCCESS;
                    }else if (strncmp(mnemonicBegin[0],"[",1) == 0){//call ram
                        (*mnemonicBegin)++;
                        int addr = 0;
                        if (getIntNumber(*mnemonicBegin, &addr)) {
                            byte opPUSHRAM = PUSH_RAM;
                            isCorrectWrite = fwrite(&opPUSHRAM, sizeof(byte), 1, file);
                            assert(isCorrectWrite == 1);
                            isCorrectWrite = fwrite(&addr, sizeof(addr), 1, file);
                            assert(isCorrectWrite == 1);
                            return EXIT_SUCCESS;
                        }
                    }
                    fprintf(stderr, "Something went wrong with push!\n");
                    fprintf(stderr, "or error when parsing double value!\n");
                    exit(EXIT_FAILURE);
                }
            case JE:
            case JNE:
            case JL:
            case JLE:
            case JG:
            case JGE:
            case JMP:
            case CALL:
                isCorrectWrite = fwrite(&opCode, sizeof(byte), 1, file);//write opCode
                assert(isCorrectWrite == 1);
                getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);//get the name of tag function
                lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
                //get mnemonic and try to find it in tags and rechange with position
                char *temp = calloc(1, sizeof(temp));
                strncpy(temp, *mnemonicBegin, lenOfMnemonic);
                for (int i = 0; i < tags->total; ++i) {
                    char tempElem[90] = "";
                    strcpy(tempElem,((struct tag *) vectorGet(tags, i))->name);
                    if (strncmp(temp, tempElem, lenOfMnemonic) == 0) {
                        int pos = ((struct tag *) vectorGet(tags, i))->position;
                        isCorrectWrite = fwrite(&pos, sizeof(int), 1, file);
                        assert(isCorrectWrite == 1);
                        free(temp);
                        return EXIT_SUCCESS;
                    }
                }
                fprintf(stderr, "Can't find key in tags arr!\n");
                return EXIT_FAILURE;
            case POP:
                if (strncmp(mnemonicEnd[0], "\n", 1) == 0) {
                    isCorrectWrite = fwrite(&opCode, sizeof(byte), 1, file);
                    assert(isCorrectWrite == 1);
                    return EXIT_SUCCESS;
                }
                if (strncmp(mnemonicEnd[0], "\n",1) != 0) {//means that is register
                    getNextMnemonic(mnemonicBegin, mnemonicEnd, endOfFile);
                    lenOfMnemonic = *mnemonicEnd - *mnemonicBegin;
                    rgCode = getRegistersByMnemonic(*mnemonicBegin, lenOfMnemonic);
                    if (rgCode != INVALID_REG_ERROR) {
                        if (writeToFileTWOValues(POPR, rgCode, file) != EXIT_SUCCESS) {
                            fprintf(stderr, "Something went wrong with writing pop!\n");
                            exit(EXIT_FAILURE);
                        }
                        return EXIT_SUCCESS;
                    }else if (strncmp(mnemonicBegin[0],"[",1) == 0){//call ram
                        (*mnemonicBegin)++;
                        int addr = 0;
                        if (getIntNumber(*mnemonicBegin, &addr)) {
                            byte opPUSHRAM = POP_RAM;
                            isCorrectWrite = fwrite(&opPUSHRAM, sizeof(byte), 1, file);
                            assert(isCorrectWrite == 1);
                            isCorrectWrite = fwrite(&addr, sizeof(addr), 1, file);
                            assert(isCorrectWrite == 1);
                            return EXIT_SUCCESS;
                        }
                    }
                    fprintf(stderr, "Something went wrong with pop!\n");
                    exit(EXIT_FAILURE);
                }//if just pop
            default:
                isCorrectWrite = fwrite(&opCode, sizeof(byte), 1, file);
                assert(isCorrectWrite == 1);
                return EXIT_SUCCESS;
        }
    }else{
        if(strncmp(mnemonicBegin[0],"f",1) == 0 || strncmp(mnemonicBegin[0],"t",1) == 0) {
            //write code
            lenOfMnemonic = (*mnemonicEnd - *mnemonicBegin);
            char *temp = calloc(1, sizeof(temp));
            strncpy(temp, *mnemonicBegin, lenOfMnemonic);
            for (int i = 0; i < tags->total; ++i) {
                char tempElem[90] = "";
                struct tag* tmpTag = (struct tag*) vectorGet(tags, i);
                strcpy(tempElem, tmpTag->name);

                if (strncmp(temp, tempElem, lenOfMnemonic - 1) == 0) {
                    //where the nme of function we put F
                    //where the tag name we put TAG
                    //in disasm you will has another names with prefix "t" and "F"
                    byte pos = getOpCodeWithStringOfCode(&tempElem[0],1);
                    isCorrectWrite = fwrite(&pos, sizeof(byte), 1, file);
                    assert(isCorrectWrite == 1);
                    free(temp);
                    return EXIT_SUCCESS;
                }
            }
            free(temp);
            fprintf(stderr, "Can't find key in tags arr!\n");
            return EXIT_FAILURE;
        }
        fprintf(stderr, "Can't identify the current instruction!\nMnemonic = %s\n", *mnemonicBegin);
        exit(EXIT_FAILURE);
    }
}

int writeToFileTWOValues(byte opCode, byte rgCode, FILE* file) {
    checkType isCorrectWrite = fwrite(&opCode, sizeof(byte), 1, file);
    if(isCorrectWrite != 1) {
        fprintf(stderr, "Can't write opCode to file!\n");
        return EXIT_FAILURE;
    }
    isCorrectWrite = fwrite(&rgCode, sizeof(byte), 1, file);
    if(isCorrectWrite != 1) {
        fprintf(stderr, "Can't write rgCode to file!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    if(argc == 3) {
        assembler(argv[1], argv[2]);
        return EXIT_SUCCESS;
    }
    printf("Please!Check your arguments!\n");
    exit(EXIT_FAILURE);
}

