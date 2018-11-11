#include <stdio.h>

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
//reset the registers
*/
void init()
{
	FILE* fp = fopen("runme.hex","r");
	int i;
	long inst;

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
	regName[0] = "r0"; regName[1] = "at"; regName[2] = "v0"; regName[3] = "v1"; 
	regName[4] = "a0"; regName[5] = "a1"; regName[6] = "a2"; regName[7] = "a3";
	regName[8] = "t0"; regName[9] = "t1"; regName[10] = "t2"; regName[11] = "t3";
	regName[12] = "t4"; regName[13] = "t5"; regName[14] = "t6"; regName[15] = "t7";
	regName[16] = "s0"; regName[17] = "s1"; regName[18] = "s2"; regName[19] = "s3";
	regName[20] = "s4"; regName[21] = "s5"; regName[22] = "s6"; regName[23] = "s7";
	regName[24] = "t8"; regName[25] = "t9"; regName[26] = "k0"; regName[27] = "k1";
	regName[28] = "gp"; regName[29] = "sp"; regName[30] = "s8"; regName[31] = "ra";
}

void print_reg()
{
	int i;

	for (i = 0; i < 32; i++)
		printf("R%2d [%s] = %d\n", i, regName[i], regs[i]);
}