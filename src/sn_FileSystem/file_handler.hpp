#include "../sn_CommonHeader.h"

namespace sn_FileSystem {
    
    namespace file_handler {

#ifdef SN_ENABLE_CPP_17_EXPERIMENTAL
		[[noreturn]] void file_throw_system() {
			throw std::system_error{static_cast<int>(::GetLastError()), std::system_category()};
		}

		std::experimental::generator<std::wstring> Files(const std::wstring& path) {
			::WIN32_FIND_DATAW ffd = {};
			auto new_path = path + L"\\\*";
			auto file = ::FindFirstFileW(new_path.c_str(), &ffd);
			auto clean = sn_Thread::scope_guard::make_scope([file]() noexcept { ::FindClose(file);});
			if (file == INVALID_HANDLE_VALUE) {
				file_throw_system();
			}
			do {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					auto name = std::wstring_view{ffd.cFileName};
					if (name == L"." || name == L"..")
						continue;
					auto next_path = path + L"\\" + name.data();
					for (auto p: Files(next_path)) {
						co_yield p;
					}
				}
				else {
					coyield ffd.cFileName;
				}
			} while (::FindNextFileW(file, &ffd));
		}
#endif
		// Notice the deleter redirect std::unique_ptr<T, D> as another type
		struct FileHandle {
			::HANDLE m_fileHandle;
			FileHandle(::HANDLE h) noexcept
				: m_fileHandle{h} {}
			FileHandle(std::nullptr_t = nullptr) noexcept
				: FileHandle{INVALID_HANDLE_VALUE} {}
			operator ::HANDLE() noexcept {
				return m_fileHandle;
			}
			explicit operator bool() noexcept {
				return m_fileHandle != INVALID_HANDLE_VALUE;
			}
		};
		bool operator== (FileHandle lhs, FileHandle rhs) noexcept {
			return lhs.m_fileHandle == rhs.m_fileHandle;
		}

		bool operator!= (FileHandle lhs, FileHandle rhs) noexcept {
			return !(lhs == rhs);
		}
		struct FileCloser {
			using pointer_t = FileHandle;
			void operator()(pointer_t p) const noexcept {
				(void)::CloseHandle(p.m_fileHandle);
			}
		};
		using unique_file_ptr = std::unique_ptr<void, FileCloser>;
		struct LocalMemoryDeleter {
			void operator()(void* p) const noexcept {
				(void)::LocalFree(p);
			}
		};
		using unique_local_memory_ptr = std::unique_ptr<void, LocalMemoryDeleter>;

		//  unique_file_ptr f.reset(::CreateFile(LR"(.\1.txt)", GENERIC_WRITE, {}, {}, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, {}));
		//  const auto h = unique_local_memory_ptr{ ::LocalAlloc(LMEM_FIXED, 20) };
	}
}