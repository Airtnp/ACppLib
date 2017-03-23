#ifndef SN_LOG_H
#define SN_LOG_H

#define _CRT_SECURE_NO_WARNING  //C-style time

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Macro.hpp"

//ref: https://github.com/chenshuo/muduo/base
//TODO: add timezone
//TODO: add Asynclogging
//Why ctor every time? MT-Safety
namespace sn_Log {

#define SN_BASIC_LOG(out_stream, ...) \
	auto MACRO_CONCAT(end, __LINE__) = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); \
	out_stream << "[" << std::put_time(std::localtime((&MACRO_CONCAT(end, __LINE__))), "%c UTC%z") << "]" << std::endl \
			   << "\tFile: " << __FILE__ << std::endl \
			 	<< "\tFunction: " << __FUNCTION__ \
				 << " at line " << __LINE__ << std::endl \
				  << "\tInfo: " << (__VA_ARGS__) << std::endl


	namespace timestamp {
		class TimeStamp : sn_Assist::sn_less_operator::less_than<TimeStamp> {
		public:
			TimeStamp() noexcept : microSecondsSinceEpoch_(0) {}
			TimeStamp(int64_t microSecondsSinceEpoch__) noexcept : microSecondsSinceEpoch_(microSecondsSinceEpoch__) {}
		
			static const int kMicroSecondsPerSecond = 1000 * 1000;

			static TimeStamp fromUnixTime(time_t t, int microseconds) {
				return TimeStamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
			}

			static TimeStamp fromUnixTime(time_t t) {
				return fromUnixTime(t, 0);
			}

			int64_t microSecondsSinceEpoch() const {
				return microSecondsSinceEpoch_;
			}

			bool operator<(const TimeStamp& rhs) const {
				return microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch();
			}

			bool operator==(const TimeStamp& rhs) const {
				return microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch();
			}

			std::string toString() const {
				char buf[32] = { 0 };
				int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
				int64_t microSeconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
				snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microSeconds);
				return buf;
			}

			std::string toFormattedString(bool showMicroSeconds = true) const {
				char buf[32] = { 0 };
				std::time_t seconds = static_cast<std::time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
				struct std::tm* tm_time = std::gmtime(&seconds);
				if (showMicroSeconds) {
					int microSeconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
					snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
						tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
						tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, microSeconds);
				}
				else {
					snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
						tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
						tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
				}
				return buf;
			}

			static TimeStamp invalid() {
				return TimeStamp();
			}

			static TimeStamp now() {
				std::time_t tm_now = time(nullptr);
				return TimeStamp(static_cast<int64_t>(tm_now) * kMicroSecondsPerSecond);
			}
		
		private:
			int64_t microSecondsSinceEpoch_;
		};
	}

	namespace logstream {

		class Fmt {
		public:
			template<typename T>
			Fmt(const char* fmt, T val) {
				static_assert(std::is_arithmetic<T>::value, "Not arithmetic value");
				length_ = snprintf(buf_, sizeof(buf_), fmt, val);
				assert(length_ < sizeof(buf_));
			}

			const char* data() const {
				return buf_;
			}

			std::size_t length() const {
				return length_;
			}

		private:
			char buf_[32];
			std::size_t length_;
		};
		
		const std::size_t kSmallBuffer = 4000;
		const std::size_t kLargeBuffer = 4000 * 1000;

		template <std::size_t SIZE>
		class FixedBuffer {
		public:
			FixedBuffer() : cur_(data_) {
				setCookie(cookieStart);
			}

			~FixedBuffer() {
				setCookie(cookieEnd);
			}

			void append(const char* buf, std::size_t len) {
				if (avail() > len) {
					memcpy(cur_, buf, len);
					cur_ += len;
				}
			}

			using CookieFunc = void(*)();

			void setCookie(CookieFunc fun) {
				cookie_ = fun;
			}

			void add(std::size_t len) {
				cur_ += len;
			}

			const char* data() const {
				return data_;
			}

			std::size_t length() const {
				return static_cast<std::size_t>(cur_ - data_);
			}

			std::size_t avail() const {
				return static_cast<std::size_t>(end() - cur_);
			}

			char* current() {
				return cur_;
			}

			void reset() {
				cur_ = data_;
			}

			void szero() {
				memset(data_, 0, sizeof(data_));
			}

			std::string toString() const {
				return std::string(data_, length());
			}


		private:
			const char* end() const {
				return data_ + sizeof(data_);
			}

			static void cookieStart() {}
			static void cookieEnd() {}


			void(*cookie_)();
			char data_[SIZE];
			char* cur_;

		};

		class LogStream {
			using self = LogStream;
		public:
			using Buffer = FixedBuffer<kSmallBuffer>;
			LogStream& operator<<(bool v) {
				buffer_.append(v ? "1" : "0", 1);
				return *this;
			}
			LogStream& operator<<(short v) {
				*this << static_cast<int>(v);
				return *this;
			}
			LogStream& operator<<(unsigned short v) {
				*this << static_cast<unsigned int>(v);
				return *this;
			}
			LogStream& operator<<(int v) {
				formatInteger(v);
				return *this;
			}
			LogStream& operator<<(unsigned int v) {
				formatInteger(v);
				return *this;
			}
			LogStream& operator<<(long v) {
				formatInteger(v);
				return *this;
			}
			LogStream& operator<<(unsigned long v) {
				formatInteger(v);
				return *this;
			}
			LogStream& operator<<(long long v) {
				formatInteger(v);
				return *this;
			}
			LogStream& operator<<(unsigned long long v) {
				formatInteger(v);
				return *this;
			}
			LogStream& operator<<(const void* p) {
				std::uintptr_t v = reinterpret_cast<uintptr_t>(p);
				if (buffer_.avail() >= kMaxNumericSize) {
					char* buf = buffer_.current();
					buf[0] = '0';
					buf[1] = 'x';
					std::size_t len = convertHex(buf + 2, v);
					buffer_.add(len + 2);
				}

				return *this;
			}

			LogStream& operator<<(double v) {
				if (buffer_.avail() >= kMaxNumericSize) {
					std::size_t len = snprintf(buffer_.current(), kMaxNumericSize, "%12g", v);
					buffer_.add(len);
				}
				return *this;
			}

			LogStream& operator<<(char v) {
				buffer_.append(&v, 1);
				return *this;
			}

			LogStream& operator<<(const char* str) {
				if (str)
					buffer_.append(str, strlen(str));
				else
					buffer_.append("(null)", 6);
				return *this;
			}

			LogStream& operator<<(const unsigned char* str) {
				return operator<<(reinterpret_cast<const char*>(str));
			}

			LogStream& operator<<(const std::string& v) {
				buffer_.append(v.c_str(), v.size());
				return *this;
			}

			LogStream& operator<<(const Buffer& v) {
				*this << v.toString();
				return *this;
			}

			LogStream& operator<<(const Fmt& fmt) {
				append(fmt.data(), fmt.length());
				return *this;
			}

			/*
			LogStream& operator<<(const logger::Logger::SourceFile& v) {
				append(v.data_, v.size_);
				return *this;
			}
			*/

			void append(const char* data, std::size_t len) {
				buffer_.append(data, len);
			}

			const Buffer& buffer() const {
				return buffer_;
			}

			void resetBuffer() {
				buffer_.reset();
			}

		private:
			Buffer buffer_;
			const char digits[20] = "9876543210123456789";
			const char zero[11] = "0123456789";
			const char digitsHex[17] = "0123456789ABCDEF";
			static const int kMaxNumericSize = 32;
			void staticCheck() {
				static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10, "double size check failed");
				static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10, "long double size check failed");
				static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10, "long size check failed");
				static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10, "long long size check failed");
			}
			template <typename T>
			std::size_t convert(char buf[], T value) {
				T i = value;
				char* p = buf;
				do {
					int lsd = static_cast<int>(i % 10);
					i /= 10;
					*p++ = zero[lsd];
				} while (i != 0);

				if (value < 0)
					*p++ = '-';
				*p = '\0';
				std::reverse(buf, p);
				return p - buf;
			}

			std::size_t convertHex(char buf[], std::uintptr_t value) {
				uintptr_t i = value;
				char* p = buf;
				do {
					int lsd = static_cast<int>(i % 16);
					i /= 16;
					*p++ = digitsHex[lsd];
				} while (i != 0);
				*p = '\0';
				std::reverse(buf, p);
				return p - buf;
			}

			template <typename T>
			void formatInteger(T v) {
				if (buffer_.avail() >= kMaxNumericSize) {
					std::size_t len = convert(buffer_.current(), v);
					buffer_.add(len);
				}
			}
		};

		
	}
	namespace logger {


		enum class LogLevel {
			TRACE, 
			DEBUG, 
			INFO, 
			WARN, 
			ERR,  //ERROR will cause compiler error. Aji MSVC?
			FATAL, 
			NUM_LOG_LEVELS,
		};
		const char* LogLevelName[static_cast<unsigned int>(LogLevel::NUM_LOG_LEVELS)] =
		{
			"TRACE ",
			"DEBUG ",
			"INFO  ",
			"WARN  ",
			"ERROR ",
			"FATAL ",
		};

		void defaultOutput(const char* msg, std::size_t len) {
			std::size_t n = std::fwrite(msg, 1, len, stdout);
		}

		void defaultFlush() {
			std::fflush(stdout);
		}
		class SourceFile {
		public:
			template <std::size_t N>
			inline SourceFile(const char(&arr)[N]) : data_(arr), size_(N - 1) {
				const char* slash = std::strrchr(data_, '/');
				if (slash) {
					data_ = slash + 1;
					size_ -= static_cast<std::size_t>(data_ - arr);
				}
			}

			explicit SourceFile(const char* filename) : data_(filename) {
				const char* slash = std::strrchr(filename, '/');
				if (slash)
					data_ = slash + 1;
				size_ = static_cast<std::size_t>(strlen(data_));
			}

			const char* data_;
			std::size_t size_;
		};


		inline logstream::LogStream& operator<<(logstream::LogStream& s, const SourceFile& v) {
			s.append(v.data_, v.size_);
			return s;
		}


		using OutputFunc = void(*)(const char* msg, std::size_t len);
		using FlushFunc = void(*)();

		LogLevel g_level = LogLevel::INFO;
		OutputFunc g_output = defaultOutput;
		FlushFunc g_flush = defaultFlush;

		class Logger {
		public:
			using OutputFunc = void(*)(const char* msg, std::size_t len);
			using FlushFunc = void(*)();
			
			Logger(SourceFile file, std::size_t line) : impl_(LogLevel::INFO, 0, file, line) {	}
			Logger(SourceFile file, std::size_t line, LogLevel level) : impl_(level, 0, file, line) {	}
			Logger(SourceFile file, std::size_t line, LogLevel level, const char* func) : impl_(level, 0, file, line) {
				impl_.stream_ << func << ' ';
			}
			Logger(SourceFile file, std::size_t line, bool toAbort) : impl_(toAbort ? LogLevel::FATAL : LogLevel::ERR, errno, file, line) {	}
			~Logger() {
				impl_.finish();
				const logstream::LogStream::Buffer& buf(stream().buffer());
				g_output(buf.data(), buf.length());
				if (impl_.level_ == LogLevel::FATAL) {
					g_flush();
					std::abort();
				}
			}

			logstream::LogStream& stream() {
				return impl_.stream_;
			}

			static LogLevel logLevel() {
				return g_level;
			}

			static void setLogLevel(LogLevel level) {
				g_level = level;
			}

			static void setOutput(OutputFunc func) {
				g_output = func;
			}

			static void setFlush(FlushFunc func) {
				g_flush = func;
			}

		private:
			class Impl {
			public:

				Impl(LogLevel level, std::size_t savedErrno, const SourceFile& file, std::size_t line)
				: time_(timestamp::TimeStamp::now()), 
					stream_(), 
					level_(level), 
					line_(line),
					basename_(file) {

					formatTime();
					stream_ << LogLevelName[static_cast<unsigned int>(level_)];
					if (savedErrno)
						stream_ << std::strerror(savedErrno) << " (errno=" << savedErrno << ") ";
						
				}

				void formatTime() {
					int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
					std::time_t seconds = static_cast<std::time_t>(microSecondsSinceEpoch / timestamp::TimeStamp::kMicroSecondsPerSecond);
					int microSeconds = static_cast<int>(microSecondsSinceEpoch % timestamp::TimeStamp::kMicroSecondsPerSecond);
					if (seconds != t_lastSecond) {
						t_lastSecond = seconds;
						struct tm* tm_time = std::gmtime(&seconds);
						std::size_t len = std::snprintf(t_time, sizeof(t_time),
							"%4d/%02d/%02d %02d:%02d:%02d", tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
							tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
						logstream::Fmt us(".%06dZ ", microSeconds);
						stream_ << t_time << us.data();
					}
				}

				void finish() {
					stream_ << " - " << basename_ << ':' << line_ << '\n';
				}

				timestamp::TimeStamp time_;
				logstream::LogStream stream_;
				LogLevel level_;
				std::size_t line_;
				SourceFile basename_;
				std::time_t t_lastSecond;
				char t_time[32];
			};
			Impl impl_;
		};


		
#define SN_LOG_TRACE if (sn_Log::logger::Logger::logLevel() <= sn_Log::logger::LogLevel::TRACE) \
	sn_Log::logger::Logger(__FILE__, __LINE__, sn_Log::logger::LogLevel::TRACE, __func__).stream()
#define SN_LOG_DEBUG if (sn_Log::logger::Logger::logLevel() <= sn_Log::logger::LogLevel::DEBUG) \
	sn_Log::logger::Logger(__FILE__, __LINE__, sn_Log::logger::LogLevel::DEBUG, __func__).stream()
#define SN_LOG_INFO if (sn_Log::logger::Logger::logLevel() <= sn_Log::logger::LogLevel::INFO) \
	sn_Log::logger::Logger(__FILE__, __LINE__, sn_Log::logger::LogLevel::INFO, __func__).stream()
#define SN_LOG_WARN sn_Log::logger::Logger(__FILE__, __LINE__, sn_Log::logger::LogLevel::WARN, __func__).stream()
#define SN_LOG_ERROR sn_Log::logger::Logger(__FILE__, __LINE__, sn_Log::logger::LogLevel::ERR, __func__).stream()
#define SN_LOG_FATAL sn_Log::logger::Logger(__FILE__, __LINE__, sn_Log::logger::LogLevel::FATAL, __func__).stream()
#define SN_LOG_SYSERR sn_Log::logger::Logger(__FILE__, __LINE__, false).stream()
#define SN_LOG_SYSFATAL sn_Log::logger::Logger(__FILE__, __LINE__, true).stream()


		
	}


}









#endif