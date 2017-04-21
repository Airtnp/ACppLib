#include <iostream>

template <typename V1, typename V2, typename Enable = void>
struct join_impl
{
  enum { forward = true };
};

/// T without reference or const/volatile qualifiers.
template <typename T>
using base_t
  = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template <typename V1, typename V2>
typename join_impl<base_t<V1>, base_t<V2>>::type
join(V1 v1, V2 v2)
{
  return join_impl<base_t<V1>, base_t<V2>>::join(v1, v2);
}

template <typename T>
struct join_impl<T, T>
{
  enum { forward = false };
  using type = T;
  static type join(type v1, type v2)   { return v1 * v2; }
};

template <typename T1, typename T2>
struct join_impl<T1, T2, typename std::enable_if<join_impl<T2, T1>::forward>::type>
{
  enum { forward = false };
  using super = join_impl<T2, T1>;
  using type = typename super::type;

  static
  type
  join(T1 v1, T2 v2)
  {
    return super::join(v2, v1);
  }
};


int main()
{
  int i = 20;
  float f = 0.1;
#define ECHO(S) std::cerr << #S": " << S << '\n'
  //  ECHO(::join(i, i));
  ECHO(::join(i, f));
  ECHO(::join(f, i));
  ECHO(::join(f, f));
}