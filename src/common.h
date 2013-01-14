/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

void store (hashtable* memory, uint32_t addr, int size, uint8_t* data);
uint8_t load (hashtable* memory, uint32_t addr);
uint32_t load32 (hashtable* memory, unsigned int addr);
uint32_t get_bits (uint32_t instruction, uint8_t n, uint8_t size);
uint8_t get_bit (uint32_t instruction, uint8_t n);
uint8_t get_cond (uint32_t instruction);
uint8_t get_instruction_type (uint32_t instruction);

#endif
