#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<stdint.h>
#include<limits.h>
#include<string.h>


/**
 * Struct for Saturating Counter on determining staken or not taken
 * 
 * member[counter] unsigned number to represent the state
 *  if MSB is 1, predict taken, otherwise predict not taken
 **/
typedef struct
{
    uint8_t counter;
}saturating_counter;

/**
 * Pattern History Table contains entries of saturating counter values 
 * 
 * member[counters] array of counters representing if taken where the index is 
 **/
typedef struct
{
    saturating_counter* counters;
}PHT;

/**
 * GAp predictor containing necessary components and bit sizes
 * member[GHR] Global History Register m bits size
 * member[phts] array of 2^m pattern history tables
 * member[m] m-bit sized GHR 
 * member[n] n-bit sized saturating counter => size of PHT entry
 * member[PC_bits] Number of lower bits used in PC (excluding last 2)
 **/
typedef struct
{
    unsigned GHR;
    PHT*     phts;
    unsigned m;
    unsigned n;
    unsigned PC_bits;
}GAp_predictor;

/**
 * simple struct to keep track of branch statistics
 *
 * member[reads] total number of entries
 * member[mispredictions] number of times predictor is wrong
 * member[rate] the misprediction rate of branch predictor
 **/
typedef struct
{
    ssize_t reads;
    ssize_t mispredictions;
    float misprediction_rate;
}branch_stats;

char* concat(const char*,const char*);
void print_results(branch_stats*);
void increment_entries(branch_stats*);
branch_stats* init_branch_stats(void);
GAp_predictor* init_GAp_predictor(unsigned,unsigned,unsigned);
void analyse_trace(GAp_predictor*,branch_stats,char*);
unsigned extract_PC_bits(unsigned,uint32_t);
/**
 * Concatenates 2 string into 1
 *
 * Takes string "a" and appends string "b" by dynamically allocating memory for combined string
 * 
 * param[a] first part of string
 * param[b] second part of string
 * return string a + string b
 **/
char*
concat(const char *a, const char *b)
{
    char *string = malloc(strlen(a) + strlen(b) + 1);
    strcpy(string, a);
    strcat(string, b);
    return string;
}

branch_stats*
init_branch_stats(void)
{
    branch_stats* stats = malloc(sizeof(branch_stats));
    stats->mispredictions = 0;
    stats->misprediction_rate = 0;
    stats->reads = 0;
    return stats;
}
void
increment_entries(branch_stats* stats)
{
    ++stats->reads;
}

void
print_results(branch_stats* stats)
{
    printf("Read %zu entries\nMisprediction rate: %.2f%%\n",stats->reads,stats->misprediction_rate);
}
/**
 * Takes the Program Counter Address and extracts the bits that would
 *  be used to choose which PHT 
 *
 *  param[n] the number of LSB of the PC, excluding bits 1:0
 *  param[PC] The address of the current instruction (32-bit address)
 *
 *  return: the n-bits of the PC that will be used
 **/
unsigned
extract_PC_bits(unsigned n,uint32_t PC)
{
    unsigned extracted_bits = 0;

    return extracted_bits;
}

/**
 * Initializes a GAp predictor with necessary bits of the struct
 *
 * param[counter_size] bits in the saturating counter, 
 *  which is also used to calculate number of PHT entries
 * param[GHR_size] bit length of GHR, which is also used 
 *  to calculate number of PHTs
 * param[PC_bits] which bits of the PC are used to index which PHT
 *
 * return: an initialized GAp predictor object
 **/
GAp_predictor*
init_GAp_predictor(unsigned counter_size, unsigned GHR_size, unsigned PC_bits)
{
    GAp_predictor* predictor = malloc(sizeof(GAp_predictor));
    predictor->m = GHR_size;
    predictor->n = counter_size;
    predictor->GHR = 0;
    
    // number of PHT is 2^m, where m is size of GHR entry
    unsigned num_PHT = 1 << GHR_size;
    // number of entries is 2^p, where p is size of saturating counter
    unsigned num_PHT_entry = 1 << counter_size;

    // Create PHT and for each PHT create appropiate number of entries
    predictor->phts = malloc(sizeof(saturating_counter)*num_PHT);
    for(ssize_t i = 0;i < num_PHT;++i)
    {
        predictor->phts[i].counters = malloc(sizeof(saturating_counter) * num_PHT_entry);
        // initialize all saturating counters to 00 (not taken)
        for(ssize_t j =0; i < num_PHT_entry;++j)
            predictor->phts[i].counters[j].counter = 0;
    }
    return predictor;
}
/**
 * Analyze the branch operation and analyze PC addresses of the 
 *  line from the trace file. FOR BASE this only will act on BCN
 *  operations
 *
 * param[stats] branch_stats object to track branch entries and misprediction
 * param[predictor] GAp_predictor object that predictor is based off of
 * param[line] line from trace file containing branch_op, PC, PC_NEXT
 **/
void
analyze_trace(branch_stats* stats,GAp_predictor* predictor,
        char* line)
{
    /*
     * FROM TRACE
     * branch operation: BUC, BCN, CAL, RET
     *  For base just look at BCN
     * Current Program Counter
     * NEXT Instruction Program Counter
     */
    ssize_t entries;
    char* branch_op = strsep(&line," ");
    uint32_t PC = atoi(strsep(&line," "));
    uint32_t PC_next = atoi(strsep(&line," "));

    if(strcmp("BCN",branch_op)==0)
        increment_entries(stats);


}

/* CLI INPUTS
 * benchmark: name of trace file
 * m: size of GHR in bytes
 * p: portion of PC that is used to index PHT excluding lower 2 bits
 * n: size of each saturating counter
 */
int
main(int argc,char *argv[])
{
    if(argc < 4)
    {
        fprintf(stderr,"Invalid Arguments!\n");
        return -1;
    }
    

    char* trace_file = concat(argv[1],".trace");

    //Reading in CLI data
    ssize_t GHR_bytes = atoi(argv[2]); 
    ssize_t PCportion_bytes = atoi(argv[3]);
    ssize_t saturatingCounter_bytes = atoi(argv[4]);

    printf("Opened file %s for reading\n",trace_file);
    printf("m=%zup=%zu\tn=%zu\n",GHR_bytes,PCportion_bytes,saturatingCounter_bytes);
    
    branch_stats* stats = init_branch_stats();
    GAp_predictor* predictor = init_GAp_predictor(saturatingCounter_bytes,
            GHR_bytes,PCportion_bytes);
    
    //setting up reading trace file
    char* line = NULL;
    size_t n =  0;
    FILE *fp_trace = fopen(trace_file,"r");
    if(!fp_trace)
    {
        fprintf(stderr,"Invalid Trace file!\n");
        return -1;
    }

    ssize_t read = getline(&line,&n,fp_trace);
    // read and analyze the trace file until the end
    do
    {
        char* string = line;
        analyze_trace(stats,predictor,string);
        free(line);
        line = NULL;
        n = 0;
    }while((read = getline(&line,&n,fp_trace)) != -1);
    print_results(stats);

    free(line);
    free(trace_file);
    fclose(fp_trace);

    return 0;
}

