# Altivec #

The ArchC Architecture Description Language:
http://www.lsc.ic.unicamp.br/publications/ARBAAB05.pdf

PowerISA Specification: in the Chapter 5 we can find the Altivec's Instruction Set:
http://atum.caco.ic.unicamp.br/~ribamar/PowerISA_203.Public.pdf


# Details #

These are the instructions section we're implementing (starting in the specification's pdf page 160):

```
 5.7 - Vector Storage Access Instructions.
 5.8 - Vector Permuting and Formatting Instructions.
 5.9 - Vector Integer Instructions.
 5.10 - Vector Floating-Point Instructions.
 5.11 - Vector Status and Control Register Instructions.  
```

These sections can be splited as this:

```
 5.7 - Vector Storage Access Instructions.
  
 7.1 - Storage access exceptions - texto/zero instruções.
 7.2 - Vector Load Instructions - 4 instruções.
 7.3 - Vector Store Instructions - 4 instruções. 
 7.4 - Vector Alignment Support Instructions -  2 instruções.
 Total - 10 instruções. 

 5.8 - Vector Permuting and Formatting Instructions.
 
 8.1 - Vector Pack and Unpack Instructions - 15 instruções.
 8.2 - Vector Merge Instructions -  6 instruções.
 8.3 - Vector Splat Instructions -  5 instruções.
 8.4 - Vector Permute Instruction - 1 instrução.
 8.5 - Vector Select Instruction - 1 instrução.
 8.6 - Vector Shift Instructions - 5 instruções.
 Total - 32 instruções.
 
 5.9 - Vector Integer Instructions.
 
 9.1.1 - Vector Integer Arithmetic Instructions - Add - 10 instruções.
 9.1.2 - Vector Integer Arithmetic Instructions - Subtract - 10 instruções.
 9.1.3 - Vector Integer Arithmetic Instructions - Multiply - 8 instruções.
 9.1.4 - Vector Integer Arithmetic Instructions - Multiply-Add/Sum - 9 instruções.
 9.1.5 - Vector Integer Arithmetic Instructions - Sum-Across - 5 instruções.
 9.1.6 - Vector Integer Arithmetic Instructions - Average - 6 instruções.
 9.1.7 - Vector Integer Arithmetic Instructions - Maximum and Minimum - 12 instruções.
 (sub total 60)
 9.2 - Vector Integer Compare Instructions - 9 instruções (ou 18 se contarmos as instruções ponto). 
 9.3 - Vector Logical Instructions - 5 instruções.
 9.4 - Vector Integer Rotate and Shift Instructions - 12 instruções.
 Total - 86 instruções.
 
 5.10 - Vector Floating-Point Instructions.
 
 10.1 - Vector Floating-Point Arithmetic Instructions - 4 instruções.
 10.2 - Vector Floating-Point Maximum and Minimum Instructions - 2 instruções.
 10.3 - Vector Floating-Point Rounding and Conversion Instructions - 8 instruções.
 10.4 - Vector Floating-Point Compare Instructions - 4 instruções.
 10.5 - Vector Floating-Point Estimate Instructions - 4 instruções.
 Total - 22 instruções.
 
 5.11 - Vector Status and Control Register Instructions 2 instruções 
 
 Total de Instruções: 152 instruções. 

```