#include <os_cpu_i.h>
#include <avr/interrupt.h>

	.extern OSIntExit
	.extern OSIntNesting
	.extern OSTCBCur
	.extern UCOS_TickHandler

	.global TCC0_OVF_vect

;; System tick ISR.
TCC0_OVF_vect:
	push_all

	lds r16,OSIntNesting       ; Notify uC/OS-II of ISR
	inc r16
	sts OSIntNesting,r16
	
	cpi r16,1                  ; if (OSIntNesting == 1) {
	brne 1f
	
	save_sp                    ; X = SP
	lds r28,OSTCBCur           ; OSTCBCur->OSTCBStkPtr = X
	lds r29,OSTCBCur+1
	
	st y+,r26
	st y+,r27                   ; }

1:	call UCOS_TickHandler       ; Handle the tick ISR
	
	call OSIntExit             ; Notify uC/OS-II about end of ISR

	pop_all

	reti

