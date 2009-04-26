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
			(unsigned int) c, bp[0], bp[1], bp[2], bp[3], bp[4], bp[5], bp[6], bp[7],
			bp[8], bp[9], bp[10], bp[11], bp[12], bp[13], bp[14], bp[15],
			sbuf);
	}
	return 0;
}

int
dumppmt(ts_stream_t *stream, ts_table_t *table)
{
	size_t c;
	ts_pidinfo_t *info;
	const ts_streamtype_t *stype;
	
	printf("  0x%04x - Program 0x%04x\n",  (unsigned int) table->pid, (unsigned int) table->progid);
	if(1 == table->expected)
	{
		printf("  - Table defined but not present in stream\n");
		return 0;
	}
	printf("  - Table contains details of %lu streams\n", (unsigned long) table->d.pmt.nes);
	for(c = 0; c < table->d.pmt.nes; c++)
	{
		info = table->d.pmt.es[c];
		printf("    0x%04x - ", info->pid);
		if(NULL == (stype = ts_typeinfo(info->stype)))
		{
			printf("Unknown stream type 0x%02x", info->stype);
		}
		else
		{
			printf("%s", stype->name);
		}
		switch(info->subtype)
		{
			case PST_UNSPEC: printf(" unspecified"); break;
			case PST_VIDEO: printf(" video"); break;
			case PST_AUDIO: printf(" audio"); break;
			case PST_INTERACTIVE: printf(" interactive"); break;
			case PST_CC: printf(" closed captioning"); break;
			case PST_IP: printf(" Internet Protocol"); break;
			case PST_SI: printf(" stream information"); break;
			case PST_NI: printf(" network information"); break;
		}
		switch(info->pidtype)
		{
			case PT_SECTIONS:
				printf(" sections");
				break;
			case PT_DATA:
				printf(" data");
				break;
			case PT_PES:
				printf(" PES");
				break;
		}
		if(1 == info->pcr)
		{
			printf(" (PCR)");
		}
		if(0 == info->seen)
		{
			printf(" (defined but not present)");
		}
		putchar('\n');
	}
	return 0;
}

int
dumppat(ts_stream_t *stream, ts_table_t *table)
{
	size_t c;
	
	printf("0x%04x - Program Association Table:\n", (unsigned int) table->pid);
	if(1 == table->expected)
	{
		printf("- Table defined but not present in stream\n");
		return 0;
	}
	for(c = 0; c < table->d.pat.nprogs; c++)
	{
		if(0 == table->d.pat.progs[c]->progid)
		{
			printf("  0x%04x - Network Information Table\n", table->d.pat.progs[c]->pid);
		}
		else
		{
			dumppmt(stream, table->d.pat.progs[c]);
		}
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
/*				printf("[0x%016llx] ", stream->seq - 1);
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
				printf("   PID: 0x%04x ", packet.pid);
				if(packet.pidinfo)
				{
					switch(packet.pidinfo->pidtype)
					{
						case PT_UNSPEC:
							putchar('-');
							break;
						case PT_SECTIONS:
							putchar('S');
							break;
						case PT_PES:
							putchar('P');
							break;
						case PT_DATA:
							putchar('D');
							break;
						case PT_NULL:
							putchar('N');
							break;
						default:
							putchar('?');
							printf(" [%d] ", packet.pidinfo->pidtype);
					}
					if(packet.pidinfo->seen)
					{
						putchar('S');
					}
					else
					{
						putchar('-');
					}
					if(packet.pidinfo->defined)
					{
						putchar('D');
					}
					else
					{
						putchar('-');
					}
				}
				else
				{
					putchar('?');
					putchar('?');
					putchar('?');
				}
				putchar('\n'); */
/*				ts_hexdump(&packet); */
			}
			if(fin != stdin)
			{
				fclose(fin);
			}
		}
	}
	printf("Stream summary:\n");
	if(NULL == stream->pat)
	{
		printf("- Stream has no Program Association Table\n");
	}
	else
	{
		dumppat(stream, stream->pat);
	}
	return r;
}
