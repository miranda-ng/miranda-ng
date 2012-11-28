#include "common.h"
#include "rawping.h"

//#define _WSPIAPI_COUNTOF
#ifndef _MSC_VER
#include <ws2tcpip.h>
#endif

USHORT ip_checksum(USHORT* buffer, int size) 
{
    unsigned long cksum = 0;
    
    // Sum all the words together, adding the final byte if size is odd
    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }
    if (size) {
        cksum += *(UCHAR*)buffer;
    }

    // Do a little shuffling
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    
    // Return the bitwise complement of the resulting mishmash
    return (USHORT)(~cksum);
}

SOCKET sd = -1;
char packet[1024];
char recv_buff[1024];
USHORT seq_no = 0;
bool inited = false;

extern int init_raw_ping() {
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		WSACleanup();
		return 1;
    }

	if(sd != -1)
		closesocket(sd);

    // Create the socket
    //sd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, 0);
	sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sd == INVALID_SOCKET) {
        return 2;
    }

	int ttl = 255;
    if (setsockopt(sd, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR) {
        return 3;
    }

	int our_recv_timeout = 100; // so we ca do multiple recv calls
	if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&our_recv_timeout, sizeof(int)) == SOCKET_ERROR) {
		return 4;
	}

 
	ICMPHeader *header = (ICMPHeader *)packet;
    header->type = PT_ICMP_ECHO_REQUEST;
    header->code = 0;
    header->id = (USHORT)GetCurrentProcessId();

	inited = true;
	if(raw_ping("127.0.0.1", 500) >= 0) {
		return 0;
	}

	inited = false;
	return 5;
}

extern int raw_ping(char *host, int timeout) {
	if(!inited) return -1;

    // Initialize the destination host info block
	sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));

    // Turn first passed parameter into an IP address to ping
    unsigned int addr = inet_addr(host);
    if (addr != INADDR_NONE) {
        // It was a dotted quad number, so save result
        dest.sin_addr.s_addr = addr;
        dest.sin_family = AF_INET;
    }
    else {
        // Not in dotted quad form, so try and look it up
        hostent* hp = gethostbyname(host);
        if (hp != 0) {
            // Found an address for that host, so save it
            memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
            //dest.sin_family = hp->h_addrtype;
			dest.sin_family = AF_INET;
        }
        else {
            // Not a recognized hostname either!
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: unrecognised host", 0);
            return -1;
        }
    }

	ICMPHeader *header = (ICMPHeader *)packet;
    header->seq = ++seq_no;
	header->checksum = 0;
    header->checksum = ip_checksum((USHORT*)header, sizeof(ICMPHeader));

	bool use_hi_res = false;
	LARGE_INTEGER hr_freq, hr_send_time;
	DWORD send_time;
	if(QueryPerformanceFrequency(&hr_freq)) {
		use_hi_res = true;
		QueryPerformanceCounter(&hr_send_time);
	} else 
		send_time = GetTickCount();

	// send packet
    int bwrote = sendto(sd, (char*)packet, sizeof(ICMPHeader), 0, (sockaddr*)&dest, sizeof(dest));
    if (bwrote == SOCKET_ERROR) {
		if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: unable to send", 0);
        return -1;
    }

    // Wait for the ping reply
	sockaddr_in source;
    int fromlen = sizeof(source);
	IPHeader *reply_header = (IPHeader *)recv_buff;
	ICMPHeader *reply;
	DWORD start, current_time;
	LARGE_INTEGER hr_start, hr_current_time, hr_timeout;
	if(use_hi_res) {
		hr_timeout.QuadPart = (timeout * hr_freq.QuadPart / 1000);
		QueryPerformanceCounter(&hr_start);
		hr_current_time = hr_start;
	} else {
		start = GetTickCount();
		current_time = start;
	}

	while(((use_hi_res && (hr_current_time.QuadPart < hr_start.QuadPart + hr_timeout.QuadPart))
				|| (!use_hi_res && current_time < start + timeout))) 
	{
	    int bread = recvfrom(sd, recv_buff, 1024, 0, (sockaddr*)&source, &fromlen);

		if(use_hi_res)
			QueryPerformanceCounter(&hr_current_time);
		else
			current_time = GetTickCount();
		
		if (bread == SOCKET_ERROR) {
			if(WSAGetLastError() != WSAETIMEDOUT) {
				if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: socket error...cycling", 0);
			}
			continue;
		}

		if(reply_header->proto != ICMP_PROTO)
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: packet not ICMP...cycling", 0);
			continue;

		if(reply_header->tos != 0)
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: TOS not 0...cycling", 0);
			continue;

		reply = (ICMPHeader *)(recv_buff + reply_header->h_len * 4);		
		if((unsigned)bread < reply_header->h_len * 4 + sizeof(ICMPHeader)) {
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: short header", 0);
			continue;
		}

		if(reply->id != (USHORT)GetCurrentProcessId())
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: wrong ID...cycling", 0);
			continue;

		if(reply->type != PT_ICMP_ECHO_REPLY && reply->type != PT_ICMP_SOURCE_QUENCH) {
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: wrong type...cycling", 0);
			continue;
		}
		
		//if(reply->seq < seq_no) continue;
		//if(reply->seq > seq_no) return -1;
		if(reply->seq != seq_no) {
			if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: wrong sequence number...cycling", 0);
			continue;
		}

		if(reply->type == PT_ICMP_SOURCE_QUENCH) {
			char buff[1024];
			sprintf(buff, Translate("Host %s requests that you reduce the amount of traffic you are sending."), host);
			MessageBox(0, buff, Translate(PLUG " Warning"), MB_OK | MB_ICONWARNING);
		}

		if(use_hi_res) {
			LARGE_INTEGER ticks;
			ticks.QuadPart = hr_current_time.QuadPart - hr_send_time.QuadPart;
			return (int)(ticks.QuadPart * 1000 / hr_freq.QuadPart); 
		} else
			return current_time - send_time;
	}
	if(options.logging) CallService(PLUG "/Log", (WPARAM)"rawping error: timeout", 0);

	return -1;
}

extern int cleanup_raw_ping() {
	if(inited) {
		closesocket(sd);
		sd = -1;
		WSACleanup();
	}
	return 0;
}
