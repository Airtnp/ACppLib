// map.emplace will invoke one ctor and one move/copy
// methods to implement inplace construction

struct value {
  struct tag { };
  value(tag){ .... };
};

std::map<int, value> xmap;
xmap.emplace(123, value::tag());

// Cpp1z

std::emplace(
    std::piecewise_construct, 
    std::forward_as_tuple(key), 
    std::forward_as_tuple()
);

xmap.try_emplace(std::move(key));
