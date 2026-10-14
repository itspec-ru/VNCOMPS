
#include "stdafx.h"
#include <string>
#include <vector>
#include "PipeChannel.h"



template<typename PtrType = void const*, typename OffType = uintptr_t>
constexpr static inline PtrType ptr_add(void const* _ptr, OffType _offset)
{
    static_assert(std::is_pointer<PtrType>::value,
        "Pointer type expected");
    static_assert(std::is_integral<OffType>::value,
        "Integral type expected");
    return reinterpret_cast<PtrType>(static_cast<char const*>(_ptr) + _offset);
}
//---------------------------------------------------------------------------//
bool Channel::is_open() const
{
    #if defined(OS_POSIX)
        return fd != -1;
    #elif defined(OS_WINDOWS)
        return handle != INVALID_HANDLE_VALUE;
    #else
        #error "Unsupported OS"
    #endif
}
//---------------------------------------------------------------------------//
size_t Channel::write(void const* _buf, size_t _size)
{
    size_t res = 0;
#if defined(OS_POSIX)
    ssize_t n;
    while ((_size > max_io_size)) {
        if (((n = ::write(fd, _buf, max_io_size)) == -1))
            throw std::runtime_error( std::string("Unable to write to Channel object: " + std::to_string(errno)).c_str());
        res += n;
        if (n < max_io_size) return res;
        _size -= n;
        _buf = ptr_add(_buf, n);
    }
    n = ::write(fd, _buf, _size);
    if ((n == -1))
        throw std::runtime_error(std::string("Unable to write to Channel object: " + std::to_string(errno)).c_str());
    res += n;
    return res;
#elif defined(OS_WINDOWS)
    DWORD n;
    static constexpr DWORD max_dword = 0xFFFFFFFFl;
    while ((_size > max_dword)) {
        if ((::WriteFile(handle, _buf, max_dword, &n, nullptr) == 0)) {
            int err = ::GetLastError();
            throw std::runtime_error(std::string("Unable to write to Channel object: " + std::to_string(err)).c_str());
        }
        res += n;
        if (n < max_dword) return res;
        _size -= n;
        _buf = ptr_add(_buf, n);
    }
    //  It is guaranteed here that _size < max_dword, so the static_cast<> is valid
    if ((::WriteFile(handle, _buf, static_cast<DWORD>(_size), &n, nullptr) == 0)) {
        int err = ::GetLastError();
        throw std::runtime_error(std::string("Unable to write to Channel object: " + std::to_string(err)).c_str());
    }
    res += n;
    return res;
#else
#error "Unsupported OS"
#endif
}
//---------------------------------------------------------------------------//
size_t Channel::read(void* _buf, size_t _size)
{
    size_t res = 0;
#if defined(OS_POSIX)
    ssize_t n;
    while ((_size > max_io_size)) {
        if (((n = ::read(fd, _buf, max_io_size)) == -1))
            throw std::runtime_error(std::string("Unable to read from Channel object: " + std::to_string(errno)).c_str());
        if ((n == 0))
            return 0;
        res += n;
        if (n < max_io_size) return res;
        _size -= n;
        _buf = (void*)ptr_add(_buf, n);
    }
    n = ::read(fd, _buf, _size);
    if ((n == -1))
        throw std::runtime_error(std::string("Unable to read from Channel object: " + std::to_string(errno)).c_str());
    if ((n == 0))
        return 0;
    res += n;
    return res;
#elif defined(OS_WINDOWS)
    DWORD n;
    static constexpr DWORD max_dword = 0xFFFFFFFFl;
    while ((_size > max_dword)) {
        if ((::ReadFile(handle, _buf, max_dword, &n, nullptr) == 0)) {
            int err = ::GetLastError();
            throw std::runtime_error(std::string("Unable to read from Channel object: " + std::to_string(err)).c_str());
        }
        if ((n == 0))
            return 0;
        res += n;
        if (n < max_dword) return res;
        _size -= n;
        _buf = reinterpret_cast<void*>(static_cast<char*>(_buf) + n);
    }
    //  It is guaranteed here that _size < max_dword, so the static_cast<> is valid
    if ((::ReadFile(handle, _buf, static_cast<DWORD>(_size), &n, nullptr) == 0)) {
        int err = ::GetLastError();
        throw std::runtime_error(std::string("Unable to read from Channel object: " + std::to_string(err)).c_str());
    }
    if ((n == 0))
        return 0;
    res += n;
    return res;
#else
#error "Unsupported OS"
#endif
}
//---------------------------------------------------------------------------//
void Channel::close() 
{
    #if defined(OS_POSIX)
        if (fd != -1) {
            ::shutdown(fd, SHUT_RDWR);
            ::close(fd);
            fd = -1;
        }
    #elif defined(OS_WINDOWS)
        if (fd != -1) {
            ::_close(fd);
            fd = -1;
            handle = INVALID_HANDLE_VALUE;
        }
        if (handle != INVALID_HANDLE_VALUE) {
            ::CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    #else
        #error "Not supported"
    #endif
}
//---------------------------------------------------------------------------//
void Channel::connect(const std::string& _namespace, uint64_t _tid)
{
    auto path = _namespace;
    #if defined(OS_POSIX)
        if (path.size() + 1 >= sizeof(::sockaddr_un::sun_path))
            throw std::runtime_error( std::string("Socket path name is too long").c_str());
        fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == -1)
            throw std::runtime_error("Unable to create a socket");
        #if defined(OS_LINUX)
            int v = 1;
            if (::setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &v, sizeof(v)) != 0)
                throw std::runtime_error("Unable to set SO_PASSCRED option for the socket");
        #endif // OS_LINUX
        ::sockaddr_un sa;
        sa.sun_family = AF_UNIX;
        ::memset(sa.sun_path, 0, sizeof(sa.sun_path));
        ::memcpy(sa.sun_path, path.data(), path.size() + 1);
        if (::connect(fd, (::sockaddr*)(&sa), sizeof(sa)) != 0)
            throw std::runtime_error(std::string("Unable to connect to the server socket").c_str());
    #elif defined(OS_WINDOWS)
        while (true) {
            handle = ::CreateFileA(
                path.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0, NULL, OPEN_EXISTING, 0, NULL
            );
            if (handle == INVALID_HANDLE_VALUE) {
                int err = ::GetLastError();
                if (err == ERROR_PIPE_BUSY) {
                    continue;
                } else {
                    throw std::runtime_error(std::string("Unable to create a pipe: " + std::to_string(err)).c_str());
                }
            } else {
                break;
            }
        }
    #else
        #error "Not supported"
    #endif
}
//---------------------------------------------------------------------------//
void Channel::read_all(void* _buf, size_t _length)
{
    do {
        size_t n = read(_buf, _length);
        ptr_add(_buf, n);
        _length -= n;
    } while (_length > 0);
}
//---------------------------------------------------------------------------//

void Channel::write_all(void const* _buf, size_t _length)
{
    do {
        size_t n = write(_buf, _length);
        ptr_add(_buf, n);
        _length -= n;
    } while (_length > 0);
}
//---------------------------------------------------------------------------//

void Channel::read_all(Buffer& _buf, size_t _length)
{
    auto sz = _buf.size();
    _buf.resize(sz + _length);
    void* addr = &_buf[sz];
    read_all(addr, _length);
}
//---------------------------------------------------------------------------//

void Channel::write_all(Buffer& _buf, size_t _offset)
{
    auto sz = _buf.size();
    if ((sz < _offset))
        throw std::runtime_error("Bad buffer offset");

    void* addr = &_buf[_offset];
    write_all(addr, sz - _offset);
}
//---------------------------------------------------------------------------//

