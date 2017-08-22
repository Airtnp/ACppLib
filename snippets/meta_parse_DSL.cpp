// ref: https://stackoverflow.com/questions/17783393/how-to-parse-text-for-a-dsl-at-compile-time

#include <iostream>
#include <vector>

#include <boost/metaparse/repeated.hpp>
#include <boost/metaparse/sequence.hpp>
#include <boost/metaparse/lit_c.hpp>
#include <boost/metaparse/last_of.hpp>
#include <boost/metaparse/middle_of.hpp>
#include <boost/metaparse/space.hpp>
#include <boost/metaparse/foldl_start_with_parser.hpp>
#include <boost/metaparse/one_of.hpp>
#include <boost/metaparse/token.hpp>
#include <boost/metaparse/entire_input.hpp>
#include <boost/metaparse/string.hpp>
#include <boost/metaparse/transform.hpp>
#include <boost/metaparse/always.hpp>
#include <boost/metaparse/build_parser.hpp>
#include <boost/metaparse/keyword.hpp>

#include <boost/mpl/apply_wrap.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/back.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/proto/proto.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/make_vector.hpp>

using boost::metaparse::sequence;
using boost::metaparse::lit_c;
using boost::metaparse::last_of;
using boost::metaparse::middle_of;
using boost::metaparse::space;
using boost::metaparse::repeated;
using boost::metaparse::build_parser;
using boost::metaparse::foldl_start_with_parser;
using boost::metaparse::one_of;
using boost::metaparse::token;
using boost::metaparse::entire_input;
using boost::metaparse::transform;
using boost::metaparse::always;
using boost::metaparse::keyword;

using boost::mpl::apply_wrap1;
using boost::mpl::front;
using boost::mpl::back;
using boost::mpl::bool_;


struct build_or
{
    typedef build_or type;

    template <class C, class State>
    struct apply
    {
        typedef apply type;
        typedef typename boost::proto::logical_or<typename State::proto_type, typename C::proto_type >::type proto_type;
    };
};

struct build_and
{
    typedef build_and type;

    template <class C, class State>
    struct apply
    {
        typedef apply type;
        typedef typename boost::proto::logical_and<typename State::proto_type, typename C::proto_type >::type proto_type;
    };
};



template<bool I>
struct value //helper struct that will be used during the evaluation in the proto context
{};

struct build_value
{
    typedef build_value type;

    template <class V>
    struct apply
    {
        typedef apply type;
        typedef typename boost::proto::terminal<value<V::type::value> >::type proto_type;
    };
};

struct build_not
{
    typedef build_not type;

    template <class V>
    struct apply
    {
        typedef apply type;
        typedef typename boost::proto::logical_not<typename V::proto_type >::type proto_type;
    };
};

template<int I>
struct placeholder //helper struct that will be used during the evaluation in the proto context
{};

template<int I>
struct arg
{
    typedef arg type;
    typedef typename boost::proto::terminal<placeholder<I> >::type proto_type;
};

#ifdef _S
#error _S already defined
#endif
#define _S BOOST_METAPARSE_STRING

typedef token < keyword < _S ( "&&" ) > > and_token;
typedef token < keyword < _S ( "||" ) > > or_token;
typedef token < lit_c < '!' > > not_token;

typedef token < keyword < _S ( "true" ), bool_<true> > > true_token;
typedef token < keyword < _S ( "false" ), bool_<false> > > false_token;

typedef token < lit_c < 'a' > > arg1_token;
typedef token < lit_c < 'b' > > arg2_token;
typedef token < lit_c < 'c' > > arg3_token;


struct paren_exp;

typedef
one_of< paren_exp, transform<true_token, build_value>, transform<false_token, build_value>, always<arg1_token, arg<0> >, always<arg2_token, arg<1> >, always<arg3_token, arg<2> > >
value_exp; //value_exp = paren_exp | true_token | false_token | arg1_token | arg2_token | arg3_token;

typedef
one_of< transform<last_of<not_token, value_exp>, build_not>, value_exp>
not_exp; //not_exp = (omit[not_token] >> value_exp) | value_exp;

typedef
foldl_start_with_parser <
last_of<and_token, not_exp>,
         not_exp,
         build_and
         >
         and_exp; // and_exp = not_exp >> *(and_token >> not_exp);

typedef
foldl_start_with_parser <
last_of<or_token, and_exp>,
         and_exp,
         build_or
         >
         or_exp; // or_exp = and_exp >> *(or_token >> and_exp);

struct paren_exp: middle_of < lit_c < '(' > , or_exp, lit_c < ')' > > {}; //paren_exp = lit('(') >> or_exp >> lit('(');

typedef last_of<repeated<space>, or_exp> expression; //expression = omit[*space] >> or_exp;

typedef build_parser<entire_input<expression> > function_parser;


template <typename Args>
struct calculator_context
        : boost::proto::callable_context< calculator_context<Args> const >
{
    calculator_context ( const Args& args ) : args_ ( args ) {}
    // Values to replace the placeholders
    const Args& args_;

    // Define the result type of the calculator.
    // (This makes the calculator_context "callable".)
    typedef bool result_type;

    // Handle the placeholders:
    template<int I>
    bool operator() ( boost::proto::tag::terminal, placeholder<I> ) const
    {
        return boost::fusion::at_c<I> ( args_ );
    }

    template<bool I>
    bool operator() ( boost::proto::tag::terminal, value<I> ) const
    {
        return I;
    }
};

template <typename Args>
calculator_context<Args> make_context ( const Args& args )
{
    return calculator_context<Args> ( args );
}

template <typename Expr, typename ... Args>
int evaluate ( const Expr& expr, const Args& ... args )
{
    return boost::proto::eval ( expr, make_context ( boost::fusion::make_vector ( args... ) ) );
}

#ifdef LAMBDA
#error LAMBDA already defined
#endif
#define LAMBDA(exp) apply_wrap1<function_parser, _S(exp)>::type::proto_type{}

int main()
{
    using std::cout;
    using std::endl;

    cout << evaluate ( LAMBDA ( "true&&false" ) ) << endl;
    cout << evaluate ( LAMBDA ( "true&&a" ), false ) << endl;
    cout << evaluate ( LAMBDA ( "true&&a" ), true ) << endl;
    cout << evaluate ( LAMBDA ( "a&&b" ), true, false ) << endl;
    cout << evaluate ( LAMBDA ( "a&&(b||c)" ), true, false, true ) << endl;
    cout << evaluate ( LAMBDA ( "!a&&(false||(b&&!c||false))" ), false, true, false ) << endl;
}

/*int main(int argc , char** argv)
{
    using std::cout;
    using std::endl;

    bool a=false, b=false, c=false;

    if(argc==4)
    {
        a=(argv[1][0]=='1');
        b=(argv[2][0]=='1');
        c=(argv[3][0]=='1');
    }

    LAMBDA("a && b || c") expr;

    cout << evaluate(expr, true, true, false) << endl;
    cout << evaluate(expr, a, b, c) << endl;

    return 0;
}*/