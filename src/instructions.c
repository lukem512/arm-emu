/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "instructions.h"
#include "common.h"

// performs conditional analysis of the operands
// returns a 1 for passed
// returns a 0 for failed
uint8_t condition_passed (uint8_t* flags, uint8_t cond)
{
    switch (cond)
    {
        case COND_EQ:
            return (flags[F_Z]) ? 1 : 0;

        case COND_MI:
            return (flags[F_N]) ? 1 : 0;

        case COND_PL:
            return (flags[F_N]) ? 0 : 1;

        case COND_GE:
            return (flags[F_N] == flags[F_V]) ? 1 : 0;

        case COND_LT:
            return (flags[F_N] != flags[F_V]) ? 1 : 0;

        case COND_GT:
            return ((flags[F_Z] == 0) && (flags[F_N] == flags[F_V])) ? 1 : 0;

        case COND_LE:
            return ((flags[F_Z] == 1) || (flags[F_N] != flags[F_V])) ? 1 : 0;
    
        // default to AL(ways)
        case COND_AL:
        default:
            return 1;
    }
}

// CarryFrom is used for setting flags
// it returns a 1 when the addition results in a value greater
// than the maximum value held by an integer (2^32 - 1)
// and a 0 otherwise
uint8_t CarryFrom (uint32_t a, uint32_t b)
{
    double sum = a + b;
    return (sum > 4294967295) ? 1 : 0;
}

// BorrowFrom is used for setting flags
uint8_t BorrowFrom (uint32_t a, uint32_t b)
{
    double res;
    res = (double) (a - b);
        
    if (res < 0)
        return 1;

    return 0;
}

// OverflowFrom is used for setting flags
// it returns a 1 when the addition/subtraction results in an sign overflow
// i.e. if both addition operands have the same sign and the result does not
// or if the first subtraction operand and the result have different signs 
uint8_t OverflowFrom (int32_t a, int32_t b, uint8_t addition)
{
    int32_t res;

    if (addition)
    {
        res = a + b;

        if (a >= 0 && b >= 0)
            if (res < 0) return 1;

        if (a < 0 && b < 0)
            if (res >= 0) return 1;
    }
    else
    {
        res = a - b;
        
        if (a >= 0 && res < 0) return 1;
        
        if (a < 0 && res >= 0) return 1;
    }

    return 0;
}

// rotate a value to the left
// taken from Wikipedia "circular shift"
uint32_t rotate_left (uint8_t shift, uint32_t value)
{
    if ((shift &= sizeof (value)*8 - 1) == 0)
      return value;

    return (value << shift) | (value >> (sizeof (value)*8 - shift));
}

// rotate a value to the right
// taken from Wikipedia "circular shift"
uint32_t rotate_right (uint8_t shift, uint32_t value)
{
    if ((shift &= sizeof (value)*8 - 1) == 0)
      return value;

    return (value >> shift) | (value << (sizeof (value)*8 - shift));
}

// performs the MOV operation on an immediate value
// this simply returns the value as the calling function
// is responsible for storing the result in registers
uint32_t MOV (uint32_t operand)
{
    return operand;
}

// performs the ADD operation on a register and an immediate value
// adds the value 'operand' to
// the value from Rn
uint32_t ADD (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand)
{
    uint32_t res = (rn + operand);

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
        flags[F_C] = CarryFrom (rn, operand);
        flags[F_V] = OverflowFrom (rn, operand, 1);
    }

    return res;
}

// performs the SUB operation on a register value and an immediate
// subtracts the value 'operand' to
// the value from Rn
uint32_t SUB (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand)
{
    uint32_t res = (rn - operand);

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
        flags[F_C] = !BorrowFrom (rn, operand);
        flags[F_V] = OverflowFrom (rn, operand, 0);
    }

    return res;
}

// performs the AND operation on a register value and an immediate 
uint32_t AND (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand)
{
    uint32_t res = (rn & operand);

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
        //flags[F_C] = shifter_carry_out; // TODO
    }

    return res;
}

// performs the E(X)OR operation on a register value and an immediate 
uint32_t EOR (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand)
{
    uint32_t res = (rn ^ operand);

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
        //flags[F_C] = shifter_carry_out; // TODO
    }

    return res;
}


// performs the ORR operation on a register value and an immediate 
uint32_t ORR (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand)
{
    uint32_t res = (rn | operand);

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
        //flags[F_C] = shifter_carry_out; // TODO
    }

    return res;
}

// performs the BIC operation on a register value and an immediate 
uint32_t BIC (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand)
{
    uint32_t res = (rn & (!operand));

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
        //flags[F_C] = shifter_carry_out; // TODO
    }

    return res;
}

// performs the CMP operation
// updates the flags 
void CMP (uint8_t* flags, uint32_t rn, uint32_t operand)
{
    int32_t alu_out = (rn - operand);
    flags[F_N] = (alu_out >> 31);
    flags[F_Z] = (alu_out == 0) ? 1 : 0;
    flags[F_C] = CarryFrom (rn, operand);
    flags[F_V] = OverflowFrom (rn, operand, 1);
}

// performs the MUL operation
// that is, rs * rm
uint32_t MUL (uint8_t* flags, uint8_t s, uint8_t rm, uint8_t rs)
{
    uint32_t res = (rm * rs);

    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
    }
    return res;
}

// performs the MLA operation
// that is, rs * rm + rn
uint32_t MLA (uint8_t* flags, uint8_t s, uint8_t rm, uint8_t rs, uint8_t rn)
{
    uint32_t res =  ((rs * rm) + rn);
    
    if (s)
    {
        flags[F_N] = get_bit (res, 31);
        flags[F_Z] = (res == 0) ? 1 : 0;
    }

    return res;
}

// SVC instruction, used for debugging
// returns 0 for most instructions, when no halt is required
// returns 1 when the CPU has been halted
uint8_t SVC (uint32_t registers[], uint32_t operand)
{
    switch (operand)
    {
        // halt CPU
        case 0:
            return 1;

        // print out all register values
        case 1:
            print_register_dump (registers);
            printf ("\n");
            break;

        // print out R0 followed by \n
        case 2:
            printf ("%08X\n\n", registers[R_0]);
            break;
    }

    // return 'not-halted'
    return 0;
}
