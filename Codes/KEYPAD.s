        AREA GPIOData, READWRITE, DATA
        THUMB

; GPIO Ports Base Addresses
GPIO_PORTC_BASE          EQU     0x40006000
GPIO_PORTD_BASE          EQU     0x40007000

        AREA KEYPAD, CODE, READONLY
        THUMB
        EXPORT KEYPAD_Read_ASM

;--------------------------------------------------------------------
; KEYPAD_Read:  Reads the 4x4 keypad
;               Returns in R0:
;                 - Key ID if a key is pressed
;                 - 0xFF if no key is pressed
;--------------------------------------------------------------------
KEYPAD_Read_ASM
        ; Push only the registers we need to preserve
        PUSH {R4-R7, LR}       

        MOV   R5, #0x01        ; R5 = row pattern = 0x01, then 0x02, 0x04, 0x08
ScanLoop
        ; Invert R5 for the row activation pattern (logic low means pressed)
        EOR   R6, R5, #0xF

        ; Write row pattern to Port D (lower nibble, bits 0..3)
        LDR   R0, =GPIO_PORTD_BASE
        LDR   R1, [R0, #0x3FC] ; Read Port D register
        AND   R1, R1, #0xF0    ; Clear lower nibble
        ORR   R1, R1, R6       ; Put row pattern in lower nibble
        STR   R1, [R0, #0x3C]  ; Write back

        ; Now read columns from PD6, PD7 and PC4, PC5
        LDR   R0, =GPIO_PORTD_BASE
        LDR   R2, =GPIO_PORTC_BASE

        LDR   R1, [R0, #0x3FC] ; PD data
        LDR   R3, [R2, #0x3FC] ; PC data

        AND   R1, R1, #0xC0    ; PD6, PD7 => 0x40, 0x80 
        AND   R3, R3, #0x30    ; PC4, PC5 => 0x10, 0x20
        ORR   R4, R1, R3       ; combine into R4

        ; Check if no key pressed => 0x78 means all columns read '1'
        CMP   R4, #0xF0
        BEQ   CheckNextRow

        ; Debounce
        BL    Delay_40ms

        ; Re-read columns
        LDR   R0, =GPIO_PORTD_BASE
        LDR   R2, =GPIO_PORTC_BASE

        LDR   R1, [R0, #0x3FC]
        LDR   R3, [R2, #0x3FC]
        AND   R1, R1, #0xC0
        AND   R3, R3, #0x30
        ORR   R4, R1, R3

        CMP   R4, #0xF0
        BEQ   CheckNextRow

        ; If here => a key is definitely pressed, identify which one
        BL    IdentifyKey

        ; IdentifyKey puts the key ID in R0
        B     ReturnFromKeypad

CheckNextRow
        ; Move to next row
        LSL   R5, R5, #1       
        CMP   R5, #0x08        ; If we exceed row 4, no key found
        BLS   ScanLoop

        ; If row > 0x08, no key was pressed in the entire scan
        MOV   R0, #0xFF
        B     ReturnFromKeypad

;--------------------------------------------------------------------
; IdentifyKey:
;   Expects R5 = active row mask (0x01, 0x02, 0x04, 0x08)
;   Expects R4 = combined column bits in bits PD6, PD7, PC4, PC5
;   Returns R0 = key ID
;--------------------------------------------------------------------
IdentifyKey
        ; Identify key ID from row and column indices
        MOV     R0, #0                  ; Initialize key ID at 0
        CMP     R5, #0x02               ; Check if Row 2 is active
        ADDEQ   R0, R0, #4              ; Add offset for Row 2
        CMP     R5, #0x04               ; Check if Row 3 is active
        ADDEQ   R0, R0, #8              ; Add offset for Row 3

		ORR		R3,R3,R1
        ; Check columns for exact key in active row
		MOV		R4,#0x00
		AND		R4,R3,#0x10
		CMP		R4,#0x10
		ADDNE	R0,R0,#0
		
		MOV		R4,#0x00
		AND		R4,R3,#0x20
		CMP		R4,#0x20
		ADDNE	R0,R0,#1
		
		MOV		R4,#0x00
		AND		R4,R3,#0x40
		CMP		R4,#0x40
		ADDNE	R0,R0,#2
		
		MOV		R4,#0x00
		AND		R4,R3,#0x80
		CMP		R4,#0x80
		ADDNE	R0,R0,#3
        
        BX      LR                      ; Return from IdentifyKey

ReturnFromKeypad
        POP {R4-R7, PC}  ; balanced pop => returns to C code that called KEYPAD_Read

;--------------------------------------------------------------------
; Simple delay ~40ms
Delay_40ms 				;100ms
        MOV32 R0, #640000
delay_loop
        SUBS  R0, R0, #1
        BNE   delay_loop
        BX    LR

        END
