#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

#include "..\..\ice_net\src\ice_net.cpp"

#pragma comment(lib, "ws2_32.lib")

class win_udp_client final : public a_client
{

private:

    SOCKET sock = INVALID_SOCKET;

    sockaddr_in local_in = sockaddr_in();

    sockaddr_in remote_in = sockaddr_in();


public:

    ~win_udp_client()
    {
        disconnect();
    }

public:

    end_point get_local_point() override
    {
        return end_point(
            ntohl(local_in.sin_addr.s_addr),
            ntohs(local_in.sin_port));
    }
    
    end_point get_remote_point() override
    {
        return end_point(
            ntohl(remote_in.sin_addr.s_addr),
            ntohs(remote_in.sin_port));
    }

public:

    bool connect(end_point& remote_point, end_point& local_point) override
    {
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) return false;

        local_in.sin_family = AF_INET;
        local_in.sin_addr.s_addr = ntohl(local_point.get_address());
        local_in.sin_port = ntohs(local_point.get_port());

        remote_in.sin_family = AF_INET;
        remote_in.sin_addr.s_addr = ntohl(remote_point.get_address());
        remote_in.sin_port = ntohs(remote_point.get_port());

        if (bind(sock, (sockaddr*)&local_in, sizeof(local_in)) == SOCKET_ERROR) return false;

        int addrLen = sizeof(local_in);
        if (getsockname(sock, (sockaddr*)&local_in, &addrLen) != 0) return false;

        return true;
    }

    bool receive_available() override
    {
        u_long available_data = 0;
        if (ioctlsocket(sock, FIONREAD, &available_data) == SOCKET_ERROR) return false;
        return (available_data > 0);
    }

    recv_result receive() override
    {
        recv_result result;

        int remote_size = sizeof(sockaddr_in);

        u_long available_data = 0;
        if (ioctlsocket(sock, FIONREAD, &available_data) == SOCKET_ERROR) return result;

        if (available_data == 0) return result;

        char* recv_arr = new char[available_data];
        int recv = recvfrom(sock, recv_arr, available_data, 0, (sockaddr*)&remote_in, &remote_size);

        if (recv == SOCKET_ERROR)
        {
            delete[] recv_arr;
            return result;
        }

        result.recv_arr = recv_arr;
        result.recv_size = available_data;

        return result;
    }

    bool send(char* data, unsigned short data_size) override
    {
        int result = sendto(sock, data, data_size, 0, (sockaddr*)&remote_in, sizeof(sockaddr_in));

        if (result == SOCKET_ERROR) return false;

        return true;
    }

    void disconnect() override
    {
        if (sock == INVALID_SOCKET) return;

        closesocket(sock);

        sock = INVALID_SOCKET;

        local_in.sin_addr.s_addr = 0;
        local_in.sin_port = 0;

        remote_in.sin_addr.s_addr = 0;
        remote_in.sin_port = 0;

        WSACleanup();
    }
};

#include "..\..\ice_net\src\ice.rudp\rudp_client.cpp"

template void rudp_client::connect<win_udp_client>(end_point remote_point, end_point local_point);