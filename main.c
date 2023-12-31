#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORYLENGTH 0xFFFF + 1
#define NOP 0b00011000
#define HALT 0b00011001

#define MATH_FUN_MASK 0b01110000
#define MATH_DEST_MASK 0b00001100
#define MATH_SRC_MASK 0b00000011

#define MEM_FUN_MASK 0b00001000
#define MEM_REG_MASK 0b00000100
#define MEM_MTHD_MASK 0b00000011

#define BRANCH_TYPE_MASK 0b00000111

void fetchNextInstruction(void);
void executeInstruction(void);
void mathOp(void);
void memoryOp(void);
void branchOp(void);

void storeBigEndian(uint8_t *dest, uint16_t val);
uint16_t loadBigEndian(uint8_t *src);

uint8_t memory[MEMORYLENGTH] = {0};
uint8_t ACC = 0;
uint8_t IR = 0;
uint16_t MAR = 0;
uint16_t PC = 0;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!! MAIN: REMOVE BEFORE SUBMISSION !!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define MEMLOADFILE "mem_in.txt"
#define MEMDUMPFILE "mem_out.txt"

int memoryInit(void);
int memoryDump(void);

int main(int argc, char *argv[]) {
    int i = 0;
    i = memoryInit();
    if (i == 0) {
        printf("Error initializing memory\n");
        return 0;
    }

    while (memory[PC] != HALT) {
        fetchNextInstruction();
        executeInstruction();
    }

    i = memoryDump();
    if (i == 0) {
        printf("Error dumping memory\n");
        return 0;
    }
    return 0;
}

/// @brief Reads the data from the input memory file and stores it in the memory
/// array
/// @return 1 if successful, 0 if not
int memoryInit() {
    FILE *fp;
    fp = fopen(MEMLOADFILE, "r");
    if (fp == NULL) {
        printf("Error opening memory file\n");
        return 0;
    }

    int i = 0;
    while (fscanf(fp, "%02x", &memory[i]) != EOF) {
        i++;
    }

    fclose(fp);
    return 1;
}

/// @brief Dumps the contents of the memory array to the output memory file
/// @return 1 if successful, 0 if not
int memoryDump() {
    FILE *fp;
    fp = fopen(MEMDUMPFILE, "w");
    if (fp == NULL) {
        printf("Error opening memory file\n");
        return 0;
    }

    int i = 0;
    // Match formatting of example output file
    while (i < MEMORYLENGTH) {
        fprintf(fp, "%02x ", memory[i]);

        if ((i + 1) % 16 == 0 && (i + 1) != MEMORYLENGTH) fprintf(fp, "\n");
        i++;

    }

    fclose(fp);
    return 1;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!! END MAIN: REMOVE BEFORE SUBMISSION !!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/**
 * @brief Fetches the next instruction from memory into IR and updates PC.
 *
 * Reads the instruction from memory at the current PC and updates the PC to
 * point to the next instruction depending on the length of operand following
 * the instruction.
 */
void fetchNextInstruction() {
    IR = memory[PC];
    PC++;

    // math op: msb is 1
    if (IR & 0b10000000) {
        // printf("Math Op at PC: %d (%02x)", PC - 1, IR);
        // destination memory (16 bit)
        if ((IR & MATH_DEST_MASK) == 0b00001100) {
            PC += 2;
        } 
        // source can only be an in-memory operand if destination is not in-memory operand
        else {
            // source constant (8 or 16 bit)
            if ((IR & MATH_SRC_MASK) == 0b00000010) {
                // acc is destination, so source is 8 bit
                if ((IR & MATH_DEST_MASK) == 0b00000100) {
                    PC++;
                }
                // otherwise, source is 16 bit
                else {
                    PC++;
                }
            }
            // source memory (16 bit)
            else if ((IR & MATH_SRC_MASK) == 0b00000011) {
                PC += 2;
            }
        }
    }
    // memory op: first 4 bits are 0
    else if ((IR & 0b11110000) == 0b00000000) {
        // printf("Mem Op at PC: %d (%02x)", PC - 1, IR);
        // address operand, 16 bit
        if ((IR & MEM_MTHD_MASK) == 0b00000000) {
            PC += 2;
        }
        // constant operand, 8 bit for acc and 16 bit for mar
        else if ((IR & MEM_MTHD_MASK) == 0b00000001) {
            // mar is reg bit set
            if (IR & MEM_REG_MASK) {
                PC += 2;
            }
            // acc is reg bit unset
            else {
                PC++;
            }
        }
    }
    // branch op: first 5 bits are 00010
    else if ((IR & 0b11111000) == 0b00010000) {
        // printf("Branch Op at PC: %d (%02x)", PC - 1, IR);
        // branch always uses next 16 bits as operand
        PC += 2;
    }

    return;
}

/**
 * @brief Executes the instruction in IR.
 *
 * Executes the instruction in IR and updates the ACC, MAR, and PC registers
 * accordingly.
 */
void executeInstruction() {
    // math op: msb is 1
    if ((IR & 0b10000000)) {
        mathOp();
    }
    // memory op: first 4 bits are 0
    else if ((IR & 0b11110000) == 0b00000000) {
        memoryOp();
    }
    // branch op: first 5 bits are 00010
    else if ((IR & 0b11111000) == 0b00010000) {
        branchOp();
    }
    // halt
    else if (IR == HALT) {
        return;
    }
    // nop
    else if (IR == NOP) {
        return;
    }
    // invalid instruction
    else {
        printf("Invalid instruction at PC: 0x%04X\n", PC - 1);
        return;
    }

    return;
}

void mathOp() {
    uint16_t *src, *dest;
    int bits = 16;

    // Get destination:
    switch (IR & MATH_DEST_MASK) {
        case 0b00000000:
            dest = (uint16_t *)&memory[MAR];
            break;
        case 0b00000100:
            dest = (uint16_t *)&ACC;
            bits = 8;
            break;
        case 0b00001000:
            dest = &MAR;
            break;
        case 0b00001100:
            dest =
                (uint16_t *)&memory[loadBigEndian((uint16_t *)&memory[PC - 2])];
            break;
        default:
            printf("Invalid destination in math op (impossible)\n");
            return;
    }

    // Get source:
    switch ((IR & MATH_SRC_MASK)) {
        case 0b00000000:
            src = (uint16_t *)&memory[MAR];
            break;
        case 0b00000001:
            src = &ACC;
            break;
        case 0b00000010:
            if (bits == 16) {
                src = (uint16_t *)&memory[PC - 2];
            } else {
                src = (uint16_t *)&memory[PC - 1];
            }
            break;
        case 0b00000011:
            src =
                (uint16_t *)&memory[loadBigEndian((uint16_t *)&memory[PC - 2])];
            break;
        default:
            printf("Invalid source in math op (impossible)\n");
            return;
    }

    uint16_t srcVal = 0;
    uint16_t destVal = 0;
    if (bits == 16) {
        srcVal = loadBigEndian(src);
        destVal = loadBigEndian(dest);
    } else {
        srcVal = *src & 0xFF;
        destVal = *dest & 0xFF;
    }

    // Perform function:
    switch (IR & MATH_FUN_MASK) {
        // AND
        case 0b00000000:
            destVal &= srcVal;
            break;
        // OR
        case 0b00010000:
            destVal |= srcVal;
            break;
        // XOR
        case 0b00100000:
            destVal ^= srcVal;
            break;
        // ADD
        case 0b00110000:
            destVal += srcVal;
            break;
        // SUB
        case 0b01000000:
            destVal -= srcVal;
            break;
        // INC
        case 0b01010000:
            destVal += 1;
            break;
        // DEC
        case 0b01100000:
            destVal -= 1;
            break;
        // NOT
        case 0b01110000:
            destVal = ~destVal;
            break;
        default:
            printf("Invalid function in math op (impossible)\n");
            break;
    }

    if (bits == 16) {
        storeBigEndian(dest, destVal);
    } else {
        *dest = destVal & 0xFF;
    }

    return;
}

void memoryOp() {
    // Register
    uint16_t *reg;
    int bits = 16;

    if (IR & MEM_REG_MASK) {
        reg = &MAR;
        bits = 16;
    } else {
        reg = &ACC;
        bits = 8;
    }

    // Operand
    uint8_t *operand;
    // Address
    if ((IR & MEM_MTHD_MASK) == 0b00000000) {
        operand = &memory[loadBigEndian(&memory[PC - 2])];
    }
    // Constant
    else if ((IR & MEM_MTHD_MASK) == 0b00000001) {
        // MAR is reg bit set, 16 bit constant
        if (IR & MEM_REG_MASK) {
            operand = &memory[PC - 2];
        }
        // ACC is reg bit unset, 8 bit constant
        else {
            operand = &memory[PC - 1];
        }
    }
    // Indirect (MAR as pointer)
    else if ((IR & MEM_MTHD_MASK) == 0b00000010) {
        operand = &memory[MAR];
    }

    // Function
    // Store
    if ((IR & MEM_FUN_MASK) == 0b00000000) {
        if (bits == 16) {
            storeBigEndian(operand, *reg);
        } else {
            *operand = *reg & 0xFF;
        }
    }
    // Load
    else if ((IR & MEM_FUN_MASK) == 0b00001000) {
        if (bits == 16) {
            *reg = loadBigEndian(operand);
        } else {
            *reg = *operand & 0xFF;
        }
    }

    return;
}

void branchOp() {
    // Branch type
    uint8_t type = IR & BRANCH_TYPE_MASK;

    // Get the address to branch to
    uint16_t address = loadBigEndian(&memory[PC - 2]);

    // Branch
    switch (type) {
        // Unconditional branch
        case 0b000:
            PC = address;
            break;
        // Branch if ACC == 0
        case 0b001:
            if (ACC == 0) PC = address;
            break;
        // Branch if ACC != 0
        case 0b010:
            if (ACC != 0) PC = address;
            break;
        // Branch if ACC > 0
        case 0b011:
            if (ACC < 0) PC = address;
            break;
        // Branch if ACC < 0
        case 0b100:
            if (ACC <= 0) PC = address;
            break;
        // Branch if ACC >= 0
        case 0b101:
            if (ACC > 0) PC = address;
            break;
        // Branch if ACC <= 0
        case 0b110:
            if (ACC >= 0) PC = address;
            break;
        default:
            printf("Invalid branch type in branch op (impossible)\n");
            break;
    }

    return;
}

/**
 * @brief Stores a 16-bit value in big-endian format.
 *
 * @param dest Pointer to the destination memory location.
 * @param val  16-bit value to be stored in big-endian format.
 */
void storeBigEndian(uint8_t *dest, uint16_t val) {
    *dest = (val >> 8) & 0xFF;
    *(dest + 1) = val & 0xFF;
}

/**
 * @brief Loads a 16-bit value from memory in big-endian format.
 *
 * @param src Pointer to the memory location containing the 16-bit value.
 * @return The 16-bit value loaded from memory in big-endian format.
 */
uint16_t loadBigEndian(uint8_t *src) { return (((uint16_t)*src) << 8) | ((uint16_t)*(src + 1)); }
