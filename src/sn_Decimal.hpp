#ifndef SN_DECIMAL_H
#define SN_DECIMAL_H

#include "sn_CommonHeader.h"
#include "sn_Log.hpp"
#include "sn_Macro.hpp"


// TODO: add FFT/NTT multiplication
// TODO: add template <char...> operator
namespace sn_Decimal {

	

	class sn_decimal_bit {
	private:
		short value;
		
	public:
		
		static const size_t sign = 0;
		static const size_t size = 4;
		static const short max = 9;
		sn_decimal_bit() : value(0) {}
		sn_decimal_bit(int r) : value(r) {}
		sn_decimal_bit(const sn_decimal_bit& r) = default;
		sn_decimal_bit& operator=(const sn_decimal_bit& r) = default;
		
		sn_decimal_bit(long long int r) : value(r) {}
		operator long long int() {
			return static_cast<long long int>(value);
		}

		bool is_zero() {
			return value == 0;
		}

	};

	

	template <typename T>
	class sn_block_array {
	private:
		size_t capacity;
		observer_ptr<T> blocks;  //may use linked_list instead
	public:
		static const size_t type_size = sizeof(T);
		static const size_t block_size = std::is_same<T, sn_decimal_bit>::value ? T::size : 8 * type_size - static_cast<size_t>(std::is_signed<T>::value);
		static const size_t type_sign = std::is_same<T, sn_decimal_bit>::value ? T::sign : static_cast<size_t>(std::is_signed<T>::value);
		using block_max_t = typename std::conditional<std::is_same<T, sn_decimal_bit>::value, short, T>::type;
		static const block_max_t block_max =
		        std::is_same<T, sn_decimal_bit>::value ?
		        static_cast<block_max_t>(sn_decimal_bit::max)
		        : static_cast<block_max_t>(static_cast<long long>(1 << block_size) - 1);
		size_t length;
		sn_block_array() noexcept : capacity(0), length(0), blocks(nullptr) {
		}

		sn_block_array(unsigned int cap_) noexcept : capacity(cap_), length(0) {
			blocks = (capacity > 0) ? new T[capacity] : nullptr;
		}

		sn_block_array(const sn_block_array& rhs) noexcept : capacity(rhs.capacity), length(rhs.length) {
			blocks = (capacity > 0) ? new T[capacity] : nullptr;
			for (size_t i = 0; i < length; ++i)
				blocks[i] = rhs.blocks[i];
		}

		sn_block_array& operator= (const sn_block_array& rhs) noexcept {
			sn_digit_array(rhs);
			return *this;
		}

		~sn_block_array() {
			if (blocks)
				delete[] blocks;
		}

		void allocate_cap(size_t cap_) noexcept {
			if (cap_ > capacity) {
				if (blocks)
					delete[] blocks;
				blocks = new T[cap_];
				capacity = cap_;
			}
		}

		void allocate_and_copy(size_t cap_) noexcept {
			if (cap_ > capacity) {
				auto old_blocks = blocks;
				blocks = new T[cap_];
				for (size_t i = 0; i < length; ++i)
					blocks[i] = old_blocks[i];
				capacity = cap_;
				delete[] old_blocks;
			}
		}

		unsigned int get_capacity() const {
			return capacity;
		}

		unsigned int get_length() const {
			return length;
		}

		T& operator[] (unsigned int index) const {
			return blocks[index];
		}

		bool operator== (const sn_block_array& rhs) const {
			if (length != rhs.length)
				return false;
			for (unsigned int i = 0; i < length; ++i)
				if (blocks[i] != rhs.blocks[i])
					return false;
			return true;
		}

		bool operator!= (const sn_block_array& rhs) const {
			return !this->operator==(rhs);
		}


	};

	//TODO: add TBCO optimization
	template <typename T>
	class sn_unsigned_decimal {
	private:
		enum sn_number_string {
			dec, bin, oct, hex
		};
		size_t S = sn_block_array<T>::block_size;
		T M = sn_block_array<T>::block_max;
		using is_decimal_bit = std::is_same<T, sn_decimal_bit>;
	public:
		sn_block_array<T> block_arr;

		sn_unsigned_decimal() noexcept : block_arr() {
		}

		~sn_unsigned_decimal() = default;
		sn_unsigned_decimal(const sn_unsigned_decimal&) = default;
		sn_unsigned_decimal& operator= (const sn_unsigned_decimal&) = default;

		sn_unsigned_decimal(const std::string& str, sn_number_string format = sn_number_string::dec) {
			std::string trim_str = trim_zero(str);
			int idx = 0;
			switch (format) {
			case(sn_number_string::dec):
				block_arr.allocate_cap(trim_str.length());
				for (const auto& c : trim_str) {
					block_arr[idx] = c - '0';
					++idx;
				}
				break;
			case(sn_number_string::bin):
				break;
			case(sn_number_string::oct):
				break;
			case(sn_number_string::hex):
				break;
			default:
				SN_BASIC_LOG(std::cerr, std::string("Format not supported"));
				break;
			}
		}

		std::string trim_zero(const std::string& str) {
			bool zero_fl = true;
			for (auto it = str.begin(); it != str.end(); ++it) {
				if (*it != '0' && *it != 'x')
					return str.substr(it - str.begin(), str.length());
			}
			return "0";
		}

		template <typename I, typename = std::enable_if_t<std::is_integral<I>::value>>
		sn_unsigned_decimal(I&& x) {
			size_t integer_size = static_cast<std::size_t>(std::is_same<T, sn_decimal_bit>::value ? (floor(log10l(x)) + 1) : (floor(log2l(x)) + 1));
			size_t block_cnt = std::is_same<T, sn_decimal_bit>::value ? integer_size : integer_size / S + 1;
			block_arr.allocate_cap(block_cnt);
			block_arr.length = block_cnt;
			for (size_t i = 0; i < block_cnt; ++i) {
				block_arr[i] = std::is_same<T, sn_decimal_bit>::value ? (x % 10) : (x & M);
				std::is_same<T, sn_decimal_bit>::value ? (x /= 10) : (x >>= S);
			}
		}

		sn_unsigned_decimal<T> operator+ (const sn_unsigned_decimal<T>& rhs) {
			sn_unsigned_decimal<T> ret{};
			size_t block_cnt_max = block_arr.length > rhs.block_arr.length ? block_arr.length : rhs.block_arr.length;
			size_t block_cnt_min = block_arr.length < rhs.block_arr.length ? block_arr.length : rhs.block_arr.length;
			ret.block_arr.allocate_cap(block_cnt_max + 1);
			T block_last_left = 0;
			
			for (size_t i = 0; i < block_cnt_max + 1; ++i) {
				long long int lhs_v = (i < block_arr.length ? (long long)block_arr[i] : 0);
				long long int rhs_v = (i < rhs.block_arr.length ? (long long)rhs.block_arr[i] : 0);

				long long int block_cur_sum = lhs_v + rhs_v + block_last_left; //avoid overflow
				if (block_cur_sum > M) {
					block_cur_sum -= (M + 1);
					block_last_left = 1;
				}
				else
					block_last_left = 0;
				
				ret.block_arr[i] = static_cast<T>(block_cur_sum);
			}
			
			for (int i = block_cnt_max; i >= 0; --i) {
				if (ret.block_arr[i]) {
					bool not_zero = std::is_same<decltype(ret.block_arr[i]), sn_decimal_bit>::value ? (!ret.block_arr[i].is_zero()) : true;
					if (not_zero) {
						ret.block_arr.length = i + 1;
						break;
					}
				}
			}
			
			return ret;
		}

		//lhs > rhs
		sn_unsigned_decimal<T> operator- (const sn_unsigned_decimal<T>& rhs) {
			sn_unsigned_decimal<T> ret{};
			sn_unsigned_decimal<T> lhs_copy = *this;
			size_t block_cnt_max = block_arr.length > rhs.block_arr.length ? block_arr.length : rhs.block_arr.length;
			size_t block_cnt_min = block_arr.length < rhs.block_arr.length ? block_arr.length : rhs.block_arr.length;
			ret.block_arr.allocate_cap(block_cnt_max + 1);
			int block_last_left = 0;

			for (size_t i = 0; i < block_cnt_max + 1; ++i) {
				long long int lhs_v = (i < block_arr.length ? lhs_copy.block_arr[i] : 0);
				long long int rhs_v = (i < rhs.block_arr.length ? rhs.block_arr[i] : 0);
				long long int block_cur_sum = lhs_v - rhs_v;
				if (lhs_v > rhs_v) {
					ret.block_arr[i] = static_cast<T>(block_cur_sum);
				}
				else {
					block_cur_sum += (M + 1);
					size_t j = i + 1;
					while (lhs_copy.block_arr[j] == 0) {
						++j;
						lhs_copy.block_arr[j] = M;
					}
					--lhs_copy.block_arr[j];
					ret.block_arr[i] = static_cast<T>(block_cur_sum);
				}
			}

			for (int i = block_cnt_max; i >= 0; --i) {
				if (ret.block_arr[i]) {
					bool not_zero = std::is_same<decltype(ret.block_arr[i]), sn_decimal_bit>::value ? (!ret.block_arr[i].is_zero()) : true;
					if (not_zero) {
						ret.block_arr.length = i + 1;
						break;
					}
				}
			}

			return ret;
		}

		//TODO: FFT operator*
		//TODO: FFT operator/

		/* no member function partial specialization
		template <typename F, typename = std::enable_if_t<std::is_floating_point<F>::value>>
		sn_unsigned_decimal(F& x) {
			throw sn_Log::sn_Exception::NotImplementException;
		}
		*/

		

	};

	template <typename T>
	std::ostream& operator<< (std::ostream& out, const sn_unsigned_decimal<T>& dec) {
		out << std::dec;
		for (int i = dec.block_arr.length - 1; i >= 0; --i)
			out << dec.block_arr[i];
		return out;
	}

	template <typename T>
	class sn_decimal {
	private:
		enum sn_sign {
			negative = -1, zero = 0, positive = 1
		};
		sn_sign sign;
		sn_unsigned_decimal<T> numerical_part;
	};


}





#endif