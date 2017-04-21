#ifndef SN_BINARY_H
#define SN_BINARY_H

#include "sn_CommonHeader.h"
#include "sn_Builtin.hpp"
#include "sn_Stream.hpp"
#include "sn_Log.hpp"

namespace sn_Binary {
	//ref: https://github.com/akemimadoka/NatsuLib
	namespace Endian {
		enum class ByteEndian {
			BigEndian,
			LittleEndian,
			MiddleEndian, //Not supported (I am shocked to know it exists!)
		};

		ByteEndian getEndian() {
			static constexpr uint16_t s_short = 0x1234;
			static const auto s_endian = reinterpret_cast<const uint8_t*>(&s_short)[0] == 0x12 ? ByteEndian::BigEndian : ByteEndian::LittleEndian;
			return s_endian;
		}
		using len_t = uint64_t;
		using byte_t = uint8_t;
		using byteptr_t = observer_ptr<byte_t>;

		inline void swapEndian(byteptr_t data, std::size_t n) {
			using std::swap;
			const auto max = n - 1;
			const auto mid = n / 2;
			for (std::size_t i = 0; i < mid; ++i) {
				swap(data[i], data[max - i]);
			}
		}
	}

	namespace reader {
		using namespace sn_Builtin::reference_counter;
		using namespace sn_Stream::stream;
		using sn_Stream::stream::IStream;
		class BinaryReader : public ReferenceCounter<IReferenceCounter> {
		public:
			using len_t = IStream::len_t;
			using byte_t = IStream::byte_t;
			using byteptr_t = IStream::byteptr_t;
			explicit BinaryReader(IR_ptr<IStream> stream, Endian::ByteEndian env = Endian::getEndian()) noexcept
				: m_stream(std::move(stream)), m_endian(env), m_needSwapEndian(env != Endian::getEndian()) {}
			~BinaryReader() {}

			IR_ptr<IStream> get_underlying_stream() const noexcept {
				return m_stream;
			}
			Endian::ByteEndian get_endian() const noexcept {
				return m_endian;
			}
			template <typename T>
			std::enable_if_t<std::is_pod<T>::value, T> read_pod() {
				T ret;
				read_pod(ret);
				return ret;
			}
			template <typename T>
			std::enable_if_t<std::is_pod<T>::value> read_pod(T& obj) {
				len_t read_bytes;
				if ((read_bytes = m_stream->read_bytes(reinterpret_cast<byteptr_t>(&obj))) < sizeof(T)) {
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "Cannot read sizeof(T) bytes.");
				}
				if (m_needSwapEndian) {
					Endian::swapEndian(reinterpret_cast<byteptr_t>(&obj), sizeof(T));
				}
			}
			void skip(len_t bytes) {
				if (!bytes)
					return;
				if (m_stream->can_seek())
					m_stream->set_position(seek_t::current, bytes);
				else {
					auto remain = bytes;
					std::vector<byte_t> buffer(std::min(remain, static_cast<len_t>(std::numeric_limits<std::size_t>::max())));
					while (remain) {
						m_stream->read_bytes(buffer.data(), std::min(remain, static_cast<len_t>(buffer.size())));
						remain -= buffer.size();
					}
				}

			}
		private:
			IR_ptr<IStream> m_stream;
			const Endian::ByteEndian m_endian;
			const bool m_needSwapEndian;
		};
	}
	namespace writer {
		using namespace sn_Builtin::reference_counter;
		using namespace sn_Stream::stream;
		using sn_Stream::stream::IStream;
		class BinaryWriter : public ReferenceCounter<IReferenceCounter> {
		public:
			using len_t = IStream::len_t;
			using byte_t = IStream::byte_t;
			using byteptr_t = IStream::byteptr_t;
			explicit BinaryWriter(IR_ptr<IStream> stream, Endian::ByteEndian env = Endian::getEndian()) noexcept
				: m_stream(std::move(stream)), m_endian(env), m_needSwapEndian(env != Endian::getEndian()) {}
			~BinaryWriter() {}

			IR_ptr<IStream> get_underlying_stream() const noexcept {
				return m_stream;
			}
			Endian::ByteEndian get_endian() const noexcept {
				return m_endian;
			}
			template <typename T>
			std::enable_if_t<std::is_pod<T>::value, T> write_pod(const T& obj) {
				len_t write_bytes;
				auto p = reinterpret_cast<const byteptr_t>(&obj);
				byte_t buffer[sizeof(T)];
				if (m_NeedSwapEndian)
				{
#ifdef _MSC_VER
					const auto copyIterator = stdext::make_checked_array_iterator(buffer, std::size(buffer));
#else
					const auto copyIterator = buffer;
#endif
					std::reverse_copy(p, p + sizeof(T), copyIterator);
					pRead = buffer;
				}
				if ((write_bytes = m_stream->write_bytes(p, sizeof(T)) < sizeof(T))) {
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "Cannot write sizeof(T) bytes.");
				}
			}
		private:
			IR_ptr<IStream> m_stream;
			const Endian::ByteEndian m_endian;
			const bool m_needSwapEndian;
		};
	}
}


#endif