#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//some definitions
#define FALSE 0
#define TRUE 1

//clock cycles
long long cycles = 0;

// registers
int regs[32];
char regName[32][10];

// program counter
int pc;

// memory
#define INST_MEM_SIZE 32*1024
#define DATA_MEM_SIZE 32*1024
int inst_mem[INST_MEM_SIZE]; //instruction memory
int data_mem[DATA_MEM_SIZE]; //data memory
int inst_bin[32];            // hexadecimal -> binary

// instruction format
typedef struct {
	int op;
	int rs;
	int rt;
	int rd;
	long addr;
} inst_fm;

// control unit
typedef struct {
	int RegDst, ALUSrc, MemtoReg;     // 멀티플렉서 제어
	int RegWrite, MemRead, MemWrite;  // 레지스터 파일, 데이터 메모리에서 읽고 쓰는 것 제어
	int Branch;                       // 분기할지 말지 판단
	int ALUOp;                        // ALU
} control;

//misc. function
void init();
void fetch(int);
void decode(inst_fm*);
void exe(inst_fm);
void mem();
void wb();
void print_reg();

// function
void hexToBin(int);
int getVal(int, int);
void setControl(int a);
void setControl_R();
void setControl_beq();
void setControl_lw();
void setControl_sw();

control ctr;

//main
int main(int ac, char *av[])
{
	char done=FALSE;
	int i = 0;
	inst_fm inst;

	inst.op = inst.rs = inst.rt = inst.rd = inst.addr = 0; 
	init();
	
	while(!done)
	{
		fetch(i++);     //fetch an instruction from a instruction memory
		decode(&inst);    //decode the instruction and read data from register file
		exe(inst);       //perform the appropriate operation 
		mem();       //access the data memory
		wb();        //write result of arithmetic operation or data read from the data memory if required
		
		cycles++;    //increase clock cycle
		
		// check the exit condition 
		if(regs[9]==10)  //if value in $t1 is 10, finish the simulation
			done=TRUE;
			

		//if debug mode, print clock cycle, pc, reg
		if (av[1] == 0) {
			printf("Clock Cycles = %d\n", cycles);
			printf("Pc           = %d\n\n", pc);
			print_reg();
		}
	}

	printf("Clock Cycles = %d\n", cycles);
	printf("Pc           = %d\n\n", pc); 
	print_reg();

	return 0;
}


/* initialize all datapat elements
//fill the instruction and data memory
//reset the registers and enter the register name into regNameyg
*/
void init()
{
	FILE* fp = fopen("runme.hex","r");
	int i, idx = 0;
	long inst;
	char tmp[10] = {0, };

	if(fp == NULL) {
		fprintf(stderr,"Error opening file.\n");
		exit(2);
	}

	/* fill instruction memory */
	i=0;
	while(fscanf(fp, "%x", &inst)==1) {
		inst_mem[i++]=inst;
	}
	
	
	/*reset the registers*/
	for(i=0;i<32;i++) {
		regs[i]= inst_bin[i] = 0;
	}

	/*reset pc*/
	pc=0;

	/*regName*/
	strcpy(regName[idx++], "r0"); strcpy(regName[idx++], "at"); 
	strcpy(regName[idx++], "v0"); strcpy(regName[idx++], "v1");

	for (i = 0; i < 4; i++) {
		strcpy(regName[idx], "a");
		strcat(regName[idx++], itoa(i, tmp, 10));
	}

	for (i = 0; i < 8; i++) {
		strcpy(regName[idx], "t");
		strcat(regName[idx++], itoa(i, tmp, 10));
	}

	for (i = 0; i < 8; i++) {
		strcpy(regName[idx], "s");
		strcat(regName[idx++], itoa(i, tmp, 10));
	}	

	for (i = 8; i < 10; i++) {
		strcpy(regName[idx], "t");
		strcat(regName[idx++], itoa(i, tmp, 10));
	}

	strcpy(regName[idx++], "k0"); strcpy(regName[idx++], "k1");
	strcpy(regName[idx++], "gp"); strcpy(regName[idx++], "sp"); 
	strcpy(regName[idx++] ,"s8"); strcpy(regName[idx++], "ra");
}

void fetch(int i)
{
   int inst;
   
   inst=(int)inst_mem[i];
   hexToBin(inst);
   pc += 4;
}

void hexToBin(int inst)
{
   int i, index = 0;
   
   
   printf("inst: %d\n",inst);

   while(inst > 0) {
      if(inst % 2 == 1)
         inst_bin[index++] = 1;
      else
         index++;
   
      inst /= 2;
      
   }
}

void decode(inst_fm *inst)
{
	int i;

	inst->op = getVal(26, 31);

	switch (inst->op) {
		case 0:  // add, jr
			inst->op = getVal(0, 5);
			inst->rs = getVal(21, 25);
			inst->rt = getVal(16, 20);
			inst->rd = getVal(11, 15);
			break;
		case 2:  // j
		case 3:  // jal
			inst->addr = getVal(0, 25);
			break;
		case 4:  // beq
		case 8:  // addi
		case 10: // slti
		case 35: // lw
		case 43: // sw
			inst->rs = getVal(21, 25);
			inst->rt = getVal(16, 20);
			inst->addr = getVal(0, 15);
			break;
	}
}

int getVal(int st, int end)
{
	int i, num = 1; // num은 2*i^n, 이때 i는 i번째 비트
	int sum = 0; 

	for (i = st; i <= end; i++) {
		if (inst_bin[i] == 1)
			sum += num;
		
		num *= 2;
	}

	return sum;
}

void setControl(int a)
{
	switch(a)
	{
		case 0://add
			setControl_R();
		case 1://addi
			setControl_beq();
		case 2://jal
		case 3://j
		case 4://jr
			setControl_R();
		case 5://sw
			setControl_lw();
		case 6://slti
			setControl_beq();
		case 7://beq
			setControl_beq();
	}
}

void setControl_R()
{
	ctr.RegDst=1;
	ctr.ALUSrc=0;
	ctr.MemtoReg=0;
	ctr.RegWrite=1;
	ctr.MemRead=0;
	ctr.MemWrite=0;
	ctr.Branch=0;
	ctr.ALUOp=1;
}

void setControl_lw()
{
	ctr.RegDst=0;
	ctr.ALUSrc=1;
	ctr.MemtoReg=1;
	ctr.RegWrite=1;
	ctr.MemRead=1;
	ctr.MemWrite=0;
	ctr.Branch=0;
	ctr.ALUOp=0;
}

void setControl_sw()
{
	ctr.RegDst=-1;
	ctr.ALUSrc=1;
	ctr.MemtoReg=-1;
	ctr.RegWrite=0;
	ctr.MemRead=0;
	ctr.MemWrite=1;
	ctr.Branch=0;
	ctr.ALUOp=0;
}
void setControl_beq()
{
	ctr.RegDst=-1;
	ctr.ALUSrc=0;
	ctr.MemtoReg=-1;
	ctr.RegWrite=0;
	ctr.MemRead=0;
	ctr.MemWrite=0;
	ctr.Branch=1;
	ctr.ALUOp=1;
}

void print_reg()
{
	int i;

	for (i = 0; i < 32; i++)
		printf("R%-2d [%s] = %x\n", i, regName[i], regs[i]);
}