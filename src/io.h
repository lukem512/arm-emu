/*
 * ARM emulator
 * Luke Mitchell
 *
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include "hash.h"

void print_usage (char* name);

void print_memory_dump (hashtable* memory);
void print_register_dump (uint32_t r[]);
void print_trace (uint32_t r[], uint32_t instr);

char* instr_to_string (uint32_t instr);
char* opcode_to_string (uint8_t opcode);
char* cond_to_string (uint8_t cond);

#endif
