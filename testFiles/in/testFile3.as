.entry LIST 
.extern W
MAIN2: add r3, LISTOFSTUFF 
LOOP: prn #48 
 lea W, r6
 inc r6 
 mov r3, K 
 sub r1, r4
 bne END 
 cmp K, #-6 
      LONGUNNECCESARYLABEL:     bne     %END    
 dec W
.entry MAIN1 
 jmp %LOOP 
 add L3, L3 
END: stop 
STR: .string "abcd" 
LIST: .data 6, -9 
 .data -100 
M: .data 31 
.extern L3 
MAIN1: add r3, LIST 
 lea STR, r6
 inc r6 
 mov r3, K 


 sub r1, r4 
 bne ENDOFMAIN 


LONGLABEL: cmp val1, #-6 


 bne %END 
 dec K 
 jmp %LOOP 
ENDOFMAIN: stop 
STRING: .string "abcd" 
LISTOFSTUFF: .data 6, -9 
 .data -100
K: .data 31 
.extern val1