;VIC  C-Compiler V2.0  (c) VE/DC 1995                            
;	USERS TEXT AREA......         
.main:
	LD	DE,C301+0
	PUSH	DE
	CALL	.puts
	POP	BC
	RET
C301:

	DEFB	72,101,108,108,111,46,46,46,0
	GLOBAL	.main
	EXTERNAL	.puts
	DSEG

;0 ERROR(S) 
	END
