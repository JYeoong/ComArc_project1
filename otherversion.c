#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct component{
    char *tag;
    char *index;
    char *offset;
} component;

//misc. function
FILE* fp = 0;
char trace_file[100]="memtrace.trc";
MEMACCESS** cache = NULL;
int** LRU_counter = NULL;
BOOL read_new_memaccess(MEMACCESS*);  //read new memory access from the memory trace file (already implemented)

//configure the cache
int init_cache(int cache_size, int block_size, int assoc, RPL repl_policy);

//check if the memory access hits on the cache
BOOL isHit(ADDR,int,int,int,int);

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR addr);


//print the simulation statistics
print_stat();

int cache_hit;
int cache_miss;

//additionally
component extractComp(ADDR,int,int,int);
unsigned int htoi(const char[]);
char *getBinary(unsigned int);
char *formatBinary(char*,int,int,int);
int btoi(char *);
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
    
    init_cache(cache_size, block_size, assoc, repl_policy);   //configure the cache with the cache parameters specified in the input arguments

    size = init_cache(cache_size, block_size, assoc, repl_policy); 
    printf("index: %d, offset: %d, tag: %d, size: %ld\n",index,offset,tag,size);

    while(1)
	{
        MEMACCESS new_access;
        
        BOOL success=read_new_memaccess(&new_access);  //read new memory access from the memory trace file
        /**contemporary**/
        if(success!=TRUE)   //check the end of the trace file
            break;
	if(isHit(new_access.addr,tag,index,offset,assoc)==TRUE)
	    cache_hit++; 
	else
	    cache_miss++;
	/***************/

    /*
        if(isHit(new_access.addr)==FALSE)   //check if the new memory access hit on the cache
        {
            insert_to_cache(new_access.addr);  //if miss, insert a cache block for the memory access to the cache
        }
    */

	}
    
    // print statistics here

	return 0;
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
            puts("hit");
        else
            puts("Miss");
        
        return TRUE;
    }       
    else
        return FALSE;

}
/******************** create cache *********************/
int init_cache(int cache_size, int block_size, int assoc, RPL repl_policy) {
	int index;
	long long i, j;

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


/******************** isHit *********************/
BOOL isHit(ADDR addr,int tag,int index,int offset,int assoc)
{
    component com,block;
    int i,idx;

    com=extractComp(addr,tag,index,offset);
    idx=btoi(com.index);

    for(i=0;i<assoc;i++){
	block=extractComp(cache[idx][i].addr,tag,index,offset);
        if(cache[idx][i].is_read == 1 && strcmp(block.tag,com.tag) == 0)
        {
            cache_hit++;
	    return TRUE;
        }
        else
        {        
            cache_miss++;
            
            return FALSE;
        }
    }
}

/******************** for get source *********************/
/*
	1)extractComp
	2)htoi
	3)getBinary
	4)formatBinary
	5)btoi
	6)logB
*/

component extractComp(ADDR addr,int TAG, int INDEX, int OFFSET)
{
    unsigned int dec;
    char *bstring, *bformatted, *tag, *index, *offset;
    int i;
    component com;
    
    dec = htoi(addr);
    bstring = getBinary(dec);
    bformatted = formatBinary(bstring,TAG,INDEX,OFFSET);
    
    tag = (char *) malloc( sizeof(char) * (TAG + 1) );
    tag[TAG] = '\0';
    
    for(i = 0; i < TAG; i++)
    {
        tag[i] = bformatted[i];
    }

    com.tag=tag;

    index = (char *) malloc( sizeof(char) * (INDEX + 1) );
    index[INDEX] = '\0';
    
    for(i = TAG + 1; i < INDEX + TAG + 1; i++)
    {
        index[i - TAG - 1] = bformatted[i];
    }
   
    com.index=index;
 
    offset = (char *) malloc( sizeof(char) * (OFFSET + 1) );
    offset[OFFSET] = '\0';
    
    for(i = INDEX + TAG + 2; i < OFFSET + INDEX + TAG + 2; i++)
    {
        offset[i - INDEX - TAG - 2] = bformatted[i];
    }

    com.offset=offset;

    printf("Tag: %s (%i)\n", tag, btoi(tag));
    printf("Index: %s (%i)\n", index, btoi(index));
    printf("Offset: %s (%i)\n", offset, btoi(offset));

    return com;
}


/* htoi
 *
 * Converts hexidecimal memory locations to unsigned integers.
 * No real error checking is performed. This function will skip
 * over any non recognized characters.
 */

unsigned int htoi(const char str[])
{
    /* Local Variables */
    unsigned int result;
    int i;

    i = 0;
    result = 0;
    
    if(str[i] == '0' && str[i+1] == 'x')
    {
        i = i + 2;
    }

    while(str[i] != '\0')
    {
        result = result * 16;
        if(str[i] >= '0' && str[i] <= '9')
        {
            result = result + (str[i] - '0');
        }
        else if(tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f')
        {
            result = result + (tolower(str[i]) - 'a') + 10;
        }
        i++;
    }

    return result;
}

/* getBinary
 *
 * Converts an unsigned integer into a string containing it's
 * 32 bit long binary representation.
 *
 *
 * @param   num         number to be converted
 *
 * @result  char*       binary string
 */
 
char *getBinary(unsigned int num)
{
    char* bstring;
    int i;
    
    /* Calculate the Binary String */
    
    bstring = (char*) malloc(sizeof(char) * 33);
    
    bstring[32] = '\0';
    
    for( i = 0; i < 32; i++ )
    {
        bstring[32 - 1 - i] = (num == ((1 << i) | num)) ? '1' : '0';
    }
    
    return bstring;
}


/* formatBinary
 *
 * Converts a 32 bit long binary string to a formatted version
 * for easier parsing. The format is determined by the TAG, INDEX,
 * and OFFSET variables.
 *
 * Ex. Format:
 *  -----------------------------------------------------
 * | Tag: 18 bits | Index: 12 bits | Byte Select: 4 bits |
 *  -----------------------------------------------------
 *
 * Ex. Result:
 * 000000000010001110 101111011111 00
 *
 * @param   bstring     binary string to be converted
 *
 * @result  char*       formated binary string
 */

char *formatBinary(char *bstring,int TAG, int INDEX, int OFFSET)
{
    char *formatted;
    int i;
    
    /* Format for Output */
    
    formatted = (char *) malloc(sizeof(char) * 35);
    
    formatted[34] = '\0';
    
    for(i = 0; i < TAG; i++)
    {
        formatted[i] = bstring[i];
    }
    
    formatted[TAG] = ' ';
    
    for(i = TAG + 1; i < INDEX + TAG + 1; i++)
    {
        formatted[i] = bstring[i - 1];
    }
    
    formatted[INDEX + TAG + 1] = ' ';
    
    for(i = INDEX + TAG + 2; i < OFFSET + INDEX + TAG + 2; i++)
    {
        formatted[i] = bstring[i - 2];
    }

    return formatted;
}


/* btoi
 *
 * Converts a binary string to an integer. Returns 0 on error.
 *
 * src: http://www.daniweb.com/software-development/c/code/216372
 *
 * @param   bin     binary string to convert
 *
 * @result  int     decimal representation of binary string
 */

int btoi(char *bin)
{
    int  b, k, m, n;
    int  len, sum;

    sum = 0;
    len = strlen(bin) - 1;

    for(k = 0; k <= len; k++)
    {
        n = (bin[k] - '0'); 
        if ((n > 1) || (n < 0))
        {
            return 0;
        }
        for(b = 1, m = len; m > k; m--)
        {
            b *= 2;
        }
        sum = sum + n * b;
    }
    return(sum);
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

