/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "io.h"
#include "instructions.h"
#include "hash.h"
#include "common.h"

/*
 * Global variables
 *
 */

/* 16 32-bit registers */
uint32_t registers[16];

/* Flags - N Z C V */
uint8_t flags[4];

/* Main memory - in 1 byte increments */
hashtable* memory;

/*
 * Data processing decoding functions
 *
 */

// determine which function is required to execute the instruction
void execute_dp_instruction (uint8_t opcode, uint8_t cond, uint8_t s,
    uint8_t rn, uint8_t rd, uint32_t operand)
{
    switch (opcode)
    {
        case OP_AND:
            registers[rd] = AND (flags, s, registers[rn], operand);
            break;

        case OP_EOR:
            registers[rd] = EOR (flags, s, registers[rn], operand);
            break;

        case OP_SUB:
            registers[rd] = SUB (flags, s, registers[rn], operand);
            break;

        case OP_ADD:
            registers[rd] = ADD (flags, s, registers[rn], operand);
            break;

        case OP_CMP:
            CMP (flags, registers[rn], operand);
            break;

        case OP_ORR:
            registers[rd] = ORR (flags, s, registers[rn], operand);
            break;

        case OP_MOV:
            registers[rd] = MOV (operand);
            break;

        case OP_BIC:
            registers[rd] = BIC (flags, s, registers[rn], operand);
            break;

        default:
            fprintf (stderr, "Opcode %X could not be decoded\n", opcode);
            break;
    }
}

// determines which shift to perform on operand
// and does so
uint32_t shift_operand (uint32_t instruction)
{
    uint8_t shift, val;

    switch (get_bits (instruction, 4, 3))
    {
        case SHIFT_LSL_I:
            shift = get_bits (instruction, 7, 5);
            val = registers[get_bits (instruction, 0, 4)];

            // carry_out = get_bit (val, 32 - shift, 1);
            return (val << shift);

        case SHIFT_LSL_R:
            shift = registers[get_bits (instruction, 8, 4)];
            val = registers[get_bits (instruction, 0, 4)];

            if (shift == 32)
            {
                // carry_out = get_bit (val, 0, 1);
                return 0;
            }

            if (shift > 32)
            {
                // carry_out = 0;
                return 0;
            }

            // carry_out = get_bit (val, 32 - shift, 1);
            return (val << shift);

        case SHIFT_LSR_I:
            shift = get_bits (instruction, 7, 5);
            val = registers[get_bits (instruction, 0, 4)];

            // if (shift == 0)
                // carry_out = get_bit (val, 31, 1);
            // else
                // carry_out = get_bit (val, shift - 1, 1);
            return (val >> shift);

        case SHIFT_LSR_R:
            shift = registers[get_bits (instruction, 8, 4)];
            val = registers[get_bits (instruction, 0, 4)];

            if (shift == 32)
            {
                // carry_out = get_bit (val, 31, 1);
                return 0;
            }

            if (shift > 32)
            {
                // carry_out = 0;
                return 0;
            }

            // carry_out = get_bit (val, shift - 1, 1);
            return (val >> shift);

        default:
            return registers[get_bits (instruction, 0, 4)];
    }
}

// decodes the data processing instructions
void decode_dp (uint32_t instruction)
{
        uint32_t operand;
        uint8_t op, cond, s, rn, rd;

        // grab the opcode
        op = get_bits (instruction, 21, 4);

        // update?
        s = get_bit (instruction, 20);

        // get Rn and Rd
        rn = get_bits (instruction, 16, 4);
        rd = get_bits (instruction, 12, 4);

        // register or operand values
        if (get_bit (instruction, 25))
        {
            // operand is an immediate
            // this value is split into a rotate and an immediate
            // the immediate must be rotated by twice the rotate
            // amount specified
            operand = get_bits (instruction, 0, 12);
            operand = rotate_right (2 * (operand >> 8), operand & 0xFF);
        }
        else
        {
            // operand is shifted
            operand = shift_operand (instruction);
        }

        // conditional code
        cond = get_cond (instruction);

        // EXECUTE
        if (condition_passed (flags, cond))
        {
            // do the damn thing!
            execute_dp_instruction (op, cond, s, rn, rd, operand);
        }
}

/*
 * Multiplication decoding functions
 *
 */

// decodes the MUL/MLA instructions
// whilst these are technically DP they are
// layed out differently
void decode_multiplication (uint32_t instruction)
{
    uint8_t rd, rn, rm, rs, cond, s;

    // Rd is stored in bits 16 to 19
    rd = get_bits (instruction, 16, 4);

    // get Rn
    // this is stored in bits 12 to 15
    rn = get_bits (instruction, 12, 4);

    // get Rs
    rs = get_bits (instruction, 8, 4);

    // get Rm
    // bits 0 to 3
    rm = get_bits (instruction, 0, 4);

    // condition
    cond = get_cond (instruction);

    // update flags?
    s = get_bit (instruction, 20);

    // EXECUTE
    // MLA (with addition)?
    if (condition_passed (flags, cond))
    {
        if (get_bit (instruction, 21))
            registers[rd] = MLA (flags, s, registers[rm], registers[rs], registers[rn]);
        else
            registers[rd] = MUL (flags, s, registers[rm], registers[rs]);
    }
}

/*
 * Branch decoding functions
 *
 */

// decodes the B/BL instructions
void decode_branch (uint32_t instruction)
{
    int32_t signed_immed, address;
    uint8_t l, cond;

    // Get the immediate, bits 0 to 23
    signed_immed = (int32_t) get_bits (instruction, 0, 24);

    // Get L bit (the L in BL) - causing return address to be stored
    l = get_bit (instruction, 24);

    // Get the conditional code
    cond = get_cond (instruction);

    // Form address
    // this is done by sign-extending the immed to 30 bits
    // then shifting left by two
    if (get_bit (signed_immed, 23))
        address = 0xFF000000;
    else
        address = 0x00000000;
    
    address |= ((signed_immed) << 2);

    // EXECUTE
    if (condition_passed (flags, cond))
    {
        if (l)
            registers[R_LR] = registers[R_PC] + 4;

        // add 4 to generate correct address
        // this seemes to work but it wasn't present in ARM ARM
        registers[R_PC] += address + 4;
    }
}

/*
 * Load/Store decoding functions
 *
 */

// decodes the LDR/STR instructions
void decode_ls (uint32_t instruction)
{
    uint32_t addr = 0, offset;
    uint8_t rn, rd, cond, i, p, u, b, w, l,
        shift;

    cond = get_cond (instruction);
    rn = get_bits (instruction, 16, 4);
    rd = get_bits (instruction, 12, 4);

    i = get_bit (instruction, 25);
    p = get_bit (instruction, 24);
    u = get_bit (instruction, 23);
    b = get_bit (instruction, 22);
    w = get_bit (instruction, 21);
    l = get_bit (instruction, 20);

    // Offset
    if (p && !w)
    {
        offset = get_bits (instruction, 0, 12);

        // register?
        if (i)
        {
            if (offset == R_PC)
                offset = registers[R_PC] + 8;
            else
                offset = registers[offset];
        }

        // add/sub?
        if (u)
            addr = registers[rn] + offset;
        else
            addr = registers[rn] - offset;
    }

    // Scaled register offset
    // TODO - how do I distinguish this from Offset? 

    // Pre-indexed
    if (p && w)
    {
        offset = get_bits (instruction, 0, 12);

        if (i)
            offset = registers[offset];

        if (u)
            addr = registers[rn] + offset;
        else
            addr = registers[rn] - offset;

        if (condition_passed (flags, cond))
            registers[rn] = addr;
    }

    // Scaled pre-indexed
    // TODO - how do I distinguish this from Pre-indexed?

    // Post-indexed
    if (!p && !w)
    {
        offset = get_bits (instruction, 0, 12);
        addr = rn;

        if (condition_passed (flags, cond))
        {
            if (u)
                rn = rn + offset;
            else
                rn = rn - offset;
        }
    }

    // Scaled register post-indexed
    // TODO

    // EXECUTE
    if (condition_passed (flags, cond))
    {
        // TODO - there looks like there may be a rotation here
        // if CP15_reg1_Ubit == 0 (what is that?!)
        if (l)
        {
            registers[rd] = load32 (memory, addr);

            if (rd == R_PC)
                registers[R_PC] &= 0xFFFFFFFC;
        }
        else
        {
            store (memory, addr, 4, &rd);
        }
    }
}

/*
 * SWI decoding functions
 *
 */

// decodes the SWI instructions
// returns 1 if the emulator should halt
// returns 0 otherwise
int decode_swi(uint32_t instruction)
{
    uint32_t immed;
    uint8_t cond;

    cond = get_cond (instruction);
    immed = get_bits (instruction, 0, 24);
    
    if (condition_passed (flags, cond))
        return SVC (registers, immed);
    else
        return 0;
}

/*
 * Main emulator functionality
 *
 */

// main emulation loop
void emulate (int trace, int before, int after)
{
    uint32_t instruction;
    uint8_t type, halt = 0;

    // need to show memory dump?
    if (before)
    {
        print_memory_dump (memory);
        printf ("\n");
    }

    for (;;)
    {
        // FETCH
        // request instruction
        instruction = load32 (memory, registers[R_PC]);

        // print the trace?
        if (trace)
            print_trace (registers, instruction);

        // increment PC
        registers[R_PC] += 4;

        // DECODE and EXECUTE
        // Note that these two stage have been grouped into
        // the same function for optimisation
        type = get_instruction_type (instruction);

        switch (type)
        {
            case INSTR_DP:
                decode_dp (instruction);
                break;

            case INSTR_MUL:
                decode_multiplication (instruction);
                break;

            case INSTR_B:
                decode_branch (instruction);
                break;

            case INSTR_LS:
                decode_ls (instruction);
                break;

            case INSTR_SWI:
                if (decode_swi (instruction))
                    halt = 1;
                break;
        }

        // halt the emulator?
        if (halt)
            break;
    }

    // need to show memory dump?
    if (after)
        print_memory_dump (memory);
}

void read_file (FILE* fp)
{
    uint32_t mem, instr;
    int pc_set = 0;

    rewind (fp);

    // format is memory address `space` instruction
    while (fscanf (fp, "%X", &mem) != EOF)
    {
        if (fscanf (fp, "%X", &instr) != 1)
            break;

        // store in 'memory'
        store (memory, mem, sizeof(instr), (uint8_t*) &instr);

        // initialise the program counter
        if (!pc_set)
        {
            registers[R_PC] = mem;
            pc_set = 1;
        }

    }
}

// code entry point
int main (int argc, char** argv)
{
    FILE *fp = NULL;
    int i, trace = 0, before = 0, after = 0;

    // arguments?
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            // options
            if (strcmp (argv[i], "-trace") == 0)
            {
                trace = 1;
                continue;
            }

            if (strcmp (argv[i], "-before") == 0)
            {
                before = 1;
                continue;
            }

            if (strcmp (argv[i], "-after") == 0)
            {
                after = 1;
                continue;
            }

            // assume this is a (.emu) file
            fp = fopen(argv[i], "r");

            // if the file is valid, stop parsing arguments
            if (fp)
                break;

            // print an error, file not found
            fprintf (stderr, "The file %s was not found.\n", argv[i]);
        }
    }

    // valid file
    if (fp == NULL)
        return 1;

    // initialise memory
    memory = hashtable_create ();

    // load .emu into memory
    read_file (fp);

    // close the file
    fclose (fp);

    // emulate!
    emulate (trace, before, after);

    // clean up
    hashtable_destroy (memory);
    return 0;
}
