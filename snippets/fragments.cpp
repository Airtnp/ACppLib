
#include <iostream>
#include <new>

void report_max_heap_alloc()
{
   const size_t tolerance = 0x00100000UL;
   const size_t biggest_alloc = 0x7FFFFFFFUL;
   
   char *address;          // Allocated address.
   size_t pass = 0;        // Largest succeeded size.
   size_t fail = 0;        // Smallest failed size.
   size_t max_alloc;       // Maximum allocated so far.
   
   while (true) {
      if (fail == 0) {
         max_alloc = biggest_alloc;
         fail = biggest_alloc;
      } else {
         max_alloc = (fail / 2) + (pass / 2);
      }
      address = new (std::nothrow) char[max_alloc];
      if (address == 0) {
         fail = max_alloc;
      } else {
         pass = max_alloc;
         delete [] address;
      }
      if (fail - pass < tolerance) break;
   }
   
   std::cout << "Maximum heap allocation: " << max_alloc / 1000000
         << " MB" << std::endl;
} // report_max_heap_alloc


class A
{
public:
   A(void) : component(NULL) { };
   
   ~A(void) {
      delete [] component;
   }
   
   bool allocate(int element_count) {
      component = new(std::nothrow) int[element_count];
      return component != NULL;
   };
   
   void deallocate() {
      delete [] component;
      component = NULL;
   };
   
   int *component;
};


int main()
{
   const int num_allocations = 2000000;
   // const int num_allocations = 500;
   const int element_count = 150;
   //const int element_count = 750000;
   const int num_rounds = 5;
   
   for (int round = 0; round < num_rounds; ++ round) {
      std::cout << "Round " << round << std::endl;
      A *arr = new A[num_allocations];
      
      std::cout << "Before allocations" << std::endl;
      report_max_heap_alloc();
      
      for(int i = 0; i < num_allocations; ++ i) {
         if (!arr[i].allocate(element_count)) {
            std::cout << ">>>>> Failed allocation " << i << std::endl;
            break;
         }
      }
      std::cout << "After allocations" << std::endl;
      report_max_heap_alloc();
      
      for(int i=0; i < num_allocations; ++ i) {
         arr[i].deallocate();
      }
      std::cout << "After deallocations" << std::endl;
      report_max_heap_alloc();
      
      delete [] arr;
   }
   return 0;
}
