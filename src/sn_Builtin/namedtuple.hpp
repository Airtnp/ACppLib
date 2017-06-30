#ifndef SN_BUILTIN_NAMED_TUPLE_H
#define SN_BUILTIN_NAMED_TUPLE_H

#include "../sn_CommonHeader.h"

namespace sn_Builtin {
    // ref: http://vitiy.info/named-tuple-for-cplusplus/
    namespace namedtuple {
        namespace detail {
            using hash_type = std::uint64_t;
                
            constexpr hash_type basis = 14695981039346656037ull;
            constexpr hash_type prime = 109951162821ull;
            
            // FNV-1a 64 bit hash
            constexpr hash_type sid_hash(const char *str, hash_type hash = basis) noexcept {
                return *str ? sid_hash(str + 1, (hash ^ *str) * prime) : hash;
            }
        }

        /// Named parameter (could be empty!)
        template <typename Hash, typename ...Ts>
        struct named_param : public std::tuple<std::decay_t<Ts>...> {
            using hash = Hash;                                                              
            named_param(Ts&&... ts) 
                : std::tuple<std::decay_t<Ts>...>(std::forward<Ts>(ts)...) {}          
            template <typename P>
            named_param<Hash, P> operator=(P&& p) {
                return named_param<Hash, P>(std::forward<P>(p)); 
            }            
        };
    
        template <typename Hash>
        using make_named_param = named_param<Hash>;
        
        /// Named tuple is just tuple of named params
        template <typename... Params>
        struct named_tuple : public std::tuple<Params...> {
            
            template <typename ...Args>
            named_tuple(Args&&... args) 
                : std::tuple<Args...>(std::forward<Args>(args)...) {}
            
            static const std::size_t error = -1;
            
            template<std::size_t I = 0, typename Hash>
            constexpr typename std::enable_if<I == sizeof...(Params), const std::size_t>::type
            static get_element_index() {
                return error;
            }
            
            template<std::size_t I = 0, typename Hash>
            constexpr typename std::enable_if<I < sizeof...(Params), const std::size_t>::type
            static get_element_index() {
                using elementType = typename std::tuple_element<I, std::tuple<Params...>>::type;
                // RTTI method (may use type_id_info)
                // return (typeid(typename elementType::hash) == typeid(Hash)) ? I : get_element_index<I + 1, Hash>();
                return (std::is_same<typename elementType::hash, Hash>::value) ? I : get_element_index<I + 1, Hash>();
            }
            
            template<typename Hash>
            const auto& get() const {
                constexpr std::size_t index = get_element_index<0, Hash>();
                static_assert((index != error), "Wrong named tuple key");
                auto& param = (std::get<index>(static_cast<const std::tuple<Params...>&>(*this)));
                return std::get<0>( param );
            }
            
            template<typename NP>
            const auto& operator[](NP&& param) {
                return get<typename NP::hash>();
            }
             
        };
        
        template <typename ...Args>
        auto make_named_tuple(Args&&... args) {
            return named_tuple<Args...>(std::forward<Args>(args)...);
        }
        
        #define SN_MAKE_NAMED_TUPLE_PARAM(x) \
            sn_Builtin::namedtuple::make_named_param< \
                std::integral_constant< \
                    sn_Builtin::namedtuple::detail::hash_type, \
                    sn_Builtin::namedtuple::detail::sid_hash(x) \
                > \
            >{} \

        /*
            Usage:
                auto student0 = make_named_tuple(param("GPA") = 3.8, param("grade") = 'A', param("name") = "Lisa Simpson");

                auto gpa = student0[param("GPA")];
                auto grade = student0[param("grade")];
        */
    }
}

#endif