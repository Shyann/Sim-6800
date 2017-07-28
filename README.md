# Sim-6800
Motorola 6800 Emulator


  Operation               |Mnem.|Immed.|Direct|Index |Extend|Inher.|Operation |CC Reg|
                          |     |OP·~·#|OP·~·#|OP·~·#|OP·~·#|OP·~·#|          |HINZVC|

  Add                     |ADDA |8B·2·2|9B·3·2|AB·5·2|BB·4·3|  · · |A=A+M     |T•TTTT|
                          |ADDB |CB·2·2|DB·3·2|EB·5·2|FB·4·3|  · · |B=B+M     |T•TTTT|
  Add Accumulators        |ABA  |  · · |  · · |  · · |  · · |1B·2·1|A=A+B     |T•TTTT|
  Add with Carry          |ADCA |89·2·2|99·3·2|A9·5·2|B9·4·3|  · · |A=A+M+C   |T•TTTT|
                          |ADCB |C9·2·2|D9·3·2|E9·5·2|F9·4·3|  · · |B=B+M+C   |T•TTTT|
  And                     |ANDA |84·2·2|94·3·2|A4·5·2|B4·4·3|  · · |A=A.M     |••TTR•|
                          |ANDB |C4·2·2|D4·3·2|E4·5·2|F4·4·3|  · · |B=B.M     |••TTR•|
  Bit Test                |BITA |85·2·2|95·3·2|A5·5·2|B5·4·3|  · · |A.M       |••TTR•|
                          |BITB |C5·2·2|D5·3·2|E5·5·2|F5·4·3|  · · |B.M       |••TTR•|
  Clear                   |CLR  |  · · |  · · |6F·7·2|7F·6·3|  · · |M=00      |••RSRR|
                          |CLRA |  · · |  · · |  · · |  · · |4F·2·1|A=00      |••RSRR|
                          |CLRB |  · · |  · · |  · · |  · · |5F·2·1|B=00      |••RSRR|
  Compare                 |CMPA |81·2·2|91·3·2|A1·5·2|B1·4·3|  · · |A-M       |••TTTT|
                          |CMPB |C1·2·2|D1·3·2|E1·5·2|F1·4·3|  · · |B-M       |••TTTT|
  Compare Accumulators    |CBA  |  · · |  · · |  · · |  · · |11·2·1|A-B       |••TTTT|
  Complement 1's          |COM  |  · · |  · · |63·7·2|73·6·3|  · · |M=-M      |••TTRS|
                          |COMA |  · · |  · · |  · · |  · · |43·2·1|A=-A      |••TTRS|
                          |COMB |  · · |  · · |  · · |  · · |53·2·1|B=-B      |••TTRS|
  Complement 2's          |NEG  |  · · |  · · |60·7·2|70·6·3|  · · |M=00-M    |••TT12|
                          |NEGA |  · · |  · · |  · · |  · · |40·2·1|A=00-A    |••TT12|
                          |NEGB |  · · |  · · |  · · |  · · |50·2·1|B=00-B    |••TT12|
  Decimal Adjust          |DAA  |  · · |  · · |  · · |  · · |19·2·1|*         |••TTT3|
  Decrement               |DEC  |  · · |  · · |6A·7·2|7A·6·3|  · · |M=M-1     |••TT4•|
                          |DECA |  · · |  · · |  · · |  · · |4A·2·1|A=A-1     |••TT4•|
                          |DECB |  · · |  · · |  · · |  · · |5A·2·1|B=B-1     |••TT4•|
  Exclusive OR            |EORA |88·2·2|98·3·2|A8·5·2|B8·4·3|  · · |A=A(+)M   |••TTR•|
                          |EORB |C8·2·2|D8·3·2|E8·5·2|F8·4·3|  · · |B=B(+)M   |••TTR•|
  Increment               |INC  |  · · |  · · |6C·7·2|7C·6·3|  · · |M=M+1     |••TT5•|
                          |INCA |  · · |  · · |  · · |  · · |4C·2·1|A=A+1     |••TT5•|
                          |INCB |  · · |  · · |  · · |  · · |5C·2·1|B=B+1     |••TT5•|
  Load Accumulator        |LDAA |86·2·2|96·3·2|A6·5·2|B6·4·3|  · · |A=M       |••TTR•|
                          |LDAB |C6·2·2|D6·3·2|E6·5·2|F6·4·3|  · · |B=M       |••TTR•|
  OR, Inclusive           |ORAA |8A·2·2|9A·3·2|AA·5·2|BA·4·3|  · · |A=A+M     |••TTR•|
                          |ORAB |CA·2·2|DA·3·2|EA·5·2|FA·4·3|  · · |B=B+M     |••TTR•|
  Push Data               |PSHA |  · · |  · · |  · · |  · · |36·4·1|Msp=A, *- |••••••|
                          |PSHB |  · · |  · · |  · · |  · · |37·4·1|Msp=B, *- |••••••|
  Pull Data               |PULA |  · · |  · · |  · · |  · · |32·4·1|A=Msp, *+ |••••••|
                          |PULB |  · · |  · · |  · · |  · · |33·4·1|B=Msp, *+ |••••••|
  Rotate Left             |ROL  |  · · |  · · |69·7·2|79·6·3|  · · |Memory  *1|••TT6T|
                          |ROLA |  · · |  · · |  · · |  · · |49·2·1|Accum A *1|••TT6T|
                          |ROLB |  · · |  · · |  · · |  · · |59·2·1|Accum B *1|••TT6T|
  Rotate Right            |ROR  |  · · |  · · |66·7·2|76·6·3|  · · |Memory  *2|••TT6T|
                          |RORA |  · · |  · · |  · · |  · · |46·2·1|Accum A *2|••TT6T|
                          |RORB |  · · |  · · |  · · |  · · |56·2·1|Accum B *2|••TT6T|
  Arithmetic Shift Left   |ASL  |  · · |  · · |68·7·2|78·6·3|  · · |Memory  *3|••TT6T|
                          |ASLA |  · · |  · · |  · · |  · · |48·2·1|Accum A *3|••TT6T|
                          |ASLB |  · · |  · · |  · · |  · · |58·2·1|Accum B *3|••TT6T|
  Arithmetic Shift Right  |ASR  |  · · |  · · |67·7·2|77·6·3|  · · |Memory  *4|••TT6T|
                          |ASRA |  · · |  · · |  · · |  · · |47·2·1|Accum A *4|••TT6T|
                          |ASRB |  · · |  · · |  · · |  · · |57·2·1|Accum B *4|••TT6T|
  Logic Shift Right       |LSR  |  · · |  · · |64·7·2|74·6·3|  · · |Memory  *5|••TT6T|
                          |LSRA |  · · |  · · |  · · |  · · |44·2·1|Accum A *5|••TT6T|
                          |LSRB |  · · |  · · |  · · |  · · |54·2·1|Accum B *5|••TT6T|
  Store Accumulator       |STAA |  · · |97·4·2|A7·6·2|B7·5·3|  · · |M=A       |••TTR•|
                          |STAB |  · · |D7·4·2|E7·6·2|F7·5·3|  · · |M=B       |••TTR•|
  Subtract                |SUBA |80·2·2|90·3·2|A0·5·2|B0·4·3|  · · |A=A-M     |••TTTT|
                          |SUBB |C0·2·2|D0·3·2|E0·5·2|F0·4·3|  · · |B=B-M     |••TTTT|
  Subtract Accumulators   |SBA  |  · · |  · · |  · · |  · · |10·2·1|A=A-B     |••TTTT|
  Subtract with Carry     |SBCA |82·2·2|92·3·2|A2·5·2|B2·4·3|  · · |A=A-M-C   |••TTTT|
                          |SBCB |C2·2·2|D2·3·2|E2·5·2|F2·4·3|  · · |B=B-M-C   |••TTTT|
  Transfer Accumulators   |TAB  |  · · |  · · |  · · |  · · |16·2·1|B=A       |••TTR•|
                          |TBA  |  · · |  · · |  · · |  · · |17·2·1|A=B       |••TTR•|
  Test, Zero/Minus        |TST  |  · · |  · · |6D·7·2|7D·6·3|  · · |M-00      |••TTRR|
                          |TSTA |  · · |  · · |  · · |  · · |4D·2·1|A-00      |••TTRR|
                          |TSTB |  · · |  · · |  · · |  · · |5D·2·1|B-00      |••TTRR|

Operation               |Mnem.|Immed.|Direct|Index |Extend|Inher.|Operation |CC Reg|

  Compare Index Register  |CPX  |8C·3·3|9C·4·2|AC·6·2|BC·5·3|  · · |Formula 1 |••7T8•|
  Decrement Index Register|DEX  |  · · |  · · |  · · |  · · |09·4·1|X=X-1     |•••T••|
  Dec Stack Pointer       |DES  |  · · |  · · |  · · |  · · |34·4·1|SP=SP-1   |••••••|
  Inc Index Regster       |INX  |  · · |  · · |  · · |  · · |08·4·1|X=X+1     |•••T••|
  Inc Stack Pointer       |INS  |  · · |  · · |  · · |  · · |31·4·1|SP=SP+1   |••••••|
  Load Index Register     |LDX  |CE·3·3|DE·4·2|EE·6·2|FE·5·3|  · · |Formula 2 |••9TR•|
  Load Stack Pointer      |LDS  |8E·3·3|9E·4·2|AE·6·2|BE·5·3|  · · |Formula 3 |••9TR•|
  Store Index Register    |STX  |  · · |DF·5·2|EF·7·2|FF·6·3|  · · |Formula 4 |••9TR•|
  Store Stack Pointer     |STS  |  · · |9F·5·2|AF·7·2|BF·6·3|  · · |Formula 5 |••9TR•|
  Index Reg > Stack Pnter |TXS  |  · · |  · · |  · · |  · · |35·4·1|SP=X-1    |••••••|
  Stack Ptr > Index Regtr |TSX  |  · · |  · · |  · · |  · · |30·4·1|X=SP+1    |••••••|

Operation: Branch If    |Mnem.|Immed.|Direct|Index |Extend|Inher.|Operation |CC Reg|

  Always                  |BRA  |  · · |20·4·2|  · · |  · · |  · · |none      |••••••|
  Carry is Clear          |BCC  |  · · |24·4·2|  · · |  · · |  · · |C=0       |••••••|
  Carry is Set            |BCS  |  · · |25·4·2|  · · |  · · |  · · |C=1       |••••••|
  Equals Zero             |BEQ  |  · · |27·4·2|  · · |  · · |  · · |Z=1       |••••••|
  Greater or Equal to Zero|BGE  |  · · |2C·4·2|  · · |  · · |  · · |N(+)V=0   |••••••|
  Greater than Zero       |BGT  |  · · |2E·4·2|  · · |  · · |  · · |Z+N(+)V=0 |••••••|
  Higher                  |BHI  |  · · |22·4·2|  · · |  · · |  · · |C+Z=0     |••••••|
  Less or Equal than Zero |BLE  |  · · |2F·4·2|  · · |  · · |  · · |Z+N(+)V=1 |••••••|
  Lower or Same           |BLS  |  · · |23·4·2|  · · |  · · |  · · |C+Z=1     |••••••|
  Less Than Zero          |BLT  |  · · |2D·4·2|  · · |  · · |  · · |N(+)V=1   |••••••|
  Minus                   |BMI  |  · · |2B·4·2|  · · |  · · |  · · |N=1       |••••••|
  Not Zero                |BNE  |  · · |26·4·2|  · · |  · · |  · · |Z=0       |••••••|
  Overflow Clear          |BVC  |  · · |28·4·2|  · · |  · · |  · · |V=0       |••••••|
  Overflow Set            |BVS  |  · · |29·4·2|  · · |  · · |  · · |V=1       |••••••|
  Plus                    |BPL  |  · · |2A·4·2|  · · |  · · |  · · |N=0       |••••••|

Operation               |Mnem.|Immed.|Direct|Index |Extend|Inher.|Operation |CC Reg|

  Branch to Subroutine    |BSR  |  · · |8D·8·2|  · · |  · · |  · · |          |••••••|
  Jump                    |JMP  |  · · |  · · |6E·4·2|7E·3·3|  · · |          |••••••|
  Jump to Subroutine      |JSR  |  · · |  · · |AD·8·2|BD·9·3|  · · |          |••••••|
  No Operation            |NOP  |  · · |  · · |  · · |  · · |01·2·1|          |••••••|
  Return from Interrupt   |RTI  |  · · |  · · |  · · |  · · |3B·A·1|          |AAAAAA|
  Return from Subroutine  |RTS  |  · · |  · · |  · · |  · · |39·5·1|          |••••••|
  Software Interrupt      |SWI  |  · · |  · · |  · · |  · · |3F·C·1|          |•S••••|
  Wait For Interrupt      |WAI  |  · · |  · · |  · · |  · · |3E·9·1|          |•B••••|

Operation               |Mnem.|Immed.|Direct|Index |Extend|Inher.|Operation |CC Reg|

  Clear Carry             |CLC  |  · · |  · · |  · · |  · · |0C·2·1|C=0       |•••••R|
  Clear Interrupt         |CLI  |  · · |  · · |  · · |  · · |0E·2·1|I=0       |•R••••|
  Clear Overflow          |CLV  |  · · |  · · |  · · |  · · |0A·2·1|V=0       |••••R•|
  Set Carry               |SEC  |  · · |  · · |  · · |  · · |0D·2·1|C=1       |•••••S|
  Set Interrupt           |SEI  |  · · |  · · |  · · |  · · |0F·2·1|I=1       |•S••••|
  Set Overflow            |SEV  |  · · |  · · |  · · |  · · |0B·2·1|V=1       |••••S•|
  CCR=Accumulator A       |TAP  |  · · |  · · |  · · |  · · |06·2·1|CCR=A     |CCCCCC|
  Accumlator A=CCR        |TPA  |  · · |  · · |  · · |  · · |07·2·1|A=CCR     |••••••|

OP  Operation Code, in Hexadecimal
  ~   Number of MPU cycles required
  #   Number of program bytes required
  +   Arithmetic Plus
  -   Arithmetic Minus
  +   Boolean AND
  Msp Contents of Memory pointed to be Stack Pointer
  +   Boolean Inclusive OR
  (+) Boolean Exclusive OR (XOR)
  *   Converts Binary Addition of BCD Characters into BCD Format
  *-  SP=SP-1
  *+  SP=SP+1

Condition Code Register Legend
   • Not Affected
   R Reset (0, Low)
   S Set   (1, High)
   T Tests and sets if True, cleared otherise
   1 Test: Result=10000000?
   2 Test: Result=00000000?
   3 Test: Decimal value of most significant BCD character greater than nine?
           (Not cleared if previously set)
   4 Test: Operand=10000000 prior to execution?
   5 Test: Operand=01111111 prior to execution?
   6 Test: Set equal to result or N(+)C after shift has occurred.
   7 Test: Sign bit of most significant byte or result=1?
   8 Test: 2's compliment overflow from subtraction of least 
           significant bytes?
   9 Test: Result less than zero? (Bit 15=1)
   A Load Condition Code Register from Stack.
   B Set when interrupt occurs.  If previously set, a NMI is 
        required to exit the wait state.
   C Set according to the contents of Accumulator A.

 
