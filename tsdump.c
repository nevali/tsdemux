/* tsdump - Dump interesting parts of MPEG-2 transport streams */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "tsdemux.h"

int
ts_hexdump(ts_packet_t *packet)
{
	size_t c, d;
	uint8_t *bp;
	char sbuf[20];
	
	for(c = 0; c < packet->payloadlen; c += 16)
	{
		bp = &(packet->payload[c]);
		for(d = 0; d < 16; d++)
		{
			if(bp[d] < 0x80 && (isalnum((char) bp[d]) || bp[d] == 0x20))
			{
				sbuf[d] = (char) bp[d];
			}
			else
			{
				sbuf[d] = '.';
			}
		}
		sbuf[d] = 0x00;
		printf("%04x  %02x %02x %02x %02x %02x %02x %02x %02x-%02x %02x %02x %02x %02x %02x %02x %02x  %s\n",
			c, bp[0], bp[1], bp[2], bp[3], bp[4], bp[5], bp[6], bp[7],
			bp[8], bp[9], bp[10], bp[11], bp[12], bp[13], bp[14], bp[15],
			sbuf);
	}
	return 0;
}

int
main(int argc, char **argv)
{
	int c, r;
	FILE *fin;
	ts_stream_t *stream;
	ts_options_t options;
	ts_packet_t packet;
	
	memset(&options, 0, sizeof(options));
	options.timecode = 0;
	options.autosync = 1;
	options.synclimit = 256;
	options.prepad = 0;
	options.postpad = 0;
	r = 0;
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s [OPTIONS] FILE1 ... FILEn\n", argv[0]);
		return 1;
	}
	stream = ts_stream_create(&options);
	for(c = 1; c < argc; c++)
	{
		if(argv[c][0] == '-' && argv[c][1] == 0)
		{
			options.filename = "*Standard input*";
			fin = stdin;
		}
		else
		{
			options.filename = argv[c];
			if(NULL == (fin = fopen(argv[c], "rb")))
			{
				r = 1;
				fprintf(stderr, "%s: %s\n", argv[c], strerror(errno));
			}
		}
		if(fin)
		{
			fprintf(stderr, "Reading from %s\n", options.filename);
			while(-1 != ts_stream_read_packetf(stream, &packet, fin))
			{
				if(packet.sync != 0x47)
				{
					printf("[0x%016llx] Sync byte does not match (expected 0x47, found 0x%02x)\n", stream->seq - 1, packet.sync);
				}
				printf("[0x%016llx] ", stream->seq - 1);
				if(options.timecode)
				{
					printf(" TC: %010lu  ", (unsigned long) packet.timecode);
				}
				printf(" Flags: ");
				if(packet.transerr) { putchar('T'); } else { putchar('-'); }
				if(packet.unitstart) { putchar('U'); } else { putchar('-'); }
				if(packet.priority) { putchar('P'); } else { putchar('-'); }
				if(packet.hasaf) { putchar('A'); } else { putchar('-'); }
				if(packet.haspd) { putchar('D'); } else { putchar('-'); }
				printf("   PID: 0x%04x\n", packet.pid);
			}
			if(fin != stdin)
			{
				fclose(fin);
			}
		}
	}
	return r;
}
