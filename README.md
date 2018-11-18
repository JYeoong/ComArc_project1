
# mips.sim.c
   이 코드는 c로 작성된 mips simulator로서 instruction을 입력받고 instruction마다 register, pc, clock cycles를 보여줍니다.
   
   
   # explain
   
   #### fetch
   16진수로 된 명령을 이진수로 변환하여 inst_bin 배열에 넣고 pc값 증가시킨다.
   
   #### decode
   instruction op(I,R,J 형식)에 따라 해당 field값을 구조체 inst_fm 멤버에 저장. 
   instruction 종류에 따른 control unit의 값도 설정.
   
   #### exe
   ALU연산 수행하거나 주소값 계산. 
   jr의 경우 점프해서 가야할 주소를 pc에 저장.
   j, jal, beq의 경우 다음에 수행해야 할 instruction의 주소값을 계산후 pc에 넣고 pc 반환.   
   lw, sw, add, addi, slti의 경우 여기서 반환되는 값이 mem과 wb의 파라미터로 쓰임.
   
   
   #### mem
   data_memmory를 쓰거나 읽을 때(MemRead나 MemWrite 값이 인가되었을 때) 실행. 즉, instruction이 lw나 sw일 때 실행.  
   lw의 경우 exe에서 얻은 data_memory의 주소에 값을 쓴다.
   sw의 경우 data_memory 주소에 있는 값을 반환.
   
   #### wb
   register에 입력할 때(RegWrite가 인가되었을 때) 실행. 
   구조체 inst_fm 멤버 rt, rd에 저장되어 있는 값을 인덱스로 써서 register에 인자로 받은 값 입력.
   
   
   
   
   #### 구조체 inst_fm
   instruction format의 field인 op, rs, rt, rd, funct, addr값 저장.
   addr에 address, immediate, target address값 저장.
   
   #### 구조체 control
   mem, wb 함수 호출 여부를 결정하는 RegWrite, MemRead, MemWrite값 저장.
   
   #### init
   instruction을 fetch하기 전에 pc, register, instruction memory, data memory, 2진수를 저장하는 배열 inst_bin, register name을 저장하는 regName 배열 초기화.
   
   #### itoa
   숫자를 문자로 바꿔주는 함수. regName 배열 초기화 때 쓰기 위해 구현.
   
   #### hexToBin
   instruction memory에 있는 값을 2진수로 변환. 각 비트는 inst_bin 배열에 저장. index 크기가 클수록 MSB.
   
   #### getVal
   inst_bin 배열에서 op, rs, rt, rd, funct, addr에 해당하는 비트의 시작과 끝을 인자로 받는다.
   2진수를 10진수로 바꾸는 작업을 함.
   
   #### getAddr
   I-format인 instruction의 addr값이 음수일 수도 있기 때문에 getAddr 함수 구현.
   MSB가 1인 경우에는 음수 10진수 값으로 바꿔주고 아닐 경우에는 getVal 함수 호출.
   
   
   
   
   # Contribution
   #### 이재영
   fetch에서 16진수->2진수 코드짜기, control unit과 ALU unit 조사 후 코딩의 방향성, 선택적 control unit 설정에 기여.  
   
   #### 우지현
   decode, mem 함수 구현. program logic 제시. 주어진 instructions들을 직접 simulate하여 디버깅 후 오류 수정(16->2진수, inst_bin 초기화 문제 등).  
  
