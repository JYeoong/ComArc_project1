#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//some definitions
#define FALSE 0
#define TRUE 1
//clock cycles
long long cycles;

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

//misc. function
void init();
void print_reg();

//main
int main(int ac, char *av[])
{
	char done=FALSE;
	init();

/*	while(!done)
	{
		fetch();     //fetch an instruction from a instruction memory
		decode();    //decode the instruction and read data from register file
		exe();       //perform the appropriate operation 
		mem();       //access the data memory
		wb();        //write result of arithmetic operation or data read from the data memory if required
		
		cycles++;    //increase clock cycle
		
		// check the exit condition 
		if(regs[9]==10)  //if value in $t1 is 10, finish the simulation
			done=TRUE;
			

		//if debug mode, print clock cycle, pc, reg
		if (av[1] == 0) {
			print_cycles();
			printf_pc();
			print_reg();
		}
	}

	print_cycles();  //print clock cycles
	print_pc();		 //print pc
	*/
	print_reg();	 //print registers

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

	if(fp == NULL)
	{
		fprintf(stderr,"Error opening file.\n");
		exit(2);
	}

	/* fill instruction memory */
	i=0;
	while(fscanf(fp, "%x", &inst)==1)
	{
		inst_mem[i++]=inst;
	}
	
	
	/*reset the registers*/
	for(i=0;i<32;i++)
	{
		regs[i]=0;
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

void print_reg()
{
	int i;

	for (i = 0; i < 32; i++)
		printf("R%2d [%s] = %d\n", i, regName[i], regs[i]);
}
