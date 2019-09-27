#ifndef SN_JSON_H
#define SN_JSON_H

#include "sn_CommonHeader.h"

namespace sn_JSON {

	namespace serializer {
		template <typename T = void, typename V = void>
		class json_serializer {};
	}

	// TODO: add output formatting
	// TODO: add parser/serializer
	namespace container {
		// ref: https://github.com/nlohmann/json/blob/develop/src/json.hpp

		using serializer::json_serializer;

		enum class xvalue_t : uint8_t {
			null,
			object,
			array,
			string,
			boolean,
			number_integer,
			number_double,
			discarded,
			num_of_values
		};

		template <
			template <typename U, typename V, typename ...Args> typename objectT = std::map,
			template <typename U, typename ...Args> typename arrayT = std::vector,
			typename stringT = std::string,
			typename booleanT = bool,
			typename integerT = std::int64_t,
			typename doubleT = double,
			template <typename U> typename allocatorT = std::allocator,
			template <typename U, typename V = void> typename serializerT = json_serializer
		>
			class basic_json {
			using basic_json_t = basic_json;
			public:
				using value_t = xvalue_t;
				using object_t = objectT<stringT, basic_json, std::less<stringT>, allocatorT<std::pair<const stringT, basic_json>>>;
				using array_t = arrayT<basic_json, allocatorT<basic_json>>;
				using string_t = stringT;
				using boolean_t = booleanT;
				using integer_t = integerT;
				using double_t = doubleT;

			private:
				template <typename T, typename ...Args>
				static T* create(Args&&... args) {
					allocatorT<T> alloc;
					auto deleter = [&](T* object) {
						alloc.deallocate(object, 1);
					};
					std::unique_ptr<T, decltype(deleter)> object(alloc.allocate(1), deleter);
					// deprecated in C++1z
					alloc.construct(object.get(), std::forward<Args>(args)...);
					assert(object != nullptr && "Failed to allocate.");
					return object.release();
				}


			public:

				union json_value {
					object_t* object;
					array_t* array;
					string_t* string;
					boolean_t boolean;
					integer_t num_integer;
					double_t num_double;

					json_value() = default;
					json_value(boolean_t v) noexcept : boolean(v) {}
					json_value(integer_t v) noexcept : num_integer(v) {}
					json_value(double_t v) noexcept : num_double(v) {}
					json_value(const string_t& v) {
						string = create<string_t>(v);
					}
					json_value(const object_t& v) {
						object = create<object_t>(v);
					}
					json_value(const array_t& v) {
						array = create<array_t>(v);
					}
					json_value(value_t t) {
						switch (t) {
						case value_t::object:
							object = create<object_t>();
							break;
						case value_t::array:
							array = create<array_t>();
							break;
						case value_t::string:
							string = create<string_t>("");
							break;
						case value_t::boolean:
							boolean = boolean_t(false);
							break;
						case value_t::number_integer:
							num_integer = integer_t(0);
							break;
						case value_t::number_double:
							num_double = double_t(0.0);
							break;
						case value_t::null:
							break;
						default:
							if (t == value_t::null)
								throw std::runtime_error("Unknown type.");
							break;
						}
					}
				};
				basic_json(const value_t v)
					: m_type(v), m_value(v) {}
				basic_json(std::nullptr_t = nullptr) noexcept
					: basic_json(value_t::null) {}

				template <typename T, typename V = void>
				struct dispatcher {};

				template <typename V>
				struct dispatcher<object_t, V> {
					static const value_t value = value_t::object;
					using type = object_t;
				};
				template <typename V>
				struct dispatcher<array_t, V> {
					static const value_t value = value_t::array;
					using type = array_t;
				};
				template <typename V>
				struct dispatcher<string_t, V> {
					static const value_t value = value_t::string;
					using type = string_t;
				};
				template <typename V>
				struct dispatcher<boolean_t, V> {
					static const value_t value = value_t::boolean;
					using type = boolean_t;
				};
				template <typename T>
				struct dispatcher<T, std::enable_if_t<std::is_integral<T>::value>> {
					static const value_t value = value_t::number_integer;
					using type = integer_t;
				};
				template <typename T>
				struct dispatcher<T, std::enable_if_t<std::is_floating_point<T>::value>> {
					static const value_t value = value_t::number_double;
					using type = double_t;
				};

				template <typename T,
					typename V = std::enable_if_t<
					std::is_same<T, object_t>::value ||
					std::is_same<T, array_t>::value ||
					std::is_same<T, string_t>::value ||
					std::is_integral<T>::value ||
					std::is_floating_point<T>::value ||
					std::is_same<T, boolean_t>::value>
				>
					basic_json(T&& v) noexcept
					: m_type(dispatcher<T>::value), m_value(static_cast<typename dispatcher<T>::type>(v)) {}

				basic_json(const char* p) noexcept
					: m_type(value_t::string), m_value(std::string(p)) {}

				template <typename T,
					typename V = std::enable_if_t<
					std::is_same<T, object_t>::value ||
					std::is_same<T, array_t>::value ||
					std::is_same<T, string_t>::value ||
					std::is_integral<T>::value ||
					std::is_floating_point<T>::value ||
					std::is_same<T, boolean_t>::value>
				>
					basic_json& operator=(T&& v) noexcept {
					m_type = dispatcher<T>::value;
					m_value = typename dispatcher<T>::type(v);
					return *this;
				}

				basic_json& operator=(const char* p) noexcept {
					m_type = value_t::string;
					m_value = std::string(p);
					return *this;
				}

				basic_json(std::initializer_list<basic_json> init) {
					bool is_an_object = std::all_of(init.begin(), init.end(), [](const basic_json& elem) {
						return elem.is_array() && elem.size() == 2 && elem[0].is_string();
					});
					if (is_an_object) {
						m_type = value_t::object;
						m_value = value_t::object;
						std::for_each(init.begin(), init.end(), [this](const basic_json& elem) {
							m_value.object->emplace(*(elem[0].m_value.string), elem[1]);
						});
					}
					else {
						m_type = value_t::array;
						m_value.array = create<array_t>(init);
					}
					assert_valid();
				}

				basic_json& operator=(std::initializer_list<basic_json> init) {
					bool is_an_object = std::all_of(init.begin(), init.end(), [](const basic_json& elem) {
						return elem.is_array() && elem.size() == 2 && elem[0].is_string();
					});
					if (is_an_object) {
						m_type = value_t::object;
						m_value = value_t::object;
						std::for_each(init.begin(), init.end(), [this](const basic_json& elem) {
							m_value.object->emplace(*(elem[0].m_value.string), elem[1]);
						});
					}
					else {
						m_type = value_t::array;
						m_value.array = create<array_t>(init);
					}
					assert_valid();
					return *this;
				}


				static basic_json array(std::initializer_list<basic_json> init = std::initializer_list<basic_json>()) {
					return basic_json(init);
				}
				static basic_json object(std::initializer_list<basic_json> init = std::initializer_list<basic_json>()) {
					return basic_json(init);
				}
				basic_json(std::size_t n, const basic_json& val)
					: m_type(value_t::array) {
					m_value.array = create<array_t>(n, val);
					assert_valid();
				}
				basic_json(const basic_json& rhs)
					: m_type(rhs.m_type) {
					rhs.assert_valid();
					switch (rhs.m_type) {
					case value_t::object:
						m_value = *rhs.m_value.object;
						break;
					case value_t::array:
						m_value = *rhs.m_value.array;
						break;
					case value_t::string:
						m_value = *rhs.m_value.string;
						break;
					case value_t::boolean:
						m_value = rhs.m_value.boolean;
						break;
					case value_t::number_integer:
						m_value = rhs.m_value.num_integer;
						break;
					case value_t::number_double:
						m_value = rhs.m_value.num_double;
						break;
					case value_t::null:
						break;
					default:
						break;
					}
					assert_valid();
				}
				basic_json(basic_json&& rhs) noexcept
					: m_type(std::move(rhs.m_type)), m_value(std::move(rhs.m_value)) {
					rhs.m_type = value_t::null;
					rhs.m_value = {};
					assert_valid();
				}
				basic_json& operator=(basic_json rhs) noexcept (
					std::is_nothrow_move_constructible<value_t>::value &&
					std::is_nothrow_move_assignable<value_t>::value &&
					std::is_nothrow_move_constructible<json_value>::value &&
					std::is_nothrow_move_assignable<json_value>::value
					) {
					rhs.assert_valid();
					using std::swap;
					swap(m_type, rhs.m_type);
					swap(m_value, rhs.m_value);
					assert_valid();
					return *this;
				}
				~basic_json() {
					assert_valid();
					switch (m_type) {
					case value_t::object: {
						allocatorT<object_t> alloc;
						alloc.destroy(m_value.object);
						alloc.deallocate(m_value.object, 1);
						break;
					}
					case value_t::array: {
						allocatorT<array_t> alloc;
						alloc.destroy(m_value.array);
						alloc.deallocate(m_value.array, 1);
						break;
					}
					case value_t::string: {
						allocatorT<string_t> alloc;
						alloc.destroy(m_value.string);
						alloc.deallocate(m_value.string, 1);
						break;
					}
					default:
						break;
					}
				}

				constexpr value_t type() const noexcept {
					return m_type;
				}
				constexpr bool is_primitive() const noexcept {
					return is_null() || is_string() || is_boolean() || is_number();
				}
				std::size_t size() const noexcept
				{
					switch (m_type) {
					case value_t::null: {
						return 0;
					}
					case value_t::array: {
						return m_value.array->size();
					}
					case value_t::object: {
						return m_value.object->size();
					}
					default: {
						return 1;
					}

					}
				}
				constexpr bool is_structured() const noexcept {
					return is_array() || is_object();
				}

				constexpr bool is_null() const noexcept {
					return m_type == value_t::null;
				}
				constexpr bool is_boolean() const noexcept {
					return m_type == value_t::boolean;
				}
				constexpr bool is_number() const noexcept {
					return is_number_integer() || is_number_double();
				}
				constexpr bool is_number_integer() const noexcept {
					return m_type == value_t::number_integer;
				}
				constexpr bool is_number_double() const noexcept {
					return m_type == value_t::number_double;
				}
				constexpr bool is_object() const noexcept {
					return m_type == value_t::object;
				}
				constexpr bool is_array() const noexcept {
					return m_type == value_t::array;
				}
				constexpr bool is_string() const noexcept {
					return m_type == value_t::string;
				}
				constexpr bool is_discarded() const noexcept {
					return m_type == value_t::discarded;
				}

				constexpr operator value_t() const noexcept {
					return m_type;
				}

				basic_json get() const {
					return *this;
				}

				basic_json& at(std::size_t idx) {
					if (is_array()) {
						try {
							return m_value.array->at(idx);
						}
						catch (std::out_of_range&) {
							throw std::runtime_error("Out of range.");
						}
					}
					else {
						throw std::runtime_error("Cannot use at with non-array json type.");
					}
				}
				const basic_json& at(std::size_t idx) const {
					if (is_array()) {
						try {
							return m_value.array->at(idx);
						}
						catch (std::out_of_range&) {
							throw std::runtime_error("Out of range.");
						}
					}
					else {
						throw std::runtime_error("Cannot use at with non-array json type.");
					}
				}
				basic_json& at(typename object_t::key_type& idx) {
					if (is_object()) {
						try {
							return m_value.object->at(idx);
						}
						catch (std::out_of_range&) {
							throw std::runtime_error("Out of range.");
						}
					}
					else {
						throw std::runtime_error("Cannot use at(key) with non-object json type.");
					}
				}
				const basic_json& at(typename object_t::key_type& idx) const {
					if (is_object()) {
						try {
							return m_value.object->at(idx);
						}
						catch (std::out_of_range&) {
							throw std::runtime_error("Out of range.");
						}
					}
					else {
						throw std::runtime_error("Cannot use at(key) with non-object json type.");
					}
				}

				basic_json& operator[](std::size_t idx) {
					if (is_null()) {
						m_type = value_t::array;
						m_value.array = create<array_t>();
						assert_valid();
					}
					if (is_array()) {
						if (idx >= m_value.array->size())
							m_value.array->insert(m_value.array->end(), idx - m_value.array->size() + 1, basic_json());
						return m_value.array->operator[](idx);
					}
					throw std::runtime_error("Cannot use [](idx) with non-array json type.");
				}
				const basic_json& operator[](std::size_t idx) const {
					if (is_array()) {
						return m_value.array->operator[](idx);
					}
					throw std::runtime_error("Cannot use [](idx) with non-array json type.");
				}

				basic_json& operator[](typename object_t::key_type idx) {
					if (is_null()) {
						m_type = value_t::object;
						m_value.object = create<object_t>();
						assert_valid();
					}
					if (is_object()) {
						return m_value.object->operator[](idx);
					}
					throw std::runtime_error("Cannot use [](key) with non-object json type.");
				}
				const basic_json& operator[](typename object_t::key_type idx) const {
					if (is_object()) {
						assert(m_value.object->find(idx) != m_value.object->end());
						return m_value.object->operator[](idx);
					}
					throw std::runtime_error("Cannot use [](key) with non-object json type.");
				}
			private:

				void assert_valid() const {
					assert(m_type != value_t::object || m_value.object != nullptr);
					assert(m_type != value_t::array || m_value.array != nullptr);
					assert(m_type != value_t::string || m_value.string != nullptr);
				}

				value_t m_type;
				json_value m_value;
		};
	}
	using json = container::basic_json<>;
	
	namespace parser {

	}
}


#endif