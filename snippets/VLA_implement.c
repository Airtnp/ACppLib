void alloca_diff( void )
{
    char *alloca_c, *vla_c;
    for (int i=1;i<10;++i)
    {
        char *alloca_mem = alloca(i*sizeof(*alloca_mem));
        alloca_c = alloca_mem;//valid
        char vla_arr[i];
        vla_c = vla_arr;//invalid
    }//end of scope, VLA memory is freed
    printf("alloca: %c\n", *alloca_c); //fine
    
    printf("vla: %c\n", *vla_c); //undefined behaviour... avoid!
} //end of function alloca memory is freed, irrespective of block scope

// probably alloca implementation
// but compiler surely ignores that esp operation
#define __alloca(p, N) \
    do { \
        __asm__ __volatile__( \
        "sub %1, %%esp \n" \
        "mov %%esp, %0  \n" \
         : "=m"(p) \
         : "i"(N) \
         : "esp"); \
    } while(0)