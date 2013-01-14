/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "common.h"
#include "instructions.h"
#include "hash.h"

// prints the usage of the program to stdout
void print_usage (char* name)
{
    printf ("Usage: %s [options] filename.emu\n", name);
    printf ("\t-trace - show instruction trace\n");
    printf ("\t-before - show memory dump before execution\n");
    printf ("\t-after - show memory dump after execution\n");
}

// prints a memory dump to stdout
void print_memory_dump (hashtable* memory)
{
    int i, j, size, first_run;
    uint32_t previous, lowest, lowest_index;
    node* n;
    node** contents;
    node** assembled;

    // get contents of memory
    contents = hashtable_get_values (memory, &size);
    
    // allocate array to store assembled values in
    assembled = calloc (size, sizeof (node));

    // assemble into order
    previous = 0;
    lowest_index = 0;
    first_run = 1;

    for (i = 0; i < size; i++)
    {
        lowest = 0xFFFFFFFF;

        for (j = 0; j < size; j++)
        {
            n = contents[j];

            if (((n->addr > previous) || (first_run)) && (n->addr <= lowest))
            {
                lowest_index = j;
                lowest = contents[j]->addr;
            }
        }
        
        if (first_run)
            first_run = 0;

        assembled[i] = contents[lowest_index];
        previous = contents[lowest_index]->addr;
    }

    // print memory
    for (i = 0; i < size; i++)
        printf ("0x%08X 0x%08X\n", assembled[i]->addr, assembled[i]->data);

    // free the memory
    free (contents);
    free (assembled);
}

// prints a register dump to stdout
void print_register_dump (uint32_t r[])
{
    printf ("R0=%08X R1=%08X R2=%08X R3=%08X R4=%08X R5=%08X R6=%08X R7=%08X\n",
        r[R_0], r[R_1], r[R_2], r[R_3], r[R_4], r[R_5], r[R_6], r[R_7]);
    printf ("R8=%08X R9=%08X R10=%08X R11=%08X R12=%08X SP=%08X LR=%08X PC=%08X\n",
        r[R_8], r[R_9], r[R_10], r[R_11], r[R_12], r[R_SP], r[R_LR], r[R_PC]);
}

char* opcode_to_string (uint8_t opcode)
{
    if (opcode == OP_AND) return "AND";
    if (opcode == OP_EOR) return "EOR";
    if (opcode == OP_SUB) return "SUB";
    if (opcode == OP_ADD) return "ADD";
    if (opcode == OP_CMP) return "CMP";
    if (opcode == OP_ORR) return "ORR";
    if (opcode == OP_MOV) return "MOV";
    if (opcode == OP_BIC) return "BIC";

    return "UNKNOWN";
}

char* cond_to_string (uint8_t cond)
{
    if (cond == COND_EQ) return "EQ";
    if (cond == COND_NE) return "NE";
    if (cond == COND_MI) return "MI";
    if (cond == COND_PL) return "PL";
    if (cond == COND_GE) return "GE";
    if (cond == COND_LT) return "LT";
    if (cond == COND_GT) return "GT";
    if (cond == COND_LE) return "LE";

    // leave blank for AL, as its default
    if (cond == COND_AL) return "";

    return "UNKNOWN";
}

void print_instruction (uint32_t registers[], uint32_t instr)
{
    uint8_t type, cond;

    uint8_t opcode, s, shift_val, shift, shift_by;
    uint32_t rd, rn, rs, rm, tmp;
    char operand[32], buffer[32];

    int32_t signed_addr;
    uint32_t addr;
    uint8_t l;

    uint8_t p, u, w, i;
    type = get_instruction_type (instr);
    cond = get_cond (instr);
    
    switch (type)
    {
        case INSTR_MUL:
            s = get_bit (instr, 20);
            rd = get_bits (instr, 16, 4);
            rn = get_bits (instr, 12, 4);
            rs = get_bits (instr, 8, 4);
            rm = get_bits (instr, 0, 4);

            if (get_bit (instr, 21))
                printf ("MLA%s%s R%d, R%d, R%d, R%d", cond_to_string (cond), ((s) ? "S" : ""), rd, rm, rs, rn);
            else
                printf ("MUL%s%s R%d, R%d, R%d", cond_to_string (cond), ((s) ? "S" : ""), rd, rm, rs);
            break;

        case INSTR_DP:
            opcode = get_bits (instr, 21, 4);
            s = get_bit (instr, 20);
            rn = get_bits (instr, 16, 4);
            rd = get_bits (instr, 12, 4);

            if (get_bit (instr, 25))
            {
                tmp = get_bits (instr, 0, 12);
                sprintf(operand, "#%d", rotate_right (2 * (tmp >> 8), tmp & 0xFF));
            }
            else
            {
                switch (get_bits (instr, 4, 3))
                {
                   case SHIFT_LSL_I:
                        shift_val = get_bits (instr, 0, 4);
                        shift_by = get_bits (instr, 7, 5);

                        if (registers[shift_by] != 0)
                            sprintf (operand, "#%d LSL R%d", shift_val, shift_by);
                        else
                           sprintf (operand, "#%d", shift_val);
                        break;

                   case SHIFT_LSL_R:
                        shift_val = get_bits (instr, 8, 4);
                        shift_by = get_bits (instr, 0, 4);
                             
                        if (registers[shift_by] != 0)
                            sprintf(operand, "R%d LSL R%d", shift_val, shift_by);
                        else
                            sprintf (operand, "R%d", shift_val);
                        break;

                   case SHIFT_LSR_I:
                        shift_by = get_bits (instr, 7, 5);
                        shift_val = get_bits (instr, 0, 4);

                        if (registers[shift_by] != 0)
                            sprintf (operand, "#%d LSR R%d", shift_val, shift_by);
                        else
                            sprintf (operand, "#%d", shift_val);
                        break;

                   case SHIFT_LSR_R:
                        shift_by = get_bits (instr, 8, 4);
                        shift_val = get_bits (instr, 0, 4);

                        if (registers[shift_by] != 0)
                            sprintf(operand, "R%d LSR R%d", shift_val, shift_by);
                        else
                            sprintf (operand, "R%d", shift_val);
                        break;

                    default:
                        sprintf(operand, "R%d", (get_bits (instr, 0, 4)));
                    }
                }

                switch (opcode)
                {
                    case OP_CMP:
                        printf ("CMP%s R%d, %s", cond_to_string (cond), rn, operand);
                        break;

                    case OP_MOV:
                        printf ("MOV%s%s R%d, %s", cond_to_string (cond),
                            ((s) ? "S" : ""), rd, operand);
                        break;

                    default:                    
                        printf ("%s%s%s R%d, R%d, %s", opcode_to_string (opcode),
                            cond_to_string (cond), ((s) ? "S" : ""), rd, rn,
                            operand);
                        break;
                }
                break;

       case INSTR_B:
            l = get_bit (instr, 24);
                
            signed_addr = (int32_t) get_bits (instr, 0, 24);
             
            if (get_bit (signed_addr, 23))
                addr = 0xFF000000;
            else
                addr = 0x00000000;
            addr |= ((signed_addr) << 2);

            // note that 8 is added as the trace is executed before
            // PC is update
            addr += (registers[R_PC] + 8);
                
            printf ("B%s%s 0x%08X", ((l) ? "L" : ""), cond_to_string (cond), addr);
            break;

        case INSTR_LS:
            rn = get_bits (instr, 16, 4);
            rd = get_bits (instr, 12, 4);
            addr = get_bits (instr, 0, 12);
            i = get_bit (instr, 25);
            u = get_bit (instr, 23);
            p = get_bit (instr, 24);
            w = get_bit (instr, 21);
            l = get_bit (instr, 20);

            if (i)
            {
                if (p && !w && (addr == R_PC))
                    addr = registers[R_PC] + 8;     
                else
                    addr = registers[addr];
            }

            if (u)
                addr = rn + addr;
            else
                addr = rn - addr;            

            if (l)
                printf ("LDR%s R%d 0x%08X", cond_to_string (cond), rd, addr);
            else
                printf ("STR%s R%d 0x%08X", cond_to_string (cond), rd, addr);
            break;

        case INSTR_SWI:
            printf ("SVC%s %d", cond_to_string (cond), get_bits (instr, 0, 24));
            break;
    }
}

void print_trace (uint32_t r[], uint32_t instr)
{
    print_register_dump (r);
    printf ("Next Instruction=");
    print_instruction (r, instr);
    printf ("\n\n");
}
