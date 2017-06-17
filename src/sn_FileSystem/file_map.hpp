#include "../sn_CommonHeader.h"

namespace sn_FileSystem {

#ifdef SN_ENABLE_FILE_MAP
    namespace file_map {

        /*
            Read-only memory-mapped file wrapper.
            It handles only files that can be wholly loaded
            into the address space of the process.
            The constructor opens the file, the destructor closes it.
            The "data" function returns a pointer to the beginning of the file,
            if the file has been successfully opened, otherwise it returns 0.
            The "size" function returns the length of the file in bytes,
            if the file has been successfully opened, otherwise it returns 0.
        */
        class InputMemoryFile {
        
        public:
            InputMemoryFile(const char *pathname)
                : data_(0), size(0), 
#if defined(__unix__)
                file_handle_(-1) {
                    file_handle_ = ::open(pathname, O_RDONLY);
                    if (file_handle_ == -1) 
                        return;
                    struct stat sbuf;
                    if (::fstat(file_handle_, &sbuf) == -1) 
                        return;
                    data_ = static_cast<const char*>(::mmap(
                        0, sbuf.st_size, PROT_READ, MAP_SHARED, file_handle_, 0
                    ));
                    if (data_ == MAP_FAILED) 
                        data_ = 0;
                    else 
                        size_ = sbuf.st_size;
#elif defined(_WIN32)
                file_handle_(INVALID_HANDLE_VALUE),
                file_mapping_handle_(INVALID_HANDLE_VALUE) {
                    file_handle_ = ::CreateFile(
                        pathname, 
                        GENERIC_READ,
                        FILE_SHARE_READ, 
                        0, 
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0);
                    if (file_handle_ == INVALID_HANDLE_VALUE)
                        return;
                    file_mapping_handle_ = ::CreateFileMapping(
                        file_handle_, 
                        0, 
                        PAGE_READONLY, 
                        0, 0, 0);
                    if (file_mapping_handle_ == INVALID_HANDLE_VALUE) 
                        return;
                    data_ = static_cast<char*>(::MapViewOfFile(
                        file_mapping_handle_, 
                        FILE_MAP_READ, 
                        0, 0, 0
                    ));
                    if (data_) 
                        size_ = ::GetFileSize(file_handle_, 0);
#endif
            }

            ~InputMemoryFile() {
#if defined(__unix__)
                ::munmap(const_cast<char*>(data_), size_);
                ::close(file_handle_);  
#elif defined(_WIN32)
                ::UnmapViewOfFile(data_);
                ::CloseHandle(file_mapping_handle_);
                ::CloseHandle(file_handle_);
#endif
            }

            const char* data() const { return data_; }
            size_t size() const { return size_; }
        private:
            const char* data_;
            size_t size_;
#if defined(__unix__)
            int file_handle_;
#elif defined(_WIN32)
            typedef void* HANDLE;
            HANDLE file_handle_;
            HANDLE file_mapping_handle_;
#else
    #error Only Posix or Windows systems can use memory-mapped files.
#endif

        };

        /*
            Read/write memory-mapped file wrapper.
            It handles only files that can be wholly loaded
            into the address space of the process.
            The constructor opens the file, the destructor closes it.
            The "data" function returns a pointer to the beginning of the file,
            if the file has been successfully opened, otherwise it returns 0.
            The "size" function returns the initial length of the file in
            bytes,
            if the file has been successfully opened, otherwise it returns 0.
            Afterwards it returns the size the physical file will get if it is
            closed now.
            The "resize" function changes the number of bytes of the
            significant
            part of the file. The resulting size can be retrieved
            using the "size" function.
            The "reserve" grows the phisical file to the specified number of
            bytes.
            The size of the resulting file can be retrieved using "capacity".
            Memory mapped files cannot be shrinked;
            a value smaller than the current capacity is ignored.
            The "capacity()" function return the size the physical file has at
            this time.
            The "flush" function ensure that the disk is updated
            with the data written in memory.
        */
        class MemoryFile {
        public:
            enum e_open_mode {
                if_exists_fail_if_not_exists_create,
                if_exists_keep_if_dont_exists_fail,
                if_exists_keep_if_dont_exists_create,
                if_exists_truncate_if_not_exists_fail,
                if_exists_truncate_if_not_exists_create,
            };
            MemoryFile(const char *pathname, e_open_mode open_mode)
                : data_(0),
                  size_(0),
#if defined(__unix__)
                  file_handle_(-1) {
                int posix_open_mode = O_RDWR;
                switch (open_mode) {
                    case if_exists_fail_if_not_exists_create:
                        posix_open_mode |= O_EXCL | O_CREAT;
                        break;
                    case if_exists_keep_if_dont_exists_fail:
                        break;
                    case if_exists_keep_if_dont_exists_create:
                        posix_open_mode |= O_CREAT;
                        break;
                    case if_exists_truncate_if_not_exists_fail:
                        posix_open_mode |= O_TRUNC;
                        break;
                    case if_exists_truncate_if_not_exists_create:
                        posix_open_mode |= O_TRUNC | O_CREAT;
                        break;
                    default: 
                        return;
                }
                const size_t min_file_size = 4096;
                file_handle_ = ::open(
                    pathname, 
                    posix_open_mode, 
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
                );
                if (file_handle_ == -1) 
                    return;
                struct stat sbuf;
                if (::fstat(file_handle_, &sbuf) == -1) 
                    return;
                size_t initial_file_size = sbuf.st_size;
                size_t adjusted_file_size = initial_file_size == 0 ? min_file_size : initial_file_size;
                ::ftruncate(file_handle_, adjusted_file_size);
                data_ = static_cast<char*>(::mmap(
                    0, 
                    adjusted_file_size, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED,
                    file_handle_, 0
                ));
                if (data_ == MAP_FAILED) 
                    data_ = 0;
                else {
                    size_ = initial_file_size;
                    capacity_ = adjusted_file_size;
                }
#elif defined(_WIN32)
                file_handle_(INVALID_HANDLE_VALUE),
                file_mapping_handle_(INVALID_HANDLE_VALUE) {

                    int windows_open_mode;
                    switch (open_mode) {
                        case if_exists_fail_if_not_exists_create:
                            windows_open_mode = CREATE_NEW;
                            break;
                        case if_exists_keep_if_dont_exists_fail:
                            windows_open_mode = OPEN_EXISTING;
                            break;
                        case if_exists_keep_if_dont_exists_create:
                            windows_open_mode = OPEN_ALWAYS;
                            break;
                        case if_exists_truncate_if_not_exists_fail:
                            windows_open_mode = TRUNCATE_EXISTING;
                            break;
                        case if_exists_truncate_if_not_exists_create:
                            windows_open_mode = CREATE_ALWAYS;
                            break;
                        default: 
                            return;
                    }
                    const size_t min_file_size = 4096;
                    file_handle_ = ::CreateFile(
                        pathname, 
                        GENERIC_READ | GENERIC_WRITE,
                        0, 0, 
                        windows_open_mode, 
                        FILE_ATTRIBUTE_NORMAL, 0);
                    if (file_handle_ == INVALID_HANDLE_VALUE) 
                        return;
                    size_t initial_file_size = ::GetFileSize(file_handle_, 0);
                    size_t adjusted_file_size = initial_file_size == 0 ? min_file_size : initial_file_size;
                    file_mapping_handle_ = ::CreateFileMapping(
                        file_handle_, 0, 
                        PAGE_READWRITE, 0, 
                        adjusted_file_size, 0
                    );
                    if (file_mapping_handle_ == INVALID_HANDLE_VALUE) 
                        return;
                    data_ = static_cast<char*>(::MapViewOfFile(
                        file_mapping_handle_, 
                        FILE_MAP_WRITE, 0, 0, 0
                    ));
                    if (data_) {
                        size_ = initial_file_size;
                        capacity_ = adjusted_file_size;
                    }
#endif
            }

            ~MemoryFile() {
#if defined(__unix__)
                    ::munmap(data_, size_);
                if (size_ != capacity_) {
                    ::ftruncate(file_handle_, size_);
                }
                ::close(file_handle_);
#elif defined(_WIN32)
                ::UnmapViewOfFile(data_);
                ::CloseHandle(file_mapping_handle_);
                if (size_ != capacity_) {
                    ::SetFilePointer(file_handle_, size_, 0, FILE_BEGIN);
                    ::SetEndOfFile(file_handle_);
                }
                ::CloseHandle(file_handle_);
            }


            char* data() { return data_; }

            void resize(size_t new_size) {
                if (new_size > capacity_) 
                    reserve(new_size);
                size_ = new_size;
            }

            void reserve(size_t new_capacity) {
                if (new_capacity <= capacity_) 
                    return;
#if defined(__unix__)
                ::munmap(data_, size_);
                ::ftruncate(file_handle_, new_capacity);
                data_ = static_cast<char*>(::mmap(
                    0, new_capacity, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED,
                    file_handle_, 0));
                if (data_ == MAP_FAILED) 
                    data_ = 0;
                capacity_ = new_capacity;
#elif defined(_WIN32)
                ::UnmapViewOfFile(data_);
                ::CloseHandle(file_mapping_handle_);
                file_mapping_handle_ = ::CreateFileMapping(
                    file_handle_, 0, 
                    PAGE_READWRITE, 0, 
                    new_capacity, 0);
                capacity_ = new_capacity;
                data_ = static_cast<char*>(::MapViewOfFile(
                    file_mapping_handle_, 
                    FILE_MAP_WRITE, 
                    0, 0, 0
                ));
#endif
            }
            
            size_t size() const { return size_; }
            size_t capacity() const { return capacity_; }
            bool flush() {
#if defined(__unix__)
                return ::msync(data_, size_, MS_SYNC) == 0;
#elif defined(_WIN32)
                return ::FlushViewOfFile(data_, size_) != 0;
#endif
            }
        private:
            char* data_;
            size_t size_;
            size_t capacity_;
#if defined(__unix__)
            int file_handle_;
#elif defined(_WIN32)
            typedef void * HANDLE;
            HANDLE file_handle_;
            HANDLE file_mapping_handle_;
#else
    #error Only Posix or Windows systems can use memory-mapped files.
#endif
            };

    }
#endif
}