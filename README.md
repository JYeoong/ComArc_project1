
# mips.sim.c
   이 코드는 c로 작성된 mips simulator로서 instruction을 입력받고 instruction마다 register, pc, clock cycles를 보여줍니다.
   
   
   # explain
   #### fetch
   16진수로 된 명령을 이진수로 inst_bin 배열에 받고 pc를 조정합니다.
   
   #### decode
   instruction op(I,R,J 형식)에 따라 값을 적절히 현재 instruction 배열에 넣어줍니다. instruction에 따른 control unit의 값도 설정합니다.
   
   #### exe
   Alu연산이나 주소값을 계산합니다. lw, sw, add, addi, lw, slti의 경우 여기서 반환되는 값이 이후, mem과 wb의 파라미터로 쓰입니다.
   
   #### mem
   data_memmory를 쓰거나 읽을 때(memoryRead나 memoryWrite 값이 인가되었을 때), 실행됩니다. 즉, instruction의 op가 lw나 sw일 때 실행됩니다.  
   exe에서 얻은 접근해야 할 data_memory의 주소에 값을 쓰거나 주소에 있는 값을 반환합니다.
   
   #### wb
   register에 입력할 때(regWrite가 인가되었을 때) 실행됩니다. regs[]의 rt 혹은 rd index에 값을 기록합니다.
   
   
   # Contribution
   #### 재영
   fetch에서 16진수->2진수 코드짜기, control unit과 alu unit 조사 후 코딩의 방향성,control unit의 ,에 기여.  
   
   #### 지현
   손수 주어진 instructions들을 simulate하여 디버깅과 오류 수정(2의 보수법, inst_bin 초기화 문제 등)에 큰 기여.  
   decode, regs초기화 등 코드 작성과 정리에 기여.  
