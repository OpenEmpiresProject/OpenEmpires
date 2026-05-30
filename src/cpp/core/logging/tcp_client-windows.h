// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// OpenEmpires project added backoff logic to the above base implementation.

#pragma once

#define WIN32_LEAN_AND_MEAN
// tcp client helper
#include <chrono>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/details/os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

namespace spdlog
{
namespace details
{
class tcp_client
{
    SOCKET socket_ = INVALID_SOCKET;
    std::chrono::time_point<std::chrono::steady_clock> lastAttemptedTime_;
    std::chrono::milliseconds backoffTime_{0};
    std::chrono::milliseconds maxBackoffTime_{10000};
    std::chrono::milliseconds minBackoffTime_{10};

    // Will be use to indicate errors only once to prevent bombarding other sinks with tcp sink
    // errors And will be reseted once a connection established in order to warn again.
    bool alreadyThrewError_ = false;

    static void init_winsock_()
    {
        WSADATA wsaData;
        auto rv = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rv != 0)
        {
            throw_winsock_error_("WSAStartup failed", ::WSAGetLastError());
        }
    }

    static void throw_winsock_error_(const std::string& msg, int last_error)
    {
        char buf[512];
        ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                         last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf,
                         (sizeof(buf) / sizeof(char)), NULL);

        throw_spdlog_ex(fmt_lib::format("tcp_sink - {}: {}", msg, buf));
    }

  public:
    tcp_client()
    {
        init_winsock_();
        lastAttemptedTime_ = std::chrono::steady_clock::now();
    }

    ~tcp_client()
    {
        close();
        ::WSACleanup();
    }

    bool is_connected() const
    {
        return socket_ != INVALID_SOCKET;
    }

    void close()
    {
        if (socket_ != INVALID_SOCKET)
        {
            ::closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
    }

    SOCKET fd() const
    {
        return socket_;
    }

    int connect_socket_with_timeout(SOCKET sockfd,
                                    const struct sockaddr* addr,
                                    int addrlen,
                                    const timeval& tv)
    {
        // If no timeout requested, do a normal blocking connect.
        if (tv.tv_sec == 0 && tv.tv_usec == 0)
        {
            int rv = ::connect(sockfd, addr, addrlen);
            if (rv == SOCKET_ERROR && WSAGetLastError() == WSAEISCONN)
            {
                return 0;
            }
            return rv;
        }

        // Switch to non‐blocking mode
        u_long mode = 1UL;
        if (::ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }

        int rv = ::connect(sockfd, addr, addrlen);
        int last_error = WSAGetLastError();
        if (rv == 0 || last_error == WSAEISCONN)
        {
            mode = 0UL;
            if (::ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR)
            {
                return SOCKET_ERROR;
            }
            return 0;
        }
        if (last_error != WSAEWOULDBLOCK)
        {
            // Real error
            mode = 0UL;
            if (::ioctlsocket(sockfd, FIONBIO, &mode))
            {
                return SOCKET_ERROR;
            }
            return SOCKET_ERROR;
        }

        // Wait until socket is writable or timeout expires
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sockfd, &wfds);

        rv = ::select(0, nullptr, &wfds, nullptr, const_cast<timeval*>(&tv));

        // Restore blocking mode regardless of select result
        mode = 0UL;
        if (::ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }

        if (rv == 0)
        {
            WSASetLastError(WSAETIMEDOUT);
            return SOCKET_ERROR;
        }
        if (rv == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }

        int so_error = 0;
        int len = sizeof(so_error);
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &len) ==
            SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        if (so_error != 0 && so_error != WSAEISCONN)
        {
            // connection failed
            WSASetLastError(so_error);
            return SOCKET_ERROR;
        }

        return 0; // success
    }

    // try to connect or throw on failure
    void connect(const std::string& host, int port, int timeout_ms = 0)
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = now - lastAttemptedTime_;

        // Still haven't spent backoffTime_, have to wait.
        if (duration < backoffTime_)
        {
            return;
        }
        lastAttemptedTime_ = now;
        backoffTime_ = std::max(backoffTime_, minBackoffTime_);
        backoffTime_ = backoffTime_ * 2; // Go exponential
        backoffTime_ = std::min(backoffTime_, maxBackoffTime_);

        if (is_connected())
        {
            close();
        }
        struct addrinfo hints{};
        ZeroMemory(&hints, sizeof(hints));

        hints.ai_family = AF_UNSPEC;     // To work with IPv4, IPv6, and so on
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_NUMERICSERV; // port passed as as numeric value
        hints.ai_protocol = 0;

        timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        auto port_str = std::to_string(port);
        struct addrinfo* addrinfo_result;
        auto rv = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &addrinfo_result);
        int last_error = 0;
        if (rv != 0)
        {
            last_error = ::WSAGetLastError();

            if (not alreadyThrewError_)
            {
                alreadyThrewError_ = true;
                throw_winsock_error_("getaddrinfo failed", last_error);
            }
            else
            {
                return;
            }
        }

        // Try each address until we successfully connect(2).
        for (auto* rp = addrinfo_result; rp != nullptr; rp = rp->ai_next)
        {
            socket_ = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (socket_ == INVALID_SOCKET)
            {
                last_error = ::WSAGetLastError();
                continue;
            }
            if (connect_socket_with_timeout(socket_, rp->ai_addr, (int) rp->ai_addrlen, tv) == 0)
            {
                last_error = 0;
                break;
            }
            last_error = WSAGetLastError();
            ::closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
        ::freeaddrinfo(addrinfo_result);
        if (socket_ == INVALID_SOCKET)
        {
            if (not alreadyThrewError_)
            {
                alreadyThrewError_ = true;
                throw_winsock_error_("connect failed", last_error);
            }
            else
            {
                return;
            }
        }
        if (timeout_ms > 0)
        {
            DWORD tv = static_cast<DWORD>(timeout_ms);
            ::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv));
            ::setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (const char*) &tv, sizeof(tv));
        }

        // set TCP_NODELAY
        int enable_flag = 1;
        ::setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&enable_flag),
                     sizeof(enable_flag));

        // A successful connection. Remove backoff time, and make it eligible to throw errors again
        backoffTime_ = std::chrono::milliseconds::zero();
        alreadyThrewError_ = false;
    }

    // Send exactly n_bytes of the given data.
    // On error close the connection and throw.
    void send(const char* data, size_t n_bytes)
    {
        if (not is_connected())
        {
            return;
        }

        size_t bytes_sent = 0;
        while (bytes_sent < n_bytes)
        {
            const int send_flags = 0;
            auto write_result =
                ::send(socket_, data + bytes_sent, (int) (n_bytes - bytes_sent), send_flags);
            if (write_result == SOCKET_ERROR)
            {
                int last_error = ::WSAGetLastError();
                close();
                throw_winsock_error_("send failed", last_error);
            }

            if (write_result == 0) // (probably should not happen but in any case..)
            {
                break;
            }
            bytes_sent += static_cast<size_t>(write_result);
        }
    }
};
} // namespace details
} // namespace spdlog