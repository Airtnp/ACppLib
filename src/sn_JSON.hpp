#ifndef SN_JSON_H
#define SN_JSON_H

#include "sn_CommonHeader.h"

namespace sn_JSON {

	namespace serializer {
		template <typename T = void, typename V = void>
		class json_serializer {};
	}

	namespace container {
		// ref: https://github.com/nlohmann/json/blob/develop/src/json.hpp

		using serializer::json_serializer;

		enum class value_t : uint8_t {
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
			using value_t = value_t;
			using object_t = objectT<stringT, basic_json, std::less<stringT>, allocatorT<std::pair<const stringT, basic_json>>>;
			using array_t = arrayT<basic_json, allocatorT<basic_json>>;
			using string_t = stringT;
			using boolean_t = booleanT;
			using integer_t = integerT;
			using double_t = doubleT;

		private:
			template <typename T, typename ...Args>
			static T* create(Args&&... args) {
				allocatorT alloc;
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
						object = create<array_t>();
						break;
					case value_t::string:
						object = create<string_t>("");
						break;
					case value_t::boolean:
						object = create<boolean_t>(false);
						break;
					case value_t::number_integer:
						object = create<integer_t>(0);
						break;
					case value_t::number_double:
						object = create<double_t>(0.0);
						break;
					case value_t::null:
						break;
					default:
						if (t == value_t::null)
							throw std::runtime_error("Unknown type.");
						break;
					}
				}
			private:
				value_t m_type;
				json_value m_value;
			};
		};

		
	}

	namespace parser {

	}
}


#endif