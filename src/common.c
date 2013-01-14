/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "hash.h"
#include "instructions.h"

#define DEBUG 1

// stores a value in memory
void store (hashtable* memory, uint32_t addr, int size, uint8_t* data)
{
    int i;

    for (i = 0; i < size; i++)
    {
        node* n = hashtable_search (memory, addr + i);

        if (n)
             n->data = data[i];
        else
            hashtable_add_node (memory, addr + i, data[i]);
    }
}

// load an 8-bit value from memory
// returns a random value if nothing exists in memory
// simulating the meaningless data in physical memory
uint8_t load (hashtable* memory, uint32_t addr)
{
    node* n = hashtable_search (memory, addr);

    if (n)
    {
        return n->data;
    }
    else
    {
        #ifdef DEBUG
            return 0x00;
        #else
            return (rand () % 0xFF);
        #endif
    }
}

// load a 32-bit value from memory
uint32_t load32 (hashtable* memory, unsigned int addr)
{
    int i;
    uint32_t val = 0;

    for (i = 3; i >= 0; i--)
    {   
        val = (val << 8) | load (memory, addr + i);
    }

    return val;
}

// retrieves and returns bits n to n+size
uint32_t get_bits (uint32_t instruction, uint8_t n, uint8_t size)
{
    uint32_t mask = (uint32_t) pow (2, size) - 1;
    return ((instruction >> n) & mask);
}

// retrieves and returns bit n
uint8_t get_bit (uint32_t instruction, uint8_t n)
{
    return get_bits (instruction, n, 1);
}

// extract the conditional bits
// bits 28 to 31
uint8_t get_cond (uint32_t instruction)
{
    return get_bits (instruction, 28, 4);
}

// determines instruction type
// bits 26 and 27
uint8_t get_instruction_type (uint32_t instruction)
{
    uint8_t type = get_bits (instruction, 26, 2);
    
    switch (type)
    {
        // MUL/MLA has 1001 at bits 4 to 7
        case INSTR_DP:
            if (get_bits (instruction, 4, 4) == 9)
                return INSTR_MUL;
            else
                return INSTR_DP;

        // Branch instructions have a 1 at bit 25
        case INSTR_B:
            if (get_bit (instruction, 25) == 1)
                return INSTR_B;
            break;

        case INSTR_LS:
            return INSTR_LS;

        // The SWI instruction has 11 at bits 24 to 25
        case INSTR_SWI:
            if (get_bits (instruction, 24, 2) == 3)
                return INSTR_SWI;
            break;
    }

    return INSTR_UNKNOWN;
}

