# Expression Templates Everywhere

[video](https://www.youtube.com/watch?v=VhIwDxkIsME&index=6&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Primer
* expression templates
* + templates used to represent expressions
* + capture c++ expression as an expression tree
* + optimization
* + expressiveness
* Eigen
* + `Vectorxf a, b, c; a = 3*b+4*c+5*d;` many temporaries
* + change to read once no temporaries
* + - `a[i] = 3*b[i]+4*c[i]+5*d[i]` (generate SIMD)
* auto-differentiation
* + `VNode* x = create_var_node();... OpNode* op = create_binary_op_node(OP_MINUS, ...);`
* + hard to understand
* + use user-defined literal
* experssion are captured by objects, transformed, evaluated as objects
* + `auto expr = 1_p + 3;`
* + - type is `Expr<Plus<TypeList<Expr<Terminal, Tuple<Placeholder<1>>>, Expr<Terminal, Tuple<int>>>>`
* + transform
* + evaluate
* + create by function overloading (overloading opreators)
* + - `expr(1) + 2 + 3 + ... `

## Implementation
* Hand-rolled implementation
* boost.proto
* give up
* yap
* + expression templates easy to use
* + mimic the behavior of built-in expressions
* + never require implicit operations/evaluations
* + provide the same functionality as boost.proto, requiring less user code

## Yap
* structure
* expression concept
* + `expr_kind`
* + refeence `expr_ref`
* + terminals `leaf_nodes`
* + overloaded operators
* + ternary-operator analogue (`if_else`)
* + call operator
* + `E::kind` kind of expression
* + `e.elements` child expression of e (tuple with elements)
* ExpressionTemplate
* `evaluate(Expr&& expr, T&&... ts)`
* + must traverse the entire expression tree/every node
* + customization points almost exclusively via ADL
* + entirely implicit and pre-determined at the point of call
* `transform(Expr&& expr, Trans&& trans)`
* + must traverse all or part of the expression tree
* + may return the original tree, a different expression tree, or a value that is not an expression tree at all
* + explicit and may be modified at the point of call
* arity checker
* use const ref& to contain all exprs, std::move to all constants