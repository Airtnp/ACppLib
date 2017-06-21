#ifndef SN_TYPETRAITS_TYPE_QUALIFIER
#define SN_TYPETRAITS_TYPE_QUALIFIER


#include "../sn_CommonHeader.h"

namespace sn_TypeTraits {
    enum class type_qualifier {
		value,
		const_value,
		volatile_value,
		volatile_const_value,
		lref,
		rref,
		const_lref,
		const_rref,
		volatile_lref,
		volatile_rref,
		volatile_const_lref,
		volatile_const_rref,

		count_
	};

	template< type_qualifier type_qual, typename type > struct add_type_qualifier;
	template< typename to > struct add_type_qualifier< type_qualifier::value                , to > { using type =          to         ; };
	template< typename to > struct add_type_qualifier< type_qualifier::const_value          , to > { using type =          to const   ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_value       , to > { using type = volatile to         ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_value , to > { using type = volatile to const   ; };
	template< typename to > struct add_type_qualifier< type_qualifier::lref                 , to > { using type =          to       & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::rref                 , to > { using type =          to       &&; };
	template< typename to > struct add_type_qualifier< type_qualifier::const_lref           , to > { using type =          to const & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::const_rref           , to > { using type =          to const &&; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_lref        , to > { using type = volatile to       & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_rref        , to > { using type = volatile to       &&; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_lref  , to > { using type = volatile to const & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_rref  , to > { using type = volatile to const &&; };

	template< type_qualifier type_qual, typename to >
	using add_type_qualifier_t = typename add_type_qualifier< type_qual, to >::type;

	template< typename from > constexpr type_qualifier type_qualifier_of                           = type_qualifier::value                ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from const    > = type_qualifier::const_value          ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from          > = type_qualifier::volatile_value       ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const    > = type_qualifier::volatile_const_value ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from       &  > = type_qualifier::lref                 ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from       && > = type_qualifier::rref                 ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from const &  > = type_qualifier::const_lref           ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from const && > = type_qualifier::const_rref           ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from       &  > = type_qualifier::volatile_lref        ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from       && > = type_qualifier::volatile_rref        ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const &  > = type_qualifier::volatile_const_lref  ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const && > = type_qualifier::volatile_const_rref  ;

	template< typename from, typename to >
	using copy_cv_reference_t = add_type_qualifier_t< type_qualifier_of< from >, to >;

}

#endif