#include <variant>
#include <tuple>
#include <utility>
#include <iostream>

template <class T>
class refw
{
public:
    // types
    typedef T type;
private:
    type* f;

public:
    // construct/copy/destroy
    refw(type& f_) noexcept
        : f(std::addressof(f_)) {}
    
    refw(type&&) = delete;

    // access
    operator type&    () const noexcept {return *f;}
    type& get() const noexcept {return *f;}
};


template<typename T>
struct Tree : std::variant<std::monostate, std::tuple<refw<Tree<T>>,
                refw<Tree<T>>, T>> 
{
    using Base = std::variant<std::monostate, std::tuple<refw<Tree<T>>,
                refw<Tree<T>>, T>>;
    using Base::Base;
};

template<typename T>
using Link = refw<Tree<T>>;

template<typename T>
using Node = std::tuple<Link<T>, Link<T>, T>;

template<typename T>
struct TreeVisitor
{
    void operator()(const Node<T>& node) const
    {
        auto [left, right, v] = node;
        std::visit(*this, left.get());
        std::visit(*this, right.get());
        std::cout << v;
    }
    
    void operator()(const std::monostate&) const {}
};

int main()
{
    auto EmptyNode = Tree<int>{std::monostate{}};
    auto left = Tree<int>{Node<int>{refw(EmptyNode), refw(EmptyNode), 2}};
    Tree<int> root = Node<int>{refw(left),
                           refw(left),
                           233};
    std::visit(TreeVisitor<int>{}, root);
}