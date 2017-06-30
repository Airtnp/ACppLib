作者：d41d8c
链接：https://www.zhihu.com/question/61745269/answer/191256664
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

void foo(int a, int b, int c = -1) {
    std::cout << "foo(" << a << ", " << b << ", " << c << ")\n";
}

int main() {
    foo(1, 2);   // output: foo(1, 2, -1)

    // error: does not use default from surrounding scope
    //void foo(int a, int b = 0, int c);

    void foo(int a, int b, int c = 30);
    foo(1, 2);   // output: foo(1, 2, 30) 

    // error: we cannot redefine the argument in the same scope
    // void foo(int a, int b, int c = 35);

    // has a default argument for c from a previous declaration
    void foo(int a, int b = 20, int c);
    foo(1);      // output: foo(1, 20, 30)

    void foo(int a = 10, int b, int c);
    foo();       // output: foo(10, 20, 30)

    {
        // in inner scopes we can completely redefine them
        void foo(int a, int b = 4, int c = 8);
        foo(2);  // output: foo(2, 4, 8)
    }

    return 0;
}