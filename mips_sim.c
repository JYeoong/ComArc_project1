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
	int funct;
	long addr;
} inst_fm;

// control unit
typedef struct {
	int reg_w;
	int mem_r;
	int mem_w;
} control;

//misc. function
void init();
void fetch(int);
void decode(inst_fm*, control*);
int exe(inst_fm);
int mem(int, control, int);
void wb(int, inst_fm);
void print_reg();

// function
char* itoa(int val, char*, int);
void hexToBin(int);
int getVal(int, int);

//main
int main(int ac, char *av[])
{
	char done=FALSE;
	int i = 0, result;
	inst_fm inst;
	control ctrl;

	inst.op = inst.rs = inst.rt = inst.rd = inst.addr = 0; 
	ctrl.reg_w = ctrl.mem_r = ctrl.mem_w = 0;
	init();
	
	while(!done) {
		fetch(pc/4);     //fetch an instruction from a instruction memory
		decode(&inst, &ctrl);    //decode the instruction and read data from register file
		result = exe(inst);       //perform the appropriate operation 
		
		// lw, sw
		if (ctrl.mem_r || ctrl.mem_w)
			result = mem(result, ctrl, inst.rt);       //access the data memory
		
		// add, addi, lw, slti
		if (ctrl.reg_w)
			wb(result, inst);        //write result of arithmetic operation or data read from the data memory if required

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

char *itoa(int val, char *buf, int radix)
{
	char *p = buf;
	int t;

	if (val == 0)
		*p++ = '0';
	
	while (val) {
		if (radix <= 10)
			*p++ = (val%radix)+'0';
		else {
			t = val % radix;

			if (t <= 9)
				*p++ = t + '0';
			else
				*p++ = t - 10 + 'a';
		}

		val /= radix;
	}

	*p = '\0';

	return buf;
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
   
   while(inst > 0) {
      if(inst % 2 == 1)
         inst_bin[index++] = 1;
      else
         index++;
   
      inst /= 2;
      
   }
}

void decode(inst_fm *inst, control *ctrl)
{
	int i;

	inst->op = getVal(26, 31);

	switch (inst->op) {
		case 0:  // add, jr
			inst->funct = getVal(0, 5);
			inst->rs = getVal(21, 25);
			inst->rt = getVal(16, 20);
			inst->rd = getVal(11, 15);

			if (inst->funct == 32)
				ctrl->reg_w = 1;
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

			// lw일 경우 RegWrite, MemRead == 1
			if (inst->op == 35)
				ctrl->reg_w = ctrl->mem_r = 1;

			// sw일 경우 MemWrite == 1
			if (inst->op == 43)
				ctrl->mem_w = 1;

			// addi, slti일 경 RegWrite == 1
			if (inst->op == 8 || inst->op == 10)
				ctrl->reg_w = 1;

			break;
	}
}

int getVal(int st, int end)
{
	int i, num = 1; // num은 2*i^n, 이때 i는 i번째 비트값
	int sum = 0; 

	for (i = st; i <= end; i++) {
		if (inst_bin[i] == 1)
			sum += num;
		
		num *= 2;
	}

	return sum;
}


int exe(inst_fm inst)
{
	int result = 0;

	switch (inst.op) {
		case 0:
			if (inst.funct == 8)  // jr
				pc = regs[inst.rs];
			else {  // add
				result = regs[inst.rs] + regs[inst.rt];
				return result;
			}
			break;
		case 2: // j
			pc = inst.addr*4;
			break;
		case 3: // jal
			regs[31] = pc;
			pc = inst.addr*4;
			break;
		case 4: // beq
			if (regs[inst.rs] == regs[inst.rt])
				pc = pc + inst.addr*4;
			break;
		case 8: // addi
			result = regs[inst.rs] + inst.addr;
			return result;
		case 10: // slti
			result = regs[inst.rs] < inst.addr ? 1 : 0;
			return result;
		case 35: // lw
			result = regs[inst.rs] + inst.addr;
			return result;
		case 43: // sw
			result = regs[inst.rs] + inst.addr;
			return result;
	}

	return 0;
}

// lw, sw
int mem(int res, control ctrl, int regnum)
{
	if (ctrl.mem_r) // lw
		return data_mem[res];
	else // sw
		data_mem[res] = regs[regnum];

	return 0;
}

// add, addi, slit, lw
void wb(int res, inst_fm inst)
{
	if (inst.op == 32)
		regs[inst.rd] = res;
	else
		regs[inst.rt] = res;
}

void print_reg()
{
	int i;

	for (i = 0; i < 32; i++)
		printf("R%-2d [%s] = %x\n", i, regName[i], regs[i]);
}
