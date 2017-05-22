#ifndef SN_TYPE_H
#define SN_TYPE_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Log.hpp"
#include "sn_TypeLisp.hpp"
#include "sn_TypeTraits.hpp"
#include "sn_Builtin.hpp"

//TODO: add concepts required by C++1z
namespace sn_Type {

	namespace byte_t {
		enum class byte : unsigned char {};

		template <class T, class = std::enable_if_t<std::is_integral<T>::value>>
		inline constexpr byte& operator<<=(byte& b, T shift) noexcept
		{
			return b = byte(static_cast<unsigned char>(b) << shift);
		}

		template <class T, class = std::enable_if_t<std::is_integral<T>::value>>
		inline constexpr byte operator<<(byte b, T shift) noexcept
		{
			return byte(static_cast<unsigned char>(b) << shift);
		}

		template <class T, class = std::enable_if_t<std::is_integral<T>::value>>
		inline constexpr byte& operator>>=(byte& b, T shift) noexcept
		{
			return b = byte(static_cast<unsigned char>(b) >> shift);
		}

		template <class T, class = std::enable_if_t<std::is_integral<T>::value>>
		inline constexpr byte operator >> (byte b, T shift) noexcept
		{
			return byte(static_cast<unsigned char>(b) >> shift);
		}

		inline constexpr byte& operator|=(byte& l, byte r) noexcept
		{
			return l = byte(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
		}

		inline constexpr byte operator|(byte l, byte r) noexcept
		{
			return byte(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
		}

		inline constexpr byte& operator&=(byte& l, byte r) noexcept
		{
			return l = byte(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
		}

		inline constexpr byte operator&(byte l, byte r) noexcept
		{
			return byte(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
		}

		inline constexpr byte& operator^=(byte& l, byte r) noexcept
		{
			return l = byte(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
		}

		inline constexpr byte operator^(byte l, byte r) noexcept
		{
			return byte(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
		}

		inline constexpr byte operator~(byte b) noexcept { return byte(~static_cast<unsigned char>(b)); }

	}
	namespace nil {
		class Nil {
			template <typename T>
			operator T() const noexcept {
				return T{};
			}
		};
	}

	// ref: qicosmos/cosmos
	// there should be a move-only any (unique_any/shared_any)
	// TODO: YSlib/base/ystdex/any ref: https://github.com/FrankHB/YSLib/blob/master/YBase/include/ystdex/any.h
	namespace any {
		class Any {
		public:
			Any() : m_typeIndex(std::type_index(typeid(void))) {}
			Any(const Any& rhs) : m_typeIndex(rhs.m_typeIndex), m_ptr(rhs.clone()) {}
			Any(Any&& rhs) : m_ptr(std::move(rhs.m_ptr)), m_typeIndex(std::move(rhs.m_typeIndex)) {}

			Any& operator=(const Any& rhs) {
				if (m_ptr == rhs.m_ptr)
					return *this;
				m_ptr = rhs.clone();
				m_typeIndex = rhs.m_typeIndex;
				return *this;
			}

			Any& operator=(Any&& rhs) {
				m_ptr = std::move(rhs.m_ptr);
				m_typeIndex = std::move(rhs.m_typeIndex);
				return *this;
			}

			template <typename U, 
				typename = std::enable_if_t<
				!std::is_same<std::decay_t<U>, Any>::value,
				U>>
			Any(U&& value) : 
				m_ptr(new Derived <std::decay_t<U>>(std::forward<U>(value))),
				m_typeIndex(std::type_index(typeid(std::decay_t<U>))) {}

			template <typename U, typename ...Args>
			Any(Args&&... args) :
				m_ptr(new Derived <std::decay_t<U>>(std::forward<U>(U{ std::forward<Args>(args)... }))),
				m_typeIndex(std::type_index(typeid(std::decay_t<U>))) {}

			bool is_null() const {
				return m_ptr == nullptr;
			}

			//TODO: change to is_convertible?
			template<typename U>
			bool is() const {
				return m_typeIndex == std::type_index(typeid(U));
			}

			template <typename U>
			U& any_cast() {
				if (!is<U>()) {
					SN_LOG_ERROR << "can not cast " << typeid(U).name() << " to " << m_typeIndex.name();
					throw std::logic_error{ "bad cast" };
				}

				auto derived = dynamic_cast<Derived<U>*>(m_ptr.get());
				return derived->m_value;
			}

		private: 
			
			struct Base {
				virtual ~Base() {}
				virtual std::unique_ptr<Base> clone() const = 0;
			};

			using base_ptr = std::unique_ptr<Base>;

			template <typename T>
			struct Derived : Base {
				template <typename U>
				Derived(U&& value) : m_value(std::forward<U>(value)) {}

				base_ptr clone() const {
					return base_ptr(new Derived<T>(m_value));
				}

				T m_value;
			};

			base_ptr clone() const {
				if (m_ptr != nullptr)
					return m_ptr->clone();
				return nullptr;
			}

			base_ptr m_ptr;
			std::type_index m_typeIndex;
		};
	}

	namespace optional {
		template <typename T>
		class Optional {
			using data_t = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;
		public:
			Optional() : m_isInit(false) {}
			Optional(const T& val) {
				create(val);
			}
			Optional(T&& val) : m_isInit(false) {
				create(std::move(v));
			}
			~Optional() {
				destroy();
			}
			Optional(const Optional& rhs) : m_isInit(false) {
				if (rhs.is_init())
					assign(rhs);
			}

			Optional(Optional&& rhs) : m_isInit(false) {
				if (rhs.is_init()) {
					assign(std::move(rhs));
				}
			}

			Optional& operator=(const Optional& rhs) {
				assign(rhs);
				return *this;
			}

			Optional& operator=(Optional&& rhs) {
				assign(std::move(rhs));
				return *this;
			}

			template <typename ...Args>
			void emplace(Args&&... args) {
				destroy();
				create(std::forward<Args>(args)...);
			}

			explicit operator bool() const {
				return is_init();
			}

			bool has_value() const {
				return is_init();
			}

			T& operator*() {
				if (is_init())
					return *(reinterpret_cast<observer_ptr<T>>(&m_data));
				throw std::logic_error("try to get data in an uninitialized optional.");
				//std::bad_optional_access
			}

			const T& operator*() const {
				return &operator*();
			}

			bool operator==(const Optional<T>& rhs) const
			{
				return (!bool(*this)) != (!rhs) ? false : (!bool(*this) ? true : (*(*this)) == (*rhs));
			}

			bool operator<(const Optional<T>& rhs) const
			{
				return !rhs ? false : (!bool(*this) ? true : (*(*this) < (*rhs)));
			}

			bool operator!=(const Optional<T>& rhs)
			{
				return !(*this == (rhs));
			}

			constexpr T& value() & {
				return operator*();
			}

			constexpr const T& value() const & {
				return operator*();
			}

			constexpr T&& value() && {
				return std::move(operator*());
			}

			constexpr const T&& value() const && {
				return std::move(operator*());
			}

			template <typename U>
			constexpr T value_or(U&& default_value) const& {
				return bool(*this) ? *(*this) : static_cast<T>(std::forward<U>(default_value));
			}

			template <typename U>
			constexpr T value_or(U&& default_value) && {
				return bool(*this) ? std::move(*(*this)) : static_cast<T>(std::forward<U>(default_value));
			}

			bool is_init() const {
				return m_isInit;
			}

		private:
			template <typename ...Args>
			void create(Args&&... args) {
				new (&m_data) T(std::forward<Args>(args)...);
				m_isInit = true;
			}

			void destroy() {
				if (m_isInit) {
					m_isInit = false;
					reinterpret_cast<observer_ptr<T>>(&m_data)->~T();
				}
			}

			void assign(const Optional& rhs) {
				if (rhs.is_init()) {
					copy(rhs.m_data);
					m_isInit = true;
					rhs.destroy();
				}
				else {
					destroy();
				}
			}

			void assign(Optional&& rhs) {
				if (rhs.is_init()) {
					move(std::move(rhs.m_data));
					m_isInit = true;
					rhs.destroy();
				}
				else {
					destroy();
				}
			}

			void move(data_t&& val) {
				destroy();
				new (&m_data) T(std::move(*reinterpret_cast<observer_ptr<T>>(&val)));
			}

			void copy(const data_t& val) {
				destroy();
				new (&m_data) T(*reinterpret_cast<observer_ptr<T>>(&val));
			}


			bool m_isInit;
			data_t m_data;
		};
	}

	namespace singleton {
		//CRTP singleton (or just inherit)
		template <typename T>
		class Singleton {
		public:
			static T& get_instance() {
				static T instance;
				return instance;
			}
		protected:
			Singleton() {}
			~Singleton() {}
		public:
			Singleton(const Singleton&) = delete;
			Singleton& operator=(const Singleton&) = delete;
		};
	}

	namespace variant {
		template <typename ...Args>
		class Variant {
			enum {
				data_size = sn_Assist::sn_numeric_assist::max_integer<sizeof(Args)...>::value,
				align_size = sn_Assist::sn_numeric_assist::max_align<Args...>::value,
			};
			using data_t = typename std::aligned_storage<data_size, align_size>::type;
		public:
			template <std::size_t index>
			using index_type = typename sn_Assist::sn_type_assist::visit_at<index, Args...>::type;

			Variant(void) : m_typeIndex(typeid(void)) {}
			~Variant() {
				destroy(m_typeIndex, &m_data);
			}

			Variant(const Variant<Args...>& rhs) : m_typeIndex(rhs.m_typeIndex) {
				copy(rhs.m_typeIndex, &rhs.m_data, &m_data);
			}

			Variant(Variant<Args...>&& rhs) : m_typeIndex(rhs.m_typeIndex) {
				move(rhs.m_typeIndex, &rhs.m_data, &m_data);
			}

			Variant& operator=(const Variant& rhs) {
				m_typeIndex = rhs.m_typeIndex;
				copy(rhs.m_typeIndex, &rhs.m_data, &m_data);
				return *this;
			}

			Variant& operator=(Variant&& rhs) {
				m_typeIndex = rhs.m_typeIndex;
				move(rhs.m_typeIndex, &rhs.m_data, &m_data);
				return *this;
			}

			template <typename T,
				typename = std::enable_if_t<
					sn_Assist::sn_type_assist::is_contain<
					std::decay_t<T>, Args...>::value>>
			Variant(T&& value) : m_typeIndex(typeid(std::decay_t<T>)) {
				destroy(m_typeIndex, &m_data);
				new (&m_data) std::decay_t<T>(std::forward<T>(value));
				get_type();
			}

			//TODO: change to is_convertible?
			template <typename T>
			bool is() const {
				return (m_typeIndex == std::type_index(typeid(T)));
			}

			bool empty() const {
				return (m_typeIndex == std::type_index(typeid(void)));
			}

			std::type_index type_info() const {
				return m_typeIndex;
			}

			template <typename T>
			std::decay_t<T>& get() {
				using U = std::decay_t<T>;
				if (!is<U>()) {
					SN_LOG_ERROR << typeid(U).name() << " is not defined. " << "current type is " << m_typeIndex.name();
					throw std::bad_cast();
				}

				return *reinterpret_cast<U*>(&m_data);
			}

			template <typename T>
			std::size_t type_index() const {
				return sn_Assist::sn_type_assist::contain_index<T, Args...>::value;
			}

			constexpr std::size_t index() const {
				//get_type();
				return m_index;
			}

			template <typename ...Ts>
			friend class Variant;

			bool operator==(const Variant& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data == rhs.m_data;
				}
				return false;
			}

			template <typename ...Ts>
			bool operator==(const Variant<Ts...>& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data == rhs.m_data;
				}
				return false;
			}

			bool operator<(const Variant& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data < rhs.m_data;
				}
				return false;
			}

			template <typename ...Ts>
			bool operator<(const Variant<Ts...>& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data < rhs.m_data;
				}
				return false;
			}

			bool operator>(const Variant& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data > rhs.m_data;
				}
				return false;
			}

			template <typename ...Ts>
			bool operator>(const Variant<Ts...>& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data > rhs.m_data;
				}
				return false;
			}

			bool operator>=(const Variant& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data >= rhs.m_data;
				}
				return false;
			}

			template <typename ...Ts>
			bool operator>=(const Variant<Ts...>& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data >= rhs.m_data;
				}
				return false;
			}

			bool operator<=(const Variant& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data <= rhs.m_data;
				}
				return false;
			}

			template <typename ...Ts>
			bool operator<=(const Variant<Ts...>& rhs) const {
				if (m_typeIndex == rhs.m_typeIndex) {
					return m_data <= rhs.m_data;
				}
				return false;
			}
		private:

			void get_type() const {
				if (m_index != -1)
					return;
				std::initializer_list<int>{(get_typeT<Args>(), 0)...};
			}

			template <typename T>
			void get_typeT() const {
				if (m_typeIndex == std::type_index(typeid(T)))
					m_index = type_index<T>();
			}

			const std::size_t m_index = -1;


			void destroy(const std::type_index& index, void* data) {
				std::initializer_list<int>{(destroyT<Args>(index, data), 0)...};
			}

			template <typename T>
			void destroyT(const std::type_index& index, void* data) {
				if (index == std::type_index(typeid(T)))
					reinterpret_cast<T*>(data)->~T();
			}

			void move(const std::type_index& index, void* old_data, void* data) {
				std::initializer_list<int>{(moveT<Args>(index, old_data, data), 0)...};
			}

			template <typename T>
			void moveT(const std::type_index& index, void* old_data, void* data) {
				if (index == std::type_index(typeid(T)))
					new (data) T(std::move(*reinterpret_cast<T*>(old_data)));
			}

			void copy(const std::type_index& index, const void* old_data, void* data) {
				std::initializer_list<int>{(copyT<Args>(index, old_data, data), 0)...};
			}

			template <typename T>
			void copyT(const std::type_index& index, const void* old_data, void* data) {
				if (index == std::type_index(typeid(T)))
					new (data) T(*reinterpret_cast<const T*>(old_data));
			}
		
			std::type_index m_typeIndex;
			data_t m_data;

		};
	}
#ifdef __GNUC__
	// constexpr variant
	namespace constexpr_variant {
		// ref: https://github.com/tomilov/variant/tree/master/include/versatile
		// VS2015 not support static constexpr non-integral...
		template <std::size_t I>
		using index_t = std::integral_constant<std::size_t, I>;
		
		struct in_place_t {};

		template <typename T = in_place_t>
		constexpr in_place_t in_place(T) {
			return{};
		}
		template <std::size_t I>
		constexpr in_place_t in_place(index_t<I>) {
			return{};
		}
		
		using sn_Assist::sn_type_assist::identity;
		using sn_TypeLisp::TypeList;
		// td = trivially destructible
		template <bool td, typename ...Args>
		class constructor_dispatcher;
		template <bool td>
		class constructor_dispatcher<td> {};
		template <typename T, typename ...Args>
		class constructor_dispatcher<true, T, Args...> {
			union {
				T m_head;
				constructor_dispatcher<true, Args...> m_tail;
			};
		public:
			template <typename ...PArgs>
			constexpr constructor_dispatcher(index_t<0>, PArgs&&... args)
				: m_head(std::forward<PArgs>(args)...) {}
			template <std::size_t I, typename ...PArgs>
			constexpr constructor_dispatcher(index_t<I>, PArgs&&... args)
				: m_tail(index_t<I - 1>{}, std::forward<PArgs>(args)...) {}
			using this_type = std::decay_t<T>;
			constexpr operator const this_type& () const noexcept {
				return m_head;
			}
			constexpr operator this_type& () noexcept {
				return m_head;
			}
			template <typename U>
			constexpr operator const U& () const noexcept {
				return m_tail;
			}
			template <typename U>
			constexpr operator U& () noexcept {
				return m_tail;
			}
		};
		template <typename T, typename ...Args>
		class constructor_dispatcher<false, T, Args...> {
			union {
				T m_head;
				constructor_dispatcher<true, Args...> m_tail;
			};
		public:
			constructor_dispatcher(const constructor_dispatcher&) = default;
			constructor_dispatcher(constructor_dispatcher&&) = default;
			constructor_dispatcher& operator= (const constructor_dispatcher&) = default;
			constructor_dispatcher& operator= (constructor_dispatcher&&) = default;
			~constructor_dispatcher() noexcept {}
			template <typename ...PArgs>
			constexpr constructor_dispatcher(index_t<0>, PArgs&&... args)
				: m_head(std::forward<PArgs>(args)...) {}
			template <std::size_t I, typename ...PArgs>
			constexpr constructor_dispatcher(index_t<I>, PArgs&&... args)
				: m_tail(index_t<I - 1>{}, std::forward<PArgs>(args)...) {}
			void destruct(in_place_t(&)(T)) noexcept {
				m_head.~T();
			}
			// Use function pointer to pass type
			template <typename U>
			void destruct(in_place_t(&)(U)) noexcept {
				m_tail.destruct(in_place<U>);
			}
			using this_type = std::decay_t<T>;
			constexpr operator const this_type& () const noexcept {
				return m_head;
			}
			constexpr operator this_type& () noexcept {
				return m_head;
			}
			template <typename U>
			constexpr operator const U& () const noexcept {
				return m_tail;
			}
			template <typename U>
			constexpr operator U& () noexcept {
				return m_tail;
			}
		};

		// td: trivially destructible
		template <bool td, typename ...Args>
		struct destructor_dispatcher {};

		template <typename ...Args>
		struct destructor_dispatcher<true, Args...> {
			std::size_t m_index;
			constructor_dispatcher<true, Args...> m_storage;
		public:
			constexpr std::size_t index() const noexcept {
				return m_index;
			}
			template <typename Is, typename ...PArgs>
			constexpr destructor_dispatcher(Is, PArgs&&... args)
				: m_index(Is::value), m_storage(Is{}, std::forward<PArgs>(args)...) {}
			template <typename U>
			constexpr operator const U& () const noexcept {
				return m_storage;
			}
			template <typename U>
			constexpr operator U& () noexcept {
				return m_storage;
			}
		};
		template <typename ...Args>
		struct destructor_dispatcher<false, Args...> {
			std::size_t m_index;
			constructor_dispatcher<false, Args...> m_storage;
			using storage = constructor_dispatcher<false, Args...>;

			template <typename U>
			static void destruct(storage& str) noexcept {
				str.destruct(in_place<U>);
			}
			using dtor = decltype(&destructor_dispatcher::template destruct<typename sn_Assist::sn_type_assist::identity<Args...>::type>);
			static constexpr dtor m_dtors[sizeof...(Args)] = { destructor_dispatcher::template destruct<Args>... };

		public:
			constexpr std::size_t index() const noexcept {
				return m_index;
			}
			destructor_dispatcher(const destructor_dispatcher&) = default;
			destructor_dispatcher(destructor_dispatcher&&) = default;
			destructor_dispatcher& operator= (const destructor_dispatcher&) = default;
			destructor_dispatcher& operator= (destructor_dispatcher&&) = default;
			~destructor_dispatcher() noexcept {
				if (0 < m_index)
					m_dtors[sizeof...(Args) - m_index](m_storage);
			}
			template <typename Is, typename ...PArgs>
			constexpr destructor_dispatcher(Is, PArgs&&... args)
				: m_index(Is::value), m_storage(Is{}, std::forward<PArgs>(args)...) {}
			template <typename U>
			constexpr operator const U& () const noexcept {
				return m_storage;
			}
			template <typename U>
			constexpr operator U& () noexcept {
				return m_storage;
			}
		};
		template <typename ...Args>
		constexpr const typename destructor_dispatcher<false, Args...>::dtor destructor_dispatcher<false, Args...>::m_dtors[sizeof...(Args)];

		template <bool ...I>
		struct get_index {};

		template <>
		struct get_index<> {};

		template <bool ...I>
		struct get_index<true, I...>
			: index_t<(1 + sizeof...(I))> {};
		template <bool ...I>
		struct get_index<false, I...>
			: get_index<I...> {};

		template <bool ...I>
		using get_index_t = typename get_index<I...>::value;

		template <typename ...Args>
		class versatile
			: sn_Assist::sn_functional_base::enable_default_constructor<(std::is_constructible<Args>::value || ...)> {
			destructor_dispatcher<(std::is_trivially_destructible<Args>::value && ...), Args...> m_storage;
		public:
			using variant_type = versatile;
			using types_t = identity<std::decay_t<Args>...>;
			using indices_t = std::index_sequence_for<Args...>;
			template <typename U>
			using index_at_t = index_t<sn_TypeLisp::TypeIndex_v<TypeList<Args...>, std::decay_t<U>>>;
			template <std::size_t I>
			using type_at_t = sn_TypeLisp::TypeAt_t<TypeList<Args...>, I>;
			template <typename ...PArgs>
			using index_of_constructible_t = get_index_t<std::is_constructible<Args, PArgs...>::value...>;
			constexpr std::size_t index() const noexcept {
				return m_storage.index();
			}
			template <typename U>
			constexpr bool active() const noexcept {
				return (m_storage.index() == index_at_t<U>::value);
			}
			template <typename T, typename I = index_at_t<T>>
			constexpr T& get() {
				return (active<T>() ? m_storage : throw std::bad_cast{});
			}
			/*
			template <std::size_t I, typename T = type_at_t<I>>
			constexpr T& get() const {
				return (active<T>() ? m_storage : throw std::bad_cast{});
			}
			*/
			template <typename T, typename I = index_at_t<T>>
			constexpr const T& get() const {
				return (active<T>() ? m_storage : throw std::bad_cast{});
			}
			template <std::size_t I, typename T = type_at_t<I>>
			constexpr const T& get() const {
				return (active<T>() ? m_storage : throw std::bad_cast{});
			}
			constexpr versatile()
				: versatile(in_place<>) {}
			template <std::size_t I, typename ...PArgs>
			explicit constexpr versatile(in_place_t(&)(index_t<I>), PArgs&&... args)
				: versatile::enable_type({}), m_storage(index_t<I>{}, std::forward<PArgs>(args)...) {}
			template <typename T, typename I = index_at_t<T>>
			constexpr versatile(T&& v)
				: versatile(in_place<I>, std::forward<T>(v)) {}
			/*
			template <typename T, typename ...PArgs, typename I = index_at_t<T>>
			explicit constexpr versatile(in_place_t(&)(T), PArgs&&... args)
				: versatile(in_place<I>, std::forward<PArgs>(args)...) {}
			*/
			template <typename ...PArgs, typename I = index_of_constructible_t<PArgs...>>
			explicit constexpr versatile(in_place_t(&)(in_place_t), PArgs&&... args)
				: versatile(in_place<I>, std::forward<PArgs>(args)...) {}
			template <typename ...PArgs, typename V = std::enable_if_t<(1 < sizeof...(PArgs))>, typename I = index_of_constructible_t<PArgs...>>
			explicit constexpr versatile(PArgs&&... args)
				: versatile(in_place<I>, std::forward<PArgs>(args)...) {}
			
			constexpr void swap(versatile & rhs) noexcept(std::is_nothrow_move_assignable<versatile>::value && std::is_nothrow_move_constructible<versatile>::value) {
				versatile this_ = std::move(*this);
				*this = std::move(rhs);
				rhs = std::move(this_);
			}
			template <typename T, typename I = index_at_t<T>>
			constexpr versatile& operator= (T&& v) noexcept {
				return (*this = versatile(std::forward<T>(v)));
			}
			template <typename T, typename I = index_at_t<T>>
			explicit constexpr operator const T& () const {
				return (active<T>() ? m_storage : throw std::bad_cast{});
			}
			template <typename T, typename I = index_at_t<T>>
			explicit constexpr operator T& () {
				return (active<T>() ? m_storage : throw std::bad_cast{});
			}
		};
		template <>
		class versatile<> {};

		// constexpr auto value = v.get<decltype(v)::type_at_t<v.index()>>();
		// T conversion is broken

		/*
		template <typename Vis, typename ...Vars>
		class dispatcher {};

		template <typename Vis, template <typename ...> class Var, typename ...Vars, typename ...Ts>
		class dispatcher<Vis, Var<Ts...>, Vars...> {
			
			template <typename ...Args>
			struct result_type {
				using type = sn_TypeLisp::TypeList<decltype(std::declval<Vis>()(std::declval<Var>(), std::declval<Args>()...))>;
			};
			
			template <typename ...Args>
			using result_type_t = sn_TypeLisp::TypeCar_t<typename result_type<Args...>::type>;

			template <typename T, typename ...Args>
			static constexpr result_type_t<Args...> callee(Vis&& vis, Var&& var, Args&&... args) {
				return std::forward<Vis>(vis)(
					std::forward<T>(var.get<T>()),
					std::forward<Args>(args)...
				);
			}

			template <typename ...Args>
			using callee_t = decltype(&dispatcher::template callee<Var, Args...>);

			template <typename ...Args>
			static constexpr callee_t<Args...> m_callies[sizeof...(Ts)] = {
				dispatcher::template callee<Ts, Args...>...
			};
			
		public:

			template <typename ...Args>
			static constexpr result_type_t<Args...> caller(Vis&& vis, Var&& var, Args&&... args) {
				const std::size_t idx = var.index();
				assert(!(sizeof...(Ts) < idx));
				return (0 < idx) ? 
					m_callies<Args...>[sizeof...(Ts) - idx](
						std::forward<Vis>(vis), 
						std::forward<Var>(var), 
						std::forward<Args>(args)...
						)
					:
					throw std::bad_cast("Bad index.");
			}
		};

		template <typename Vis, template <typename ...> class Var, typename...Vars, typename ...Ts>
		template <typename ...Args>
		constexpr typename dispatcher<Vis, Var<Ts...>, Vars...>::template callee_t<Args...> dispatcher<Vis, Var<Ts...>, Vars...>::m_callies[sizeof...(Ts)];

		template <typename Vis, typename Var, typename ...Vars>
		constexpr decltype(auto) visit(Vis&& vis, Var&& var, Vars&&... vars) {
			return dispatcher<Vis, Var, Vars...>::template caller<Vars...>(
				std::forward<Vis>(vis), 
				std::forward<Var>(var), 
				std::forward<Vars>(vars)...
			);
		}
		*/

		template <typename Var>
		struct variant_size {};

		template <typename Var>
		struct variant_size<const Var> : variant_size<Var> {};

		template <typename Var>
		struct variant_size<volatile Var> : variant_size<Var> {};
		
		template <typename Var>
		struct variant_size<const volatile Var> : variant_size<Var> {};
		
		template <typename ...Ts>
		struct variant_size<versatile<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

		template <typename Var>
		/*inline*/ constexpr const std::size_t variant_size_v = variant_size<Var>::value;

		template <std::size_t Np, typename Var>
		struct variant_alternative {};

		template <std::size_t Np, typename T, typename ...Ts>
		struct variant_alternative<Np, versatile<T, Ts...>>
			: variant_alternative<Np - 1, versatile<Ts...>> {};
		
		template <typename T, typename ...Ts>
		struct variant_alternative<0, versatile<T, Ts...>> {
			using type = T;
		};

		template <std::size_t Np, typename Var>
		using variant_alternative_t = typename variant_alternative<Np, Var>::type;
		
		template <std::size_t Np, typename Var>
		struct variant_alternative<Np, const Var> {
			using type = std::add_const_t<variant_alternative_t<Np, Var>>;
		};

		template <std::size_t Np, typename Var>
		struct variant_alternative<Np, volatile Var> {
			using type = std::add_const_t<variant_alternative_t<Np, Var>>;
		};

		template <std::size_t Np, typename Var>
		struct variant_alternative<Np, const volatile Var> {
			using type = std::add_const_t<variant_alternative_t<Np, Var>>;
		};

		
		template <std::size_t I, typename ...Ts>
		constexpr auto get(versatile<Ts...>& var) {
			return var.template get<I>();
		}

		// GCC-SVN visit implementaion
		namespace vt {

			template <std::size_t Np, typename ...Ts>
			struct Nth_type;

			template <std::size_t Np, typename T, typename ...Ts>
			struct Nth_type<Np, T, Ts...>
				: Nth_type<Np - 1, Ts...> {};

			template <typename T, typename ...Ts>
			struct Nth_type<0, T, Ts...> {
				using type = T;
			};

			template <typename T, std::size_t ...Dimensions>
			struct multi_array {
				constexpr const T& M_access() const { return M_data; }
				T M_data;
			};

			template <typename T, std::size_t first, std::size_t ...rest>
			struct multi_array<T, first, rest...> {
				template <typename ...Args>
				constexpr const T& M_access(std::size_t first_idx, Args&&... rest_idxs) const {
					return M_arr[first_idx].M_access(rest_idxs...);
				}
				multi_array<T, rest...> M_arr[first];
			};

			template <typename ArrayT, typename VarTp, typename IdxSeq>
			struct gen_vtable_impl {};

			template <typename RT, typename Vis, std::size_t... Dis, typename... Vars, std::size_t... Idxs>
			struct gen_vtable_impl<multi_array<RT(*)(Vis, Vars...), Dis...>, std::tuple<Vars...>, std::index_sequence<Idxs...>> {
				using next_t = std::remove_reference_t<typename Nth_type<sizeof...(Idxs), Vars...>::type>;
				using array_t = multi_array<RT(*)(Vis, Vars...), Dis...>;
				static constexpr array_t S_apply() {
					array_t vtable{};
					S_apply_all_alts(vtable, std::make_index_sequence<variant_size_v<next_t>>());
					return vtable;
				}
				template <std::size_t ...var_idxs>
				static constexpr void S_apply_all_alts(array_t& vtable, std::index_sequence<var_idxs...>) {
					std::initializer_list<int>{(S_apply_single_alt<var_idxs>(vtable.M_arr[var_idxs]), 0)...};
				}
				template <std::size_t idx, typename T>
				static constexpr void S_apply_single_alt(T& elem) {
					using alternative_t = variant_alternative_t<idx, next_t>;
					elem = gen_vtable_impl<std::remove_reference_t<decltype(elem)>, std::tuple<Vars...>, std::index_sequence<Idxs..., idx>>::S_apply();
				}
			};

			template <typename RT, typename Vis, typename ...Vars, std::size_t ...Idx>
			struct gen_vtable_impl<multi_array<RT(*)(Vis, Vars...)>, std::tuple<Vars...>, std::index_sequence<Idx...>> {
				using array_t = multi_array<RT(*)(Vis&&, Vars...)>;
				static constexpr decltype(auto) visit_invoke(Vis&& vis, Vars... vars) {
					return std::forward<Vis>(vis)(get<Idx>(std::forward<Vars>(vars))...);
				}
				static constexpr auto S_apply() {
					return array_t{&visit_invoke};
				}
			};

			template <typename RT, typename Vis, typename ...Vars>
			struct gen_vtable {
				using func_ptr_t = RT(*)(Vis&&, Vars...);
				using array_t = multi_array<func_ptr_t, variant_size_v<std::remove_reference_t<Vars>>...>;
				static constexpr array_t S_apply() {
					return gen_vtable_impl<array_t, std::tuple<Vars...>, std::index_sequence<>>::S_apply();
				}
				static constexpr auto S_vtable = S_apply();
			};
		}
		

		template <typename Vis, typename ...Vars>
		constexpr decltype(auto) visit(Vis&& vis, Vars&&... vars) {
			using result_type = decltype(
				std::forward<Vis>(vis)(
					get<0>(std::forward<Vars>(vars)...)
				)
			);
			constexpr auto& vtable = vt::gen_vtable<result_type, Vis, Vars...>::S_vtable;
			auto func_ptr = vtable.M_access(vars.index()...);
			return (*func_ptr)(std::forward<Vis>(vis), std::forward<Vars>(vars)...);
		}

	}
#endif

	namespace heterogeneous {
		
		// ref: https://gieseanw.wordpress.com/2017/05/03/a-true-heterogeneous-container-in-c/
		class HeterogeneousContainer {
		public:
			HeterogeneousContainer() = default;
			HeterogeneousContainer(const HeterogeneousContainer& rhs) {
				*this = rhs;
			}
			HeterogeneousContainer& operator=(const HeterogeneousContainer& rhs) {
				clear();
				m_clearFuncs = rhs.m_clearFuncs;
				m_copyFuncs = rhs.m_copyFuncs;
				m_sizeFuncs = rhs.m_sizeFuncs;
				for (auto&& copyFunc : m_copyFuncs)
					copyFunc(rhs, *this);
				return *this;
			}

			template <typename T>
			void push_back(const T& t) {
				if (m_items<T>.find(this) == std::end(m_items<T>)) {
					m_clearFuncs.emplace_back([](HeterogeneousContainer& c){
						m_items<T>.erase(&c);
					});
					m_copyFuncs.emplace_back([](const HeterogeneousContainer& from, HeterogeneousContainer& to){
						m_items<T>[&to] = m_items<T>[&from];
					});
					m_sizeFuncs.emplace_back([](const HeterogeneousContainer& c){
						return m_items<T>[&c].size();
					});
					m_items<T>[this].push_back(t);
				}
			}

			void clear() {
				for (auto&& clearFunc : m_clearFuncs)
					clearFunc(*this);
			}

			std::size_t size() const {
				std::size_t sum = 0;
				for (auto&& sizeFunc : m_sizeFuncs)
					sum += sizeFunc(*this);
				return sum;
			}

			~HeterogeneousContainer() {
				clear();
			}

			template <typename T>
			void visit(T&& visitor) {
				visit_impl(visitor, typename std::decay_t<T>::types{});
			}

		private:
			template <typename T>
			static std::unordered_map<const HeterogeneousContainer*, std::vector<T>> m_items;
			
			template <typename T, typename U>
			using visit_function = decltype(std::declval<T>().operator()(std::declval<U&>()));

			template <typename T, typename U>
			static constexpr bool has_visit_v = sn_Assist::sn_detect::sn_is_detected<visit_function, T, U>::value;

			template <typename T, template <typename...> typename TL, typename ...Args>
			void visit_impl(T&& visitor, TL<Args...>) {
				//(..., visit_impl_helper<std::decay_t<T>, Args>(visitor));
				std::initializer_list<int>{(visit_impl_helper<std::decay_t<T>, Args>(visitor), 0)...};
			}

			template <typename T, typename U>
			void visit_impl_helper(T& visitor) {
				static_assert(has_visit_v<T, U>, "Visitors must provide a visit function accepting a reference to each type");
				for (auto&& elem : m_items<U>[this])
					visitor(elem);
			}

			std::vector<std::function<void(HeterogeneousContainer&)>> m_clearFuncs;
			std::vector<std::function<void(const HeterogeneousContainer&, HeterogeneousContainer&)>> m_copyFuncs;
			std::vector<std::function<std::size_t(const HeterogeneousContainer&)>> m_sizeFuncs;
			
		};

		template <typename T>
		std::unordered_map<const HeterogeneousContainer*, std::vector<T>> HeterogeneousContainer::m_items;

		template <typename ...Args>
		struct TypeList {};

		template <typename ...Args>
		struct base_visitor {
			using types = TypeList<Args...>;
		};

	}

	namespace heterogeneous_option_map {
		class typeinfo {
			const std::type_info* m_info;
		public:
			typeinfo()
				: m_info(0) {}
			typeinfo(const std::type_info& t)
				: m_info(&t);
			inline const char* name() const {
				return m_info ? m_info->name() : "";
			}
			inline bool operator<(const typeinfo& rhs) const {
				return (m_info != rhs.m_info) &&
					(!m_info !! (rhs.m_info && static_cast<bool>(m_info->before(*rhs.m_info))));
			}
			inline bool operator==(const typeinfo& that) const {
				return (m_info == rhs.m_info) &&
					(m_info !! (rhs.m_info && static_cast<bool>(m_info == rhs.m_info));
			}
		};

		template <typename T>
		struct initialized_value {
			T result;
			initialized_value()
				: result() {}
		};

		template <typename userkey_t>
		class option_map {
			struct generic_t {
				void* obj;
				void (*copy)(void*, const void*);
				void (*del)(void*);
			};
			using key_t = std::pair<userkey_t, typeinfo>;
			using map_t = std::map<key_t, generic_t>;
			using iterator_t = typename map_t::iterator;
			map_t m_map;
			template <typename T>
			bool put(const userkey_t& name, const T& value) {
				struct local_cast {
					static void copy(void* dest, const void* src) {
						*static_cast<T*>(dest) = *static_cast<const T*>(src);
					}
					static void destroy(void* p) {
						delete static_cast<T*>(p);
					}	
				};
				generic_t& p = m_map[key_t(name, typeid(T))];
				p.obj = new T(value);
				p.copy = &local_cast::copy;
				p.del = &local_cast::destroy;
				return true;
			}
			std::size_t size() const {
				return m_map.size();
			}
			template <typename T>
			bool find(const userkey_t& name) const {
				return m_map.find(key_t(name, typeid(T))) != map_.end();
			}
			template <typename T>
			bool get(T& dest, const userkey_t& name) const {
				const typename map_t::const_iterator i = m_map.find(key_t(name, typeid(T)));
				const bool test = (i != m_map.end());
				if (test && i->second.obj)
				i->second.copy(&dest, i->second.obj);
				return test;
			}
			template <typename T>
			T get(const userkey_t& name) const {
				std::initialized_value<T> v;
				get(v.result, name);
				return v.result;
			}
			bool scan(const userkey_t& name) const {
				const typename map_t::const_iterator i
					= m_map.upper_bound(key_t(name, typeinfo()));
				return i != m_map.end() && i->first.first == name;
			}
			~option_map() {
				iterator_t i = m_map.begin();
				while (i != m_map.end())	{
					generic_t& p = (i++)->second;
					if (p.del)
						p.del(p.obj);
				}
			}
		};

		class option_parser {
			using option_map_t = option_map<std::string>;
			using store_t = bool(*)(option_map_t&, const char*, const char*);
			using map_t = std::map<std::string, store_t>;
			map_t m_map;
		public:
			template <typename T>
			void declare_as(const char* const name) {
				struct local_store {
					static bool store(
						option_map_t& m,
						const char* name, 
						const char* value) {
						std::istringstream is(value);
						T temp;
						return (is >> temp) && m.put(name, temp);
					}
				};
				m_map[name] = &local_store::store;
			}
			template <typename iterator_t>
			iterator_t parse(option_map_t& m, iterator_t begin, iterator_t end) {
			/*
				for every iterator i=begin...end {
					get the string S = *i;
					if S has no prefix
						stop and return i;
					else
						remove the prefix
					
					if S has the form "N=V"
						split S in N and V
					else
						set N = S
						set V = <empty string>
					if N is not contained in m_map
						throw exception "unknown option"
					else
						set F := local_store::store
						execute F(m, N, V)
					if it fails, throw exception "illegal value" 
				}
			*/
			}
		};

		struct option {};
		inline std::istream& operator>>(std::istream& is, option&) {
			return is;
		}
	}

	namespace helper {
		template <typename T, typename ...Args>
		any::Any make_any(Args&&... args) {
			return any::Any<T, Args...>(std::forward<Args>(args)...); 
		}
		
		template <typename T>
		constexpr optional::Optional<std::decay_t<T>> make_optional(T&& value) {
			return optional::Optional<std::decay_t<T>>(std::forward<T>(value));
		}

		template <typename T, typename ...Args>
		constexpr optional::Optional<std::decay_t<T>> make_optional(T&& value) {
			return optional::Optional<std::decay_t<T>>(std::forward<T>(value)).emplace(std::forward<Args>(args)...);
		}

		// actual it's not constexpr index()
		template <typename Overloader, typename ...Args>
		constexpr auto visit(Overloader&& vis, variant::Variant<Args...> var) {
			return sn_Assist::sn_invoke(std::forward<Overloader>(vis), var.get<sn_Assist::sn_type_assist::visit_at<var.index(), Args...>>());
		}

		/* Usage:
		auto vis = sn_Assist::sn_overload::make_overload(
		[](int a) { return 1; },
		[](double b) {return 2;}
		);
		sn_Type::helper::visit(vis, var);

		*/
	}

}



#endif