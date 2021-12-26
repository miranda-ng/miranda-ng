#ifndef _RAWPING_H
#define _RAWPING_H

// ICMP protocol identifier
#define ICMP_PROTO	1

// ICMP packet types
#define PT_ICMP_ECHO_REPLY 0
#define PT_ICMP_DEST_UNREACH 3
#define PT_ICMP_TTL_EXPIRE 11
#define PT_ICMP_ECHO_REQUEST 8
#define PT_ICMP_SOURCE_QUENCH 4

// Minimum ICMP packet size, in bytes
#define ICMP_MIN 8

// The following two structures need to be packed tightly, but unlike
// Borland C++, Microsoft C++ does not do this by default.
#pragma pack(1)

// The IP header
struct IPHeader {
	uint8_t h_len : 4;           // Length of the header in dwords
	uint8_t version : 4;         // Version of IP
	uint8_t tos;               // Type of service
	USHORT total_len;       // Length of the packet in dwords
	USHORT ident;           // unique identifier
	USHORT flags;           // Flags
	uint8_t ttl;               // Time to live
	uint8_t proto;             // Protocol number (TCP, UDP etc)
	USHORT checksum;        // IP checksum
	ULONG source_ip;
	ULONG dest_ip;
};

// ICMP header
struct ICMPHeader {
	uint8_t type;          // ICMP packet type
	uint8_t code;          // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
};

#pragma pack()

extern USHORT ip_checksum(USHORT* buffer, int size);

extern int init_raw_ping();
extern int raw_ping(char *host, int timeout);
extern int cleanup_raw_ping();

#endif
