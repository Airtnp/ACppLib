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
		using std::min;
		using std::max;
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
			virtual void set_size(len_t) = 0;
			virtual len_t get_position() const = 0;
			virtual void set_position(seek_t origin, len_t offset) = 0;
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

			virtual std::future<len_t> write_bytes_async(const byteptr_t data, len_t length) {
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
			StreamSpan(IR_ptr<IStream> stream, len_t start_pos, len_t end_pos) : m_internalStream{ stream }, m_startPos{ start_pos }, m_endPos{ end_pos }, m_currentPos{ start_pos } {
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
					std::memmove(m_data, data, static_cast<std::size_t>(length));
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
					auto storage = new byte_t[static_cast<std::size_t>(rhs.m_size)];
					std::swap(m_data, storage);
					delete[] storage;
				}

				if (rhs.m_data)
					std::memmove(m_data, rhs.m_data, static_cast<std::size_t>(rhs.m_size));

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

			len_t write_bytes(const byteptr_t data, len_t length) {
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

			std::future<len_t> write_bytes_async(const byteptr_t data, len_t length) {
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
				auto storage = new byte_t[static_cast<std::size_t>(cap)];
				const auto deleter = sn_Thread::scope_guard::make_scope([&storage] {
					delete[] storage;
				});

				if (m_size > 0 && m_data)
					std::memmove(storage, m_data, static_cast<std::size_t>(m_size));

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

		class FixedMemoryStream : public sn_Builtin::reference_counter::ReferenceCounter<IStream> {
		public:

			FixedMemoryStream(byteptr_t data, len_t size, bool readable, bool writable) noexcept : m_data{ data }, m_size{ size }, m_currentPos{ 0 }, m_readable{ readable }, m_writable{ writable } {}
			FixedMemoryStream(const byteptr_t data, len_t size, bool readable) noexcept : m_data{ const_cast<byteptr_t>(data) }, m_size{ size }, m_currentPos{ 0 }, m_readable{ readable }, m_writable{ false } {}

			template <std::size_t N>
			FixedMemoryStream(byte_t(&arr)[N], bool readable, bool writable) noexcept : MemoryStream(arr, N, readable, writable) {}
			template <std::size_t N>
			FixedMemoryStream(byte_t(&arr)[N], bool readable) noexcept : MemoryStream(arr, N, readable) {}

			~FixedMemoryStream() {}

			bool can_read() const {
				return m_readable;
			}

			bool can_write() const {
				return m_writable;
			}

			bool can_resize() const {
				return false;
			}

			bool can_seek() const {
				return true;
			}

			bool is_end_of_stream() const {
				return m_currentPos == m_size;
			}

			len_t get_size() const {
				return m_size;
			}

			void set_size(len_t size) {
				SN_LOG_ERROR_WTL(sn_Error::NotSupported, "Fixed stream cannot set size.");
			}

			len_t get_position() const {
				return m_currentPos;
			}

			void set_position(seek_t origin, len_t offset) {
				switch (origin) {
				case seek_t::begin:
					if (offset < 0 || m_size < offset)
						SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range.");
					m_currentPos = offset;
					break;
				case seek_t::current:
					if ((offset < 0 && m_currentPos + offset < 0) || m_size < offset + m_currentPos)
						SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range.");
					m_currentPos = m_currentPos + offset;
					break;
				case seek_t::end:
					if (offset > 0 || (m_size + offset < 0))
						SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Position out of range.");
					m_currentPos = m_size + offset;
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
				if (m_currentPos >= m_size)
					SN_LOG_ERROR_WTL(sn_Error::OutofRange, "Reached the end of stream.");
				m_data[m_currentPos++] = byte;
			}

			len_t write_bytes(const byteptr_t data, len_t length) {
				assert(m_data && "Data should not be nullptr.");
				len_t avail_write_len = 0;
				if (!m_writable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be written");
				if (length == 0)
					return avail_write_len;

				avail_write_len = std::min(length, m_size - m_currentPos);
				memmove(m_data + m_currentPos, data, static_cast<std::size_t>(avail_write_len));
				m_currentPos += avail_write_len;
				return avail_write_len;
			}

			std::future<len_t> write_bytes_async(const byteptr_t data, len_t length) {
				return std::async(std::launch::async, [=] {
					return write_bytes(data, length);
				});
			}

			void flush() {}

			byteptr_t get_internal_buffer() noexcept {
				return m_data;
			}

		private:
			const byteptr_t m_data;
			const len_t m_size;
			len_t m_currentPos;
			const bool m_readable;
			const bool m_writable;
		};

		class FileStream : public sn_Builtin::reference_counter::ReferenceCounter<IStream> {
		public:
#ifdef _WIN32
			using handle_t = HANDLE;
#else
			using handle_t = int;
#endif

#ifdef _WIN32
			FileStream(std::string filename, bool readable, bool writable, bool isasync = false, bool truncate = false) :
				m_mappedFile{ NULL }, m_dispose{ true }, m_isAsync{ isasync }, m_filename{ filename }, m_readable{ readable }, m_writable{ writable } {
				if (truncate && (readable || !writable))
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "File should be writable and not readable while truncated.");
				m_file = CreateFile(filename.c_str(),
					truncate ? GENERIC_WRITE : ((readable ? GENERIC_READ : 0) | (writable ? GENERIC_WRITE : 0)),
					FILE_SHARE_READ, NULL,
					truncate ? TRUNCATE_EXISTING : (writable ? OPEN_ALWAYS : OPEN_EXISTING),
					FILE_ATTRIBUTE_NORMAL | (isasync ? FILE_FLAG_OVERLAPPED : 0),
					NULL);

				if (!m_file || m_file == INVALID_HANDLE_VALUE)
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "Open file failed.");
			}

			FileStream(handle_t handle, bool readable, bool writable, bool transowner, bool isasync) :
				m_file{ handle }, m_mappedFile { NULL }, m_dispose{ transowner }, m_isAsync{ isasync }, m_readable{ readable }, m_writable{ writable } {
				
				if (!m_file || m_file == INVALID_HANDLE_VALUE)
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "Open file failed.");
			}

			len_t get_size() const {
				LARGE_INTEGER tmpSize{};
				if (!GetFileSizeEx(m_file, &tmpSize))
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "GetFileSizeEx failed.");
				return static_cast<len_t>(tmpSize.QuadPart);
			}

			void set_size(len_t size) {
				if (!m_writable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be written.");
				LARGE_INTEGER tVal;
				tVal.QuadPart = size;

				if (SetFilePointer(m_file, tVal.LowPart, &tVal.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "SetFilePointer failed.");
				if (SetEndOfFile(m_file) == FALSE)
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "SetEndOfFile failed.");
				
				set_position(seek_t::begin, 0);
			}

			len_t get_position() const {
				LARGE_INTEGER tVal;
				tVal.QuadPart = 0;

				tVal.LowPart = SetFilePointer(m_file, tVal.LowPart, &tVal.HighPart, FILE_CURRENT);
				return tVal.QuadPart;
			}

			void set_position(seek_t origin, len_t offset) {
				std::size_t tOrigin;
				switch (origin) {
				case seek_t::begin:
					tOrigin = FILE_BEGIN;
					break;
				case seek_t::current:
					tOrigin = FILE_CURRENT;
					break;
				case seek_t::end:
					tOrigin = FILE_END;
					break;
				default:
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Invalid origin.");
				}

				LARGE_INTEGER tVal;
				tVal.QuadPart = offset;

				if (SetFilePointer(m_file, tVal.LowPart, &tVal.HighPart, tOrigin) == INVALID_SET_FILE_POINTER)
					SN_LOG_ERROR_WTL(sn_Error::CheckFailed, "SetFilePointer failed.");
			}

			bool is_end_of_stream() const {
				return get_size() == get_position();
			}


			// data should be pre-allocated, byteptr_t data = new byte_t[size];
			len_t read_bytes(byteptr_t data, len_t length) {
				if (m_isAsync)
					return read_bytes_async(data, length).get();

				DWORD avail_read_len = 0;
				if (!m_readable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be read");
				if (length == 0)
					return avail_read_len;
				if (data == nullptr)
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Data cannot be nullptr.");

				if (FALSE == ReadFile(m_file, data, static_cast<DWORD>(length), &avail_read_len, NULL))
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, "ReadFile failed.");
				return avail_read_len;
			}

			len_t write_bytes(const byteptr_t data, len_t length) {
				if (m_isAsync)
					return write_bytes_async(data, length).get();

				DWORD avail_write_len = 0;
				if (!m_writable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be written");
				if (length == 0)
					return avail_write_len;
				if (data == nullptr)
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Data cannot be nullptr.");
				
				if (FALSE == WriteFile(m_file, data, static_cast<DWORD>(length), &avail_write_len, NULL))
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, "WriteFile failed.");
				return avail_write_len;
			}

			std::future<len_t> read_bytes_async(byteptr_t data, len_t length) {
				if (m_isAsync) {
					if (!m_readable)
						SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be read");
					if (length == 0) {
						std::promise<len_t> dummy;
						dummy.set_value(0);
						return dummy.get_future();
					}
					if (data == nullptr)
						SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Data cannot be nullptr.");

					auto olp = std::make_unique<OVERLAPPED>();

					if (FALSE == ReadFile(m_file, data, static_cast<DWORD>(length), NULL, olp.get())) {
						auto lastError = GetLastError();
						if (lastError != ERROR_IO_PENDING) {
							SN_LOG_ERROR_WTL(sn_Error::APIFailed, "ReadFile failed.");
						}
					}

					return std::async(std::launch::deferred, [olp = move(olp), this]()->len_t {
						DWORD avail_read_len;
						if (!GetOverlappedResult(m_file, olp.get(), &avail_read_len, TRUE))
							SN_LOG_ERROR_WTL(sn_Error::APIFailed, "GetOverlappedResult failed.");
						return avail_read_len;
					});
				}

				return std::async(std::launch::async, [=] {
					return read_bytes(data, length);
				});

			}

			std::future<len_t> write_bytes_async(const byteptr_t data, len_t length) {
				if (m_isAsync) {
					if (!m_writable)
						SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be written");
					if (length == 0) {
						std::promise<len_t> dummy;
						dummy.set_value(0);
						return dummy.get_future();
					}
					if (data == nullptr)
						SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Data cannot be nullptr.");

					auto olp = std::make_unique<OVERLAPPED>();

					if (FALSE == WriteFile(m_file, data, static_cast<DWORD>(length), NULL, olp.get())) {
						auto lastError = GetLastError();
						if (lastError != ERROR_IO_PENDING) {
							SN_LOG_ERROR_WTL(sn_Error::APIFailed, "ReadFile failed.");
						}
					}

					return std::async(std::launch::deferred, [olp = move(olp), this]()->len_t {
						DWORD avail_write_len;
						if (!GetOverlappedResult(m_file, olp.get(), &avail_write_len, TRUE))
							SN_LOG_ERROR_WTL(sn_Error::APIFailed, "GetOverlappedResult failed.");
						return avail_write_len;
					});
				}

				return std::async(std::launch::async, [=] {
					return write_bytes(data, length);
				});

			}

			void flush() {
				if (m_mappedStream)
					FlushViewOfFile(m_mappedStream->get_internal_buffer(), static_cast<SIZE_T>(get_size()));
				FlushFileBuffers(m_file);
			}

			IR_ptr<FixedMemoryStream> map_to_memory_stream() {
				if (m_mappedStream)
					return m_mappedStream;
				m_mappedFile = CreateFileMapping(m_file, NULL,
					m_writable ? PAGE_READWRITE : PAGE_READONLY,
					NULL, NULL, NULL);
				if (!m_mappedFile || m_mappedFile == INVALID_HANDLE_VALUE)
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, "CreateFileMapping failed.");

				auto pFile = MapViewOfFile(m_mappedFile,
					(m_readable ? FILE_MAP_READ : 0) | (m_writable ? FILE_MAP_WRITE : 0),
					NULL, NULL, NULL);
				if (!pFile)
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, "MapViewOfFile failed.");

				m_mappedStream = sn_Builtin::reference_counter::make_ref_ptr<FixedMemoryStream>(static_cast<byteptr_t>(pFile), get_size(), m_readable, m_writable);

				return m_mappedStream;
			}

			~FileStream() {
				CloseHandle(m_mappedFile);
				if (m_dispose)
					CloseHandle(m_file);
			}
#else
			FileStream(std::string filename, bool readable, bool writable, bool truncate) :
				m_file{}, m_dispose{ true }, m_isEndOfFile{}, m_filename{ filename }, m_readable{ readable }, m_writable{ writable } {
				int mode = 0;
				if (readable && writable)
					mode = O_RDWR;
				else if (readable)
					mode = O_RDONLY;
				else
					mode = O_WRONLY | O_CREAT;

				if (truncate)
					mode |= O_TRUNC;

				m_file = open(filename.c_str(), mode, S_IRWXU | S_IRWXG | S_IRWXO);
				if (!m_file)
					SN_LOG_ERROR_WTL(sn_Error::InternalError, "Cannot open file.");
			}

			FileStream(handle_t file, bool readable, bool writable, bool transferowner) :
				m_file{ file }, m_dispose{ transferowner }, m_isEndOfFile{}, m_filename{ filename }, m_readable{ readable }, m_writable{ writable } {
				if (m_file < 0)
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "File descriptor should be positive");
			}

			bool is_end_of_stream() const {
				return m_isEndOfFile;
			}

			len_t get_size() const {
				const auto currentPos = lseek(m_file, 0, SEEK_CUR);
				lseek(m_file, 0, SEEK_SET);
				const auto beginPos = lseek(m_file, 0, SEEK_CUR);
				lseek(m_file, 0, SEEK_END);
				const auto totalSize = lseek(m_file, 0, SEEK_CUR) - beginPos;
				lseek(m_file, currentPos, SEEK_SET);
				return static_cast<len_t>(totalSize);
			}

			void set_size(len_t size) {
				if (!m_writable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be writtern.");
				if (ftruncate(m_file, static_cast<off_t>(size)))
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, sn_Log::logstream::Fmt("ftruncate failed (errno = %d).", errno));
			}

			len_t get_position() const {
				return static_cast<len_t>(lseek(m_file, 0, SEEK_CUR));
			}

			len_t set_position(seek_t origin, len_t offset) {
				int tOrigin;
				switch (origin) {
				case seek_t::begin:
					tOrigin = SEEK_SET;
					break;
				case seek_t::current:
					tOrigin = SEEK_CUR;
					break;
				case seek_t::end:
					tOrigin = SEEK_END;
					break;
				default:
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Invalid origin.");
				}
				lseek(m_file, static_cast<off_t>(offset), tOrigin);
			}

			len_t read_bytes(byteptr_t data, len_t length) {
				if (!m_readable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be read");
				if (length == 0)
					return 0;
				if (data == nullptr)
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Data cannot be nullptr.");
				const auto ret = read(m_file, data, static_cast<std::size_t>(length));
				if (ret < 0)
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, "read file failed.");

				m_isEndOfFile = !ret;
				return static_cast<len_t>(ret);
			}

			len_t write_bytes(const byteptr_t data, len_t length) {
				if (!m_writable)
					SN_LOG_ERROR_WTL(sn_Error::IllegalState, "Stream cannot be written");
				if (length == 0)
					return 0;
				if (data == nullptr)
					SN_LOG_ERROR_WTL(sn_Error::InvalidArgs, "Data cannot be nullptr.");
				const auto ret = write(m_file, data, static_cast<std::size_t>(length));
				if (ret < 0)
					SN_LOG_ERROR_WTL(sn_Error::APIFailed, "write file failed.");

				return static_cast<len_t>(ret);
			}
		
			void flush() {}

			~FileStream() {
				if (m_dispose)
					close(m_file);
			}

#endif
			bool can_write() const {
				return m_writable;
			}

			bool can_read() const {
				return m_readable;
			}

			bool can_resize() const {
				return m_writable;
			}

			bool can_seek() const {
				return true;
			}

			
			virtual byte_t read_byte() {
				byte_t byte;
				if (read_bytes(&byte, 1) == 1)
					return byte;
				SN_LOG_ERROR_WTL(sn_Error::InternalError, "Unable to read byte.");
			}

			virtual void write_byte(byte_t byte) {
				if (write_bytes(&byte, 1) != 1)
					SN_LOG_ERROR_WTL(sn_Error::InternalError, "Unable to write byte.");
			}

			std::string get_filename() const noexcept {
				return m_filename;
			}

			handle_t get_handle() const noexcept {
				return m_file;
			}


		private:
			handle_t m_file;
			const bool m_dispose;
			std::string m_filename;
			bool m_readable;
			bool m_writable;

#ifdef _WIN32
			handle_t m_mappedFile;
			IR_ptr<FixedMemoryStream> m_mappedStream;
			const bool m_isAsync;
#else
			bool m_isEndofFile;
#endif

		};

	}
}






#endif