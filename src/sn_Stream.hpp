#ifndef SN_STREAM_H
#define SN_STREAM_H

#ifndef _CRT_SECURE_NO_WARNING
#define _CRT_SECURE_NO_WARNING // C-style memmove
#endif

#include "sn_CommonHeader.h"
#include "sn_Builtin.hpp"
#include "sn_Log.hpp"
#include "sn_Thread.hpp"

namespace sn_Stream {
	namespace stream {
		using sn_Builtin::reference_counter::IR_ptr;
		using sn_Builtin::reference_counter::IWR_ptr;
		enum class seek_t {
			begin, current, end,
		};

		//TODO: deal with offset overflow;
		class IStream : public sn_Builtin::reference_counter::IReferenceCounter {
		public:
			enum {
				defaultCopyBufferSize = 1024,
			};

			using len_t = uint64_t;
			using byte_t = uint8_t;
			using byteptr_t = observer_ptr<byte_t>;
			using length_type = uint64_t;
			using byte_type = uint8_t;

			virtual ~IStream() {}

			virtual bool can_write() const = 0;
			virtual bool can_read() const = 0;
			virtual bool can_resize() const = 0;
			virtual bool can_seek() const = 0;
			virtual bool is_end_of_stream() const = 0;
			virtual len_t get_size() const = 0;
			virtual void set_size(len_t) const = 0;
			virtual len_t get_position() const = 0;
			virtual void set_position(seek_t origin, len_t offset) const = 0;
			virtual void flush() = 0;

			virtual len_t read_bytes(byteptr_t data, len_t length) = 0;
			virtual len_t write_bytes(const byteptr_t data, len_t length) = 0;

			virtual byte_t read_byte() {
				byte_t byte;
				if (read_bytes(&byte, 1) == 1)
					return byte;
				SN_LOG_ERROR_WITH_THROW("Unable to read byte.");
			}

			virtual void force_read_bytes(byteptr_t data, len_t length) {
				len_t total_bytes_len{ 0 };
				do {
					const auto current_bytes_len = read_bytes(data + total_bytes_len, length - total_bytes_len);
					if (!current_bytes_len)
						SN_LOG_ERROR_WITH_THROW("Unexpected end of stream");
					total_bytes_len += current_bytes_len;
				} while (total_bytes_len < length);
			}

			virtual std::future<len_t> read_bytes_async(byteptr_t data, len_t length) {
				return std::async(std::launch::async, [=] {
					return read_bytes(data, length);
				});
			}

			virtual void write_byte(byte_t byte) {
				if (write_bytes(&byte, 1) != 1)
					SN_LOG_ERROR_WITH_THROW("Unable to write byte.");
			}

			virtual void force_write_bytes(const byteptr_t data, len_t length) {
				len_t total_bytes_len{ 0 };
				do {
					const auto current_bytes_len = write_bytes(data + total_bytes_len, length - total_bytes_len);
					if (!current_bytes_len)
						SN_LOG_ERROR_WITH_THROW("Unexpected end of stream");
					total_bytes_len += current_bytes_len;
				} while (total_bytes_len < length);
			}

			virtual std::future<len_t> write_bytes_async(byteptr_t data, len_t length) {
				return std::async(std::launch::async, [=] {
					return write_bytes(data, length);
				});
			}

			virtual len_t copy_to(const IR_ptr<IStream> rhs) {
				assert(rhs && "Copy source should be not nullptr");

				byte_t buffer[defaultCopyBufferSize];
				len_t read_bytes_len{ 0 }, total_bytes_len{ 0 };
				while (true) {
					read_bytes_len = read_bytes(buffer, sizeof(buffer));
					total_bytes_len += read_bytes_len;
					if (!read_bytes_len)
						break;
					rhs->write_bytes(buffer, read_bytes_len);
				}
				return total_bytes_len;
			}
		};
		
		class StreamSpan : public sn_Builtin::reference_counter::ReferenceCounter<IStream> {
		public:
			StreamSpan(IR_ptr<IStream> stream, len_t start_pos, len_t end_pos) : m_interalStream{ stream }, m_startPos{ start_pos }, m_endPos{ end_pos }, m_currentPos{ start_pos } {
				if (!m_internalStream->can_seek())
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Should be seekable stream.");
				if (start_pos > end_pos)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Start position should have operator<= with end position");
				if (m_internalStream->get_size() < end_pos)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Stream size should be less than end position");
				m_internalStream->set_position(seek_t::begin, start_pos);
			} 
			~StreamSpan() {}

			IR_ptr<IStream> get_underlying_stream() const noexcept {
				return m_internalStream;
			}

			bool can_write() const {
				return m_internalStream->can_write();
			}

			bool can_read() const {
				return m_internalStream->can_read();
			}

			bool can_seek() const {
				return m_internalStream->can_seek();
			}

			bool can_resize() const {
				return m_internalStream->can_resize();
			}

			bool is_end_of_stream() const {
				return m_internalStream->is_end_of_stream();
			}

			len_t get_size() const {
				return m_endPos - m_startPos;
			}

			void set_size(len_t size) const {
				SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Span of stream can't set size.");
			}

			len_t get_position() const {
				return m_currentPos;
			}

			len_t get_seek_position(seek_t origin) const {
				switch (origin) {
				case seek_t::begin:
					return m_startPos;
					break;
				case seek_t::current:
					return m_currentPos;
					break;
				case seek_t::end:
					return m_endPos;
					break;
				default:
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Invalid seek argument.");
				}
			}

			void set_position(seek_t origin, len_t offset) {
				len_t pos{ 0 };
				pos = get_seek_position(origin) + offset;
				if (pos < m_startPos || pos > m_endPos)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range");
				m_currentPos = pos;
				adjust_position();
			}

			byte_t read_byte() {
				if (!m_internalStream->can_read())
					SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Underlying stream cannot read.");
				if (m_currentPos >= m_endPos)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Reaches the end of stream.");
				adjust_position();
				return m_internalStream->read_byte();
			}

			len_t read_bytes(byteptr_t data, len_t length) {
				if (!m_internalStream->can_read())
					SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Underlying stream cannot read.");
				const auto can_read_len = std::min(length, m_endPos - m_startPos);
				adjust_position();
				return m_internalStream->read_bytes(data, can_read_len);
			}

			std::future<len_t> read_bytes_async(byteptr_t data, len_t length) {
				if (!m_internalStream->can_read())
					SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Underlying stream cannot read.");
				const auto can_read_len = std::min(length, m_endPos - m_startPos);
				adjust_position();
				return m_internalStream->read_bytes_async(data, can_read_len);
			}

			void write_byte(byte_t byte) {
				if (!m_internalStream->can_write())
					SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Underlying stream cannot write.");
				if (m_currentPos >= m_endPos)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Reaches the end of stream.");
				adjust_position();
				m_internalStream->write_byte(byte);
			}

			len_t write_bytes(const byteptr_t data, len_t length) {
				if (!m_internalStream->can_write())
					SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Underlying stream cannot write.");
				const auto can_write_len = std::min(length, m_endPos - m_startPos);
				adjust_position();
				return m_internalStream->write_bytes(data, can_write_len);
			}

			std::future<len_t> write_bytes_async(const byteptr_t data, len_t length) {
				if (!m_internalStream->can_write())
					SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Underlying stream cannot write.");
				const auto can_write_len = std::min(length, m_endPos - m_startPos);
				adjust_position();
				return m_internalStream->write_bytes_async(data, can_write_len);
			}

			void flush() {
				m_internalStream->flush();
			}

		private:
			const IR_ptr<IStream> m_internalStream;
			const len_t m_startPos;
			const len_t m_endPos;
			len_t m_currentPos;

			void adjust_position() {
				assert(m_currentPos >= m_startPos && m_currentPos <= m_endPos);
				if (m_currentPos == m_internalStream->get_position())
					return;
				//deal with overflow;
				m_internalStream->set_position(seek_t::begin, m_currentPos);
			}

			void check_position() {
				if (m_currentPos < m_startPos || m_currentPos > m_endPos)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Current position is out of range.");
				if (m_currentPos != m_internalStream->get_position())
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Current position doesn't equal the real position.");
			}
		};

		class MemoryStream : public sn_Builtin::reference_counter::ReferenceCounter<IStream> {
		public:
			MemoryStream(const byteptr_t data, len_t length, bool readable, bool writable, bool autoresize) noexcept :
				m_data{ nullptr }, m_currentPos{ 0 }, m_size{ 0 }, m_capacity{ 0 },
				m_readable{ readable }, m_writable{ writable }, m_autoResize{ autoresize } {
				reserve(length);
				m_size = length;
				if (data)
					std::memmove(m_data, data, length);
			}
			MemoryStream(len_t length, bool readable, bool writable, bool autoresize) noexcept : MemoryStream(nullptr, length, readable, writable, autoresize) {}
			
			MemoryStream(const MemoryStream& rhs) noexcept : MemoryStream(nullptr, 0, false, false, false) {
				*this = rhs;
			}
			MemoryStream& operator=(const MemoryStream& rhs) noexcept {
				if (this == &rhs)
					return *this;
				std::lock_guard<std::recursive_mutex> self_lock{ m_mutex };
				std::lock_guard<std::recursive_mutex> rhs_lock{ rhs.m_mutex };

				if (rhs.m_size >= m_capacity) {
					auto storage = new byte_t[rhs.m_size];
					std::swap(m_data, storage);
					delete[] storage;
				}

				if (rhs.m_data)
					memmove(m_data, rhs.m_data, rhs.m_size);

				m_size = rhs.m_size;
				m_capacity = rhs.m_capacity;
				m_currentPos = rhs.m_currentPos;
				m_readable = rhs.m_readable;
				m_writable = rhs.m_writable;
				m_autoResize = rhs.m_autoResize;
				
				return *this;
			}
			MemoryStream(const MemoryStream&& rhs) noexcept : MemoryStream(nullptr, 0, false, false, false) {
				*this = std::move(rhs);
			}
			MemoryStream& operator=(MemoryStream&& rhs) noexcept {
				if (this == &rhs)
					return *this;
				std::lock_guard<std::recursive_mutex> self_lock{ m_mutex };
				std::lock_guard<std::recursive_mutex> rhs_lock{ rhs.m_mutex };

				m_data = std::move(rhs.m_data);
				m_size = std::move(rhs.m_size);
				m_capacity = std::move(rhs.m_capacity);
				m_currentPos = std::move(rhs.m_currentPos);
				m_readable = std::move(rhs.m_readable);
				m_writable = std::move(rhs.m_writable);
				m_autoResize = std::move(rhs.m_autoResize);

				return *this;
			}

			MemoryStream(MemoryStream&& rhs) noexcept : MemoryStream(nullptr, 0, false, false, false) {
				*this = std::move(rhs);
			}

			bool can_read() const {
				return m_readable;
			}

			bool can_write() const {
				return m_writable;
			}

			bool can_resize() const {
				return true;
			}

			bool can_seek() const {
				return true;
			}

			bool is_end_of_stream() const {
				return m_currentPos >= m_capacity;
			}

			len_t get_size() const {
				return m_size;
			}

			void set_size(len_t size) {
				reserve(size);
				m_size = size;
				m_currentPos = std::min(m_currentPos, m_size);
			}

			len_t get_position() const {
				return m_currentPos;
			}

			void set_position(seek_t origin, len_t offset) {
				switch (origin) {
				case seek_t::begin:
					if (offset < 0 || m_capacity < offset)
						SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range.");
					m_currentPos = offset;
					break;
				case seek_t::current:
					if ((offset < 0 && m_currentPos + offset < 0) || m_capacity < offset + m_currentPos)
						SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range.");
					m_currentPos = m_currentPos + offset;
					break;
				case seek_t::end:
					if (offset > 0 || (m_capacity + offset < 0))
						SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range.");
					m_currentPos = m_capacity + offset;
					break;
				default:
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Invalid seek argument.");
				}
			}

			byte_t read_byte() {
				if (m_currentPos >= m_size)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Current position out of range.");
				return m_data[m_currentPos++];
			}

			len_t read_bytes(byteptr_t data, len_t length) {
				assert(m_data && "Data should not be nullptr.");
				len_t avail_read_len = 0;
				if (!m_readable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be read");
				if (length == 0)
					return avail_read_len;

				guard_t guard(m_mutex);
				avail_read_len = std::min(length, m_size - m_currentPos);
				memmove(data + m_currentPos, m_data, static_cast<std::size_t>(avail_read_len));
				m_currentPos += avail_read_len;
				return avail_read_len;
			}

			std::future<len_t> read_bytes_async(byteptr_t data, len_t length) {
				return std::async(std::launch::async, [=] {
					return read_bytes(data, length);
				});
			}

			void write_byte(byte_t byte) {
				if (m_size >= m_capacity)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Stream cannot be larger than capacity.");
				if (m_currentPos >= m_capacity)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Reached the end of stream.");
				m_data[m_currentPos++] = byte;
				m_size = std::max(m_currentPos, m_size);
			}

			len_t write_bytes(byteptr_t data, len_t length) {
				assert(m_data && "Data should not be nullptr.");
				len_t avail_write_len = 0;
				if (!m_writable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be written");
				if (length == 0)
					return avail_write_len;

				guard_t guard(m_mutex);
				if (length > m_capacity - m_currentPos) {
					if (m_autoResize) {
						reserve(length * 2);
						avail_write_len = length;
					}
					else {
						avail_write_len = m_capacity - m_currentPos;
					}
				}
				memmove(m_data + m_currentPos, data, static_cast<std::size_t>(avail_write_len));
				m_currentPos += avail_write_len;
				m_size = std::max(m_currentPos, m_size);
				return avail_write_len;
			}

			std::future<len_t> write_bytes_async(byteptr_t data, len_t length) {
				return std::async(std::launch::async, [=] {
					return write_bytes(data, length);
				});
			}

			void flush() {}

			byteptr_t get_internal_buffer() noexcept {
				return m_data;
			}

			const byteptr_t get_internal_buffer() const noexcept {
				return m_data;
			}

			len_t get_capacity() const noexcept {
				return m_capacity;
			}

			~MemoryStream() {
				if (m_data)
					delete[] m_data;
			}

			void reserve(len_t cap) {
				if (cap > 0 && cap <= m_capacity)
					return;
				m_capacity = cap;
				auto storage = new byte_t[cap];
				const auto deleter = sn_Thread::scope_guard::make_scope([&storage] {
					delete[] storage;
				});

				if (m_size > 0 && m_data)
					std::memmove(storage, m_data, m_size);

				std::swap(m_data, storage);
			}


		
		private:
			mutable std::recursive_mutex m_mutex;
			using guard_t = std::lock_guard<std::recursive_mutex>;
			byteptr_t m_data;
			len_t m_size;
			len_t m_capacity;
			len_t m_currentPos;
			bool m_readable;
			bool m_writable;
			bool m_autoResize;
		};
	}
}






#endif