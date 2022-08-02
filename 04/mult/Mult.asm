// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
//
// This program only needs to handle arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.

// Put your code here.

	@i 		// int i; A = &i;
	M=1 	// *A = 1
	@R2		// A = 2 == &R2
	M=0     // *A = 0
(LOOP)
	@R0		// A = 0
	D=M 	// D = 0
	@END    // A = &(END)
	D;JEQ   // if D != 0 then PC = PC + 1 else PC = A
	@R1     // A = 1
	D=M     // D = 1
	@END    // A = &(END)
	D;JEQ   // if D != 0 then PC = PC + 1 else PC = A


	@R1	    // A = 1
	D=M 	// D = *A
	@R2     // A = 2
	M=M+D   // *A = *A + D
	@R0     // A = 0
	M=M-1   // *A = *A - 1
	D=M     // D = *A
	@LOOP   // A = &(LOOP)
	D;JGT   // if D > 0 then PC = A else PC = PC + 1



(END)
	@END    // A = &(END)
	0;JMP   // PC = A






