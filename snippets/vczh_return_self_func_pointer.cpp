#include <cassert>
#include <cxxabi.h>
#include <iostream>

class C { 
    public: 	
    struct Nest {
        Nest(C::*ptr)();  //declare a member function pointer
    }; 	
    Nest GetFunc() {
        return{ &C::GetFunc };
    } 
}; 

int main() {
    C shit; 
    auto pointer = (shit.*(shit.GetFunc().ptr))().ptr; 
    assert(pointer == &C::GetFunc); 
    std::cout << abi::__cxa_demangle(typeid(decltype(shit.GetFunc())).name(), nullptr, nullptr, nullptr);
}