#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//some definitions
#define FALSE 0
#define TRUE 1
#define ADDR long long
#define BOOL char

typedef struct _MEMACCESS{
    ADDR addr;
    BOOL is_read;
} MEMACCESS;

typedef enum _RPL{LRU=0, RAND=1} RPL;

//misc. function
FILE* fp = 0;
char trace_file[100]="memtrace.trc";
MEMACCESS** cache = NULL;
int** LRU_counter = NULL;
BOOL read_new_memaccess(MEMACCESS*);  //read new memory access from the memory trace file (already implemented)


//configure the cache
int init_cache(int, int, int, RPL);

//check if the memory access hits on the cache
BOOL isHit(ADDR, int, int, int, int);

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR, int, int, int, RPL);


//print the simulation statistics
void print_stat(int, int, int, RPL, int, int);

void update_LRU(int, int, int, int);
int logB(int);


//main
int main(int argc, char*argv[])  
{
    int i=0, size;
	int offset, index, tag;
	int cache_hit, cache_miss;
    int cache_size=32768;
    int assoc=8;
    int block_size=32;
    RPL repl_policy=LRU;


	/*
    *  Read through command-line arguments for options.
    */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 's') 
                cache_size=atoi(argv[i+1]);
            
            if (argv[i][1] == 'b')
                block_size=atoi(argv[i+1]);
            
            if (argv[i][1] == 'a')
                assoc=atoi(argv[i+1]);
            
            if (argv[i][1] == 'f')
                strcpy(trace_file,argv[i+1]);


            if (argv[i][1] == 'r')
            {
                if(strcmp(argv[i+1],"lru")==0)
                    repl_policy=LRU;
                else if(strcmp(argv[i+1],"rand")==0)
                    repl_policy=RAND;
                else
                {
                    printf("unsupported replacement policy:%s\n",argv[i+1]);
                    return -1;
                }           
            }
        }
    }
	index = logB(cache_size / (block_size * assoc));
	offset = logB(block_size);
	tag = 32 - (index + offset);
    
	cache_hit = cache_miss = 0;

    /*
     * main body of cache simulator
    */

	//configure the cache with the cache parameters specified in the input arguments
    size = init_cache(cache_size, block_size, assoc, repl_policy); 

    while(1)
	{
        MEMACCESS new_access;
        
		//read new memory access from the memory trace file
        BOOL success=read_new_memaccess(&new_access);
        
        if(success!=TRUE)   //check the end of the trace file
            break;

		//check if the new memory access hit on the cache
        if(isHit(new_access.addr,index,tag,assoc,repl_policy) == FALSE)   
        {
			//if miss, insert a cache block for the memory access to the cache
            insert_to_cache(new_access.addr, index, tag, assoc, repl_policy);  
			cache_miss++;
        }
		else
			cache_hit++;
	}
    
    // print statistics here
	print_stat(cache_size, block_size, assoc, repl_policy, cache_hit, cache_miss);

	for (i = 0; i < size; i++)
		free(cache[i]);
	free(cache);

	if (LRU_counter != NULL) {
		for (i = 0; i < size; i++)
			free(LRU_counter[i]);
		free(LRU_counter);
	}

	return 0;
}

int logB(int x) {
	int i;
	int cnt = 0;

	while (x > 0) {
		x /= 2;
		cnt++;
	}

	return cnt;
}

int init_cache(int cache_size, int block_size, int assoc, RPL repl_policy) {
	int index;
	int i, j;

	// cache index
	index = cache_size / (block_size * assoc);

	cache = (MEMACCESS**)malloc(sizeof(MEMACCESS*)*index);
	for (i = 0; i < index; i++)
		cache[i] = (MEMACCESS*)malloc(sizeof(MEMACCESS)*assoc);

	// init cache
	for (i = 0; i < index; i++) {
		for (j = 0; j < assoc; j++) {
			cache[i][j].is_read = FALSE;
			cache[i][j].addr = 0;
		}
	}


	// LRU counters if repl_policy == LRU
	if (repl_policy == LRU) {
		LRU_counter = (int**)malloc(sizeof(int*)*index);
		for (i = 0; i < index; i++)
			LRU_counter[i] = (int*)malloc(sizeof(int)*assoc);

		for (i = 0; i < index; i++)
			for (j = 0; j < assoc; j++)
				LRU_counter[i][j] = assoc-(i+1);
	}

	return index;
}


/*
 * read a new memory access from the memory trace file
 */
BOOL read_new_memaccess(MEMACCESS* mem_access)
{
    ADDR access_addr;
    char access_type[10];
    /*
     * open the mem trace file
     */

    if(fp==NULL)
    {
        fp=fopen(trace_file,"r");
        if(fp==NULL)
        {
            fprintf(stderr,"error opening file");
            exit(2);

        }   
    }

    if(mem_access==NULL)
    {
        fprintf(stderr,"MEMACCESS pointer is null!");
        exit(2);
    }

    if(fscanf(fp,"%llx %s", &access_addr, &access_type)!=EOF)
    {
        mem_access->addr=access_addr;
        if(strcmp(access_type,"RD")==0)
            mem_access->is_read=TRUE;
        else
            mem_access->is_read=FALSE;
        
        return TRUE;
    }       
    else
        return FALSE;
}

BOOL isHit(ADDR addr, int offset, int index, int tag,int assoc){
	int i;
	int idx,tg; // index, tag

	idx= (addr << tag) >> (32-index);
	tg = addr >> (index + offset);

	for (i = 0; i < assoc; i++) {
		if ((cache[idx][i].addr >> (index + offset)) == tg) {
			if (cache[idx][i].is_read==0)
    			return FALSE;
			else
				return TRUE;
		}
	}
	
	return FALSE;
}

ADDR insert_to_cache(ADDR addr, int index, int tag, int assoc, RPL repl_policy) {
	int r, c; // cache row, column
	int check = FALSE;

	r = (addr << tag) >> (32-index);

	if (repl_policy == LRU) {
		for (c = assoc-1; c >= 0; c--) {
			if (!cache[r][c].is_read) {
				check = TRUE;
				break;
			}
		}

		if (!check) {
			for (c = assoc-1; c >= 0; c--)
				if (LRU_counter[r][c] == 0)
					break;

			cache[r][c].addr = addr;
			update_LRU(r, c, 0, assoc);
			LRU_counter[r][c] = assoc-1;
		}

		cache[r][c].is_read = TRUE;
		cache[r][c].addr = addr;
		update_LRU(r, c, LRU_counter[r][c], assoc);
		LRU_counter[r][c] = assoc-1;
	}
	else {
		for (c = 0; c < assoc; c++) {
			if (!cache[r][c].is_read) {
				check = TRUE;
				break;
			}
		}

		if (!check) {
			srand(time(NULL));
			c = rand() % assoc;
		}

		cache[r][c].addr = addr;
		cache[r][c].is_read = TRUE;
	}
}

void update_LRU(int r, int c, int prev, int assoc) {
	int i;

	for (i = 0; i < assoc; i++) {
		if (i == c)
			continue;
		
		if (LRU_counter[r][i] > prev)
			LRU_counter[r][i] -= 1;
	}
}

void print_stat(int cache, int block, int assoc, RPL repl_policy, int hit, int miss)
{
	printf("cache_size : %d B\n", cache);
	printf("block_size : %d B\n", block);
	printf("associativity : %d\n", assoc);
	
	if (repl_policy == LRU)
		printf("replacement policy : LRU\n");
	else
		printf("replacement policy : Random\n");

	printf("cache accesses : %d\n", hit+miss);
	printf("cache_hits : %d\n", hit);
	printf("cache_misses : %d\n", miss);
	printf("cache_miss_rate : %lf\n", (double)miss/(double)(miss+hit));
}
