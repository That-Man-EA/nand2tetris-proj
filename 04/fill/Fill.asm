

//r0 = kb
//r1 = 8192
//r2 = i
//r3 = color
//R4 = SCREEN + i

(INFINITE_LOOP)
	@R2
	M=0 //i = 0 
	
	@KBD
	D = M

	@CHOOSE_WHITE
	D;JEQ

	
	
(CHOOSE_BLACK)
	@R3
	M=-1
	@AFTER_CHOOSE
	0;JMP

(CHOOSE_WHITE)
	@R3
	M=0

(AFTER_CHOOSE)
	@R1
	D=M
	@R2
	D=M-D

	@AFTER_LOOP
    D;JLE

    @R2
    D=M //D=i
    @SCREEN
    A=A+D //A=SCREEN+i
    D=A   //D=A

    @R4
    D=A

    //RAM[R4] = R3
	@R3
	D=M




    //increment i
    @R2
    M=M+1

    @AFTER_CHOOSE
    0;JMP
    


(AFTER_LOOP)
@INFINITE_LOOP
0;JMP