#ifndef SN_BINARY_READER_H
#define SN_BINARY_READER_H


#include "../sn_CommonHeader.h"
#include "../sn_Builtin.hpp"
#include "../sn_Stream.hpp"
#include "../sn_Log.hpp"
#include "Endian.hpp"

namespace sn_Binary {
    
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
				if ((read_bytes = m_stream->read_bytes(reinterpret_cast<byteptr_t>(&obj), sizeof(T))) < sizeof(T)) {
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
}


#endif