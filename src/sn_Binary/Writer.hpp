#ifndef SN_BINARY_WRITER_H
#define SN_BINARY_WRITER_H


#include "../sn_CommonHeader.h"
#include "../sn_Builtin.hpp"
#include "../sn_Stream.hpp"
#include "../sn_Log.hpp"
#include "Endian.hpp"

namespace sn_Binary {
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