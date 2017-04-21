template <typename F, typename FF>
void demangle_type_rec(F out, FF prepend_suffix)
{
//    int class_type = // uncomment to get infinite stream of error messages
    demangle_type([&] {});
}

template <typename Out>
void demangle_type(Out out)
{
    demangle_type_rec(out, [&] {});
}

void print_seqid()
{
    demangle_type([&] {});
}
