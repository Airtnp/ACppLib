#include <cstdio>
#include <iostream>

template <typename T>
struct GetTypeNameHelper
{
    static const char* GetTypeName(void)
    {
        static const size_t size = sizeof(__FUNCTION__) - sizeof("GetTypeName ");
        static char typeName[size] = {};
        return typeName;
    }
};
static void test_case__freeze()
{
    std::cout << GetTypeNameHelper<int>::GetTypeName() << "\n";
}