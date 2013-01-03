arm-emu
=======

COMS12600 CW - implement an ARM emulator

This was a piece of coursework for the 1st year COMS12600 Introduction to Computer Architecture CS unit at the University of Bristol, set in March 2012. I received a mark of 70% for this piece of coursework.

The assignment was to create an ARM emulator in C and to write a Bubble Sort program in ASM that could be executed by it.

The emulator received a .emu file as input in the following format:

Your system will be fed input in the following format

     00000000 E3A00001
     00000004 E2800001
     00000008 00000042
     0000000C 00000000
     00000010 EAFFFFFB

which translates to:
     
     ADDRESS  INSTRUCTION    MNEMONIC         MEANING  
     ---------------------------------------------------     
     00000000 E3A00001        MOV              r0, #1
     00000004 E2800001        ADD              r0, #1
     00000008 00000042        DCD              0x42
     0000000C 00000000        DCD              0
     00000010 EAFFFFFB        B                FooLabel
