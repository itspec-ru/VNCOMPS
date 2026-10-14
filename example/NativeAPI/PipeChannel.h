#ifndef __PIPECHANNEL__
#define __PIPECHANNEL__

#include <thread>
#include <future>
#include <vector>
#include <exception>
#include <stdexcept>
#if defined(OS_POSIX)
    #include <unistd.h>
    #include <type_traits>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/un.h>
#elif defined(OS_WINDOWS)
    #include <windows.h>
    #include <io.h>
    #include <stdint.h>
#else
    #error "Unsupported OS"
#endif

using Buffer = std::vector<unsigned char>;
using Thread = std::thread;
class Channel 
{
protected:
    int fd = -1;
#if defined(OS_WINDOWS)
    HANDLE handle = INVALID_HANDLE_VALUE;
#endif
//  On Linux we have a specific limitation for max size of I/O operations
//  for both 32- and 64-bit systems.
#if defined(OS_POSIX)
    static constexpr ssize_t max_io_size = 0x7ffff000;
#endif

public:
    Channel() {}

    Channel(
    #if defined(OS_POSIX)
        int _fd
    #elif defined(OS_WINDOWS)
        HANDLE _handle
    #endif
    ) :
        #if defined(OS_POSIX)
            fd(_fd)
        #elif defined(OS_WINDOWS)
            handle(_handle)
        #endif
    {
#if defined(OS_WINDOWS)
        fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(handle), 0);
#endif
    }


    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&& other) noexcept :
        fd(other.fd)
        #if defined(OS_WINDOWS)
            , handle(other.handle)
        #endif
    {
        other.fd = -1;
        #if defined(OS_WINDOWS)
            other.handle = INVALID_HANDLE_VALUE;
        #endif
    }

    Channel& operator=(Channel&& other) noexcept
    {
        if (this != &other)
        {
            fd = other.fd;
            other.fd = -1;
            #if defined(OS_WINDOWS)
                handle = other.handle;
                other.handle = INVALID_HANDLE_VALUE;
            #endif
        }
        return *this;
    }
    ~Channel()
    {
        close();
    }

    int get_fd() const noexcept {   return fd;   }

#if defined(OS_WINDOWS)
    HANDLE get_handle() const noexcept {   return handle;   }
#endif // OS_WINDOWS

    bool is_open() const;
    size_t write(void const* _buf, size_t _size);
    size_t read(void* _buf, size_t _size);
    void read_all(void* _buf, size_t _length);
    void write_all(void const* _buf, size_t _length);
    void read_all(Buffer& _buf, size_t _length);
    void write_all(Buffer& _buf, size_t _offset = 0);
    void flush()
    {   }
    void connect(const std::string& _namespace, uint64_t _tid = 0);
    void close();
};

class ChannelServer :
    public Channel
{
    std::string ns;
    std::thread::id pid;
    std::string path;
#if defined(OS_WINDOWS)
    std::atomic<int> stop_flag{0};
#endif
    std::unique_ptr<std::thread> listen_thread;

public:

    ChannelServer(std::string const& _namespace) :
        ns{_namespace},
        pid{std::this_thread::get_id()},
        path{_namespace}
    {   }

    ~ChannelServer()
    {   
        if (listen_thread != nullptr) {
            stop();
        }
    }

    template<typename L>
    std::future<void> start(L&& _lambda)
    {
        std::future<void> f;
        std::promise<void> pr;
        #if defined(OS_POSIX)
            fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (fd == -1)
                throw std::runtime_error("Unable to create a socket");
            ::sockaddr_un sa;
            sa.sun_family = AF_UNIX;
            ::memset(sa.sun_path, 0, sizeof(sa.sun_path));
            ::memcpy(sa.sun_path, path.data(), path.size() + 1);
            if (::bind(fd, (::sockaddr*)(&sa), sizeof(sa)) != 0)
                throw std::runtime_error("Unable to bind server socket");
        #elif defined(OS_WINDOWS)
        #endif
        listen_thread.reset(new Thread{[this, lambda = std::forward<L>(_lambda), pr = std::move(pr)]() mutable {
            #if defined(OS_POSIX)
                if (::listen(fd, 512) != 0) {
                    pr.set_exception(std::make_exception_ptr(
                        std::runtime_error("Error listening a server socket")
                    ));
                    return;
                }
                pr.set_value();
                while (true) {
                    int clt = ::accept(fd, nullptr, nullptr);
                    if (clt == -1) {
                        pr.set_exception(std::make_exception_ptr(
                            std::runtime_error("Error accepting a connection")
                        ));
                        break;
                    }
                    #if defined(OS_LINUX)
                        ::ucred cred;
                        ::socklen_t l = sizeof(cred);
                        if (::getsockopt(clt, SOL_SOCKET, SO_PEERCRED, &cred, &l) != 0) {
                            pr.set_exception(std::make_exception_ptr(
                                std::runtime_error("Unable to set SO_PASSCRED option for the socket")
                            ));
                            break;
                        }
                    #endif // OS_LINUX
                    std::thread(lambda, std::move(Channel( clt ))).detach();
                }
            #elif defined(OS_WINDOWS)
                bool first_time = true;
                while (stop_flag.load(std::memory_order_acquire) == 0) {
                    static constexpr DWORD pipe_bufsz = 4096;
                    handle = ::CreateNamedPipeA(
                        path.data(),
                        PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                        PIPE_UNLIMITED_INSTANCES,
                        pipe_bufsz, pipe_bufsz, 0, NULL
                    );
                    if (first_time) {
                        if (handle == INVALID_HANDLE_VALUE) {
                            int err = ::GetLastError();
                            pr.set_exception(std::make_exception_ptr(
                                std::runtime_error("Unable to create a pipe")
                            ));
                        } else {
                            pr.set_value();
                        }
                        first_time = false;
                    } else {
                        /// @todo: What to do with errors while running the loop?
                    }
                    if (::ConnectNamedPipe(handle, NULL) == 0) {
                        int err = ::GetLastError();
                        if (err != ERROR_PIPE_CONNECTED) {
                            break;
                        }
                    }
                    std::thread(lambda, std::move(Channel( handle ))).detach();
                }
                stop_flag.store(2, std::memory_order_release);
            #endif
        }});
        return f;
    }

    void stop()
    {
        #if defined(OS_POSIX)
            ::shutdown(fd, SHUT_RDWR);
            ::close(fd);
            #if !defined (OS_LINUX)
                if (!path.empty()) {
                    unlink(path.c_str());
                }
            #endif
        #elif defined(OS_WINDOWS)
            HANDLE h;
            stop_flag.store(1, std::memory_order_release);
            while (stop_flag.load(std::memory_order_acquire) != 2) {
                h = ::CreateFileA(
                    path.data(),
                    GENERIC_READ | GENERIC_WRITE,
                    0, NULL, OPEN_EXISTING, 0, NULL
                );
                if (h == INVALID_HANDLE_VALUE) {
                    int err = ::GetLastError();
                    if (err == ERROR_PIPE_BUSY) {
                        continue;
                    } else {
                        // Well, if an error occured and the pipe is not busy, probably somebody else stopped a listening thread, so just break the loop.
                        break;
                    }
                } else {
                    if (h != INVALID_HANDLE_VALUE)
                    {
                        ::CloseHandle(h);
                        h = INVALID_HANDLE_VALUE;
                    }
                    break;
                }
            }
        #endif // OS
        if (listen_thread != nullptr) listen_thread->join();
        listen_thread = nullptr;
    }

    std::string get_namespace() const noexcept
    {   return ns;   }

    Thread::id get_pid() const noexcept
    {   return pid;   }
};
#endif //__PIPECHANNEL__
