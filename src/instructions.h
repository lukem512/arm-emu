/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

/* Non-general purpose register indexes */
#define R_0 0
#define R_1 1
#define R_2 2
#define R_3 3
#define R_4 4
#define R_5 5
#define R_6 6
#define R_7 7
#define R_8 8
#define R_9 9
#define R_10 10
#define R_11 11
#define R_12 12

#define R_PC 15
#define R_LR 14
#define R_SP 13

/* Flag definitions */
#define F_N 0
#define F_Z 1
#define F_C 2
#define F_V 3

/* Instruction type definitions */
#define INSTR_DP        0 // Data processing
#define INSTR_LS        1 // Load and store
#define INSTR_B         2 // Branch/with link
#define INSTR_SWI       3 // Software interrupt
#define INSTR_MUL       4
#define INSTR_UNKNOWN   -1

/* OpCode definitions */
#define OP_AND 0
#define OP_EOR 1
#define OP_SUB 2
#define OP_ADD 4
#define OP_CMP 10
#define OP_ORR 12
#define OP_MOV 13
#define OP_BIC 14

/* Conditional definitions */
#define COND_EQ 0
#define COND_NE 1
#define COND_MI 4
#define COND_PL 5
#define COND_GE 10
#define COND_LT 11
#define COND_GT 12
#define COND_LE 13
#define COND_AL 14

/* Shift definitions */
#define SHIFT_LSL_I 0
#define SHIFT_LSL_R 1
#define SHIFT_LSR_I 2
#define SHIFT_LSR_R 3
#define SHIFT_ASR_I 4
#define SHIFT_ASR_R 5
#define SHIFT_ROR_I 6
#define SHIFT_ROR_R 7

/* Function prototypes */
uint8_t condition_passed (uint8_t* flags, uint8_t cond);

uint32_t rotate_left (uint8_t shift, uint32_t value);
uint32_t rotate_right (uint8_t shift, uint32_t value);

uint32_t MOV (uint32_t operand);
uint32_t ADD (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand);
uint32_t SUB (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand);
uint32_t AND (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand);
uint32_t EOR (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand);
uint32_t ORR (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand);
uint32_t BIC (uint8_t* flags, uint8_t s, uint32_t rn, uint32_t operand);
void CMP (uint8_t* flags, uint32_t rn, uint32_t operand);

uint32_t MUL (uint8_t* flags, uint8_t s, uint8_t rm, uint8_t rs);
uint32_t MLA (uint8_t* flags, uint8_t s, uint8_t rm, uint8_t rs, uint8_t rn);

uint8_t SVC (uint32_t r[], uint32_t operand);

#endif
