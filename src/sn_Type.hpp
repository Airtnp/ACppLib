#ifndef SN_TYPE_H
#define SN_TYPE_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Log.hpp"

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
			Any(void) : m_typeIndex(std::type_index(typeid(void))) {}
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

			std::size_t index() const {
				get_type();
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

			void get_type() {
				if (m_index != -1)
					return;
				std::initializer_list<int>{(get_typeT<Args>(), 0)...};
			}

			template <typename T>
			void get_typeT() {
				if (m_typeIndex == std::type_index(typeid(T)))
					m_index = type_index<T>();
			}

			std::size_t m_index = -1;


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