#include <stdio.h>
#include <stdlib.h>

#define NOP 0b00011000
#define HALT 0b00011001

#define MEMORYLENGTH 65536
#define MEMFILE "mem_in.txt"

void fetchNextInstruction(void);
void executeInstruction(void);
int memoryInit(void);
int memoryDump(void);

unsigned char memory[MEMORYLENGTH] = {0};
unsigned char ACC = 0;
unsigned char IR = 0;
unsigned int MAR = 0;
unsigned int PC = 0;

int main(int argc, char *argv[]) {
    printf("TODO\n");
    return 0;
}

int memoryInit() {
    printf("TODO\n");
    return 0;
}