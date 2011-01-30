#pragma asm

/* ; Convert a double to DCFE/DCFP form */

/* ; a1,a2 = double, a3 -> mem */

	EXPORT	storedcfe
	EXPORT	storedcfp

.storedcfe
	STMFD	SP!,{R0, R1}
	LDFD	F0, [SP], #8
	STFE	F0, [R2]
	MOVS	PC, R14

.storedcfp
	STMFD	SP!,{R0, R1}
	LDFD	F0, [SP], #8
	STFP	F0, [R2]
	MOVS	PC, R14

#pragma endasm
