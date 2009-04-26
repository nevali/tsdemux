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

static int hexdump(ts_packet_t *packet);
static int dumptable(ts_stream_t *stream, ts_table_t *table, int complete);

static int
hexdump(ts_packet_t *packet)
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

static int
dumpdvbnit(ts_stream_t *stream, ts_table_t *table, int complete)
{
	if(0 == complete && 1 == table->expected)
	{
		printf("  0x%04x - DVB Network Information Table 0x%02x (not yet defined)\n", (unsigned int) table->pid, table->tableid);
		return 0;
	}
	printf("  0x%04x - DVB Network Information Table 0x%02x\n", (unsigned int) table->pid, table->tableid);
	if(1 == table->expected)
	{
		if(1 == complete)
		{
			printf("  - Table defined but not present in stream\n");
		}
		return 0;
	}
	return 0;
}


static int
dumppmt(ts_stream_t *stream, ts_table_t *table, int complete)
{
	size_t c;
	ts_pidinfo_t *info;
	const ts_streamtype_t *stype;
	
	if(0 == complete && 1 == table->expected)
	{
		printf("  0x%04x - Program 0x%04x (not yet defined)\n",  (unsigned int) table->pid, (unsigned int) table->progid);
		return 0;
	}
	printf("  0x%04x - Program 0x%04x\n",  (unsigned int) table->pid, (unsigned int) table->progid);
	if(1 == table->expected)
	{
		if(1 == complete)
		{
			printf("  - Table defined but not present in stream\n");
		}
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
		if(1 == complete && 0 == info->seen)
		{
			printf(" (defined but not present)");
		}
		putchar('\n');
	}
	return 0;
}

static int
dumppat(ts_stream_t *stream, ts_table_t *table, int complete)
{
	size_t c;
	
	printf("0x%04x - Program Association Table\n", (unsigned int) table->pid);
	if(1 == table->expected)
	{
		if(1 == complete)
		{
			printf("- Table defined but not present in stream\n");
		}
		return 0;
	}
	for(c = 0; c < table->d.pat.nprogs; c++)
	{
		dumptable(stream, table->d.pat.progs[c], complete);
	}
	return 0;
}

int
dumptable(ts_stream_t *stream, ts_table_t *table, int complete)
{
	switch(table->tableid)
	{
		case TID_PAT:
			return dumppat(stream, table, complete);
		case TID_PMT:
			return dumppmt(stream, table, complete);
		case TID_DVB_NIT:
			return dumpdvbnit(stream, table, complete);
		default:
			printf("0x%04x - Unknown table ID 0x%02x\n", (unsigned int) table->pid, table->tableid);
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("Usage: %s [OPTIONS] FILE1 [... FILEn]\n\n", progname);
	printf("Parse an MPEG-2 transport stream from one or more files. Specify '-' as a\n"
		"filename to read from standard input.\n\n");
	printf("  -h           Print this usage message\n");
	printf("  -p           Show header details from each packet as it's read\n");
	printf("  -H           Hex dump each packet as it's read (implies -p)\n");
	printf("  -d           Print detailed information about each packet (implies -p)\n");
	printf("  -s           Print a summary of the stream once complete\n");
	printf("  -a           Attempt to automatically sync the stream\n");
	printf("  -A           Source stream is AVCHD/Blu-Ray (disables -a)\n");
	printf("  -q           Halt as soon as the first PAT and PMT have been decoded\n"
		"               (implies -s)\n");
	printf("\nIf no options are specified, defaults are to autosync (-a) and print a\n"
		"final summary (-s). Autosync is disabled if -A is specified (AVCHD/Blu-Ray\n"
		"streams have a fixed format where a 32-bit timecode prefixes each packet).\n"
		"If you know your source only contains a single program and you just want a\n"
		"list of the elementary stream PIDs, use the -q option.\n"
		"\n"
		"Examples:\n"
		"  tsdump -A 000001.MTS\n"
		"      Read the AVCHD file 0000001.MTS and print a summary of the contents.\n\n"
		"  dvbstream -o 8192 | tsdump -qp -\n"
		"      Use dvbstream(1) to capture TS packets from a DVB tuner card, which\n"
		"      tsdump will read from standard input. Halt as soon as a complete\n"
		"      PAT and PMT have been read, and print a summary of each packet decoded.\n\n"
		"Packet summary flags:\n"
		"   T = Transport error       U = Unit start      P = Priority\n"
		"   A = Adaption field present                    D = Payload present\n\n"
		"Packet summary PID suffixes:\n"
		"   1. PID type (U=unspec, S=sections, P=PES, D=data, N=null, ?=unknown)\n"
		"   2. Seen flag (indicates that packets were decoded, always the case in\n"
		"      in a packet summary)\n"
		"   3. Defined flag (indicates that the PID was defined by a PAT or PMT)\n\n"
		"   A suffix of '???' indicates that no information about the PID is\n"
		"   available at the time of output. You may see this at the beginning of a\n"
		"   DVB stream capture (prior to the PAT and PMTs being decoded).\n"
		);
}

int
main(int argc, char **argv)
{
	int showhex, showpackets, showsummary, showdetail, quick;
	int c, r;
	FILE *fin;
	ts_stream_t *stream;
	ts_options_t options;
	ts_packet_t packet;
	size_t pmtcount;
	int patseen;
	
	memset(&options, 0, sizeof(options));
	options.progname = argv[0];
	options.timecode = 0;
	options.autosync = 1;
	options.synclimit = 256;
	options.prepad = 0;
	options.postpad = 0;
	r = 0;
	showhex = 0;
	showpackets = 0;
	showdetail = 0;
	showsummary = -1;
	quick = 0;
	while(-1 != (c = getopt(argc, argv, "hpHdsaAq")))
	{
		switch(c)
		{
			case 'p':
				showpackets = 1;
				break;
			case 'H':
				showhex = showpackets = 1;
				break;
			case 'd':
				showdetail = showpackets = 1;
				break;
			case 's':
				showsummary = 1;
				break;
			case 'a':
				options.autosync = 1;
				options.timecode = 0;
				break;
			case 'A':
				options.autosync = 0;
				options.timecode = 1;
				break;
			case 'q':
				quick = 1;
				showsummary = 1;
				break;
			case 'h':
				usage(options.progname);
				return 0;
			case '?':
				usage(options.progname);
				return 1;
		}
	}
	if(-1 == showsummary)
	{
		if(0 == showhex && 0 == showpackets && 0 == showdetail)
		{
			showsummary = 1;
		}
		else
		{
			showsummary = 0;
		}
	}
	if(argc < 2)
	{
		usage(options.progname);
		return 1;
	}
	pmtcount = 0;
	patseen = 0;
	stream = ts_stream_create(&options);
	for(c = optind; c < argc; c++)
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
			while(-1 != ts_stream_read_packetf(stream, &packet, fin))
			{
				if(packet.sync != 0x47)
				{
					fprintf(stderr, "%s: Sync byte does not match (expected 0x47, found 0x%02x)\n", options.filename, packet.sync);
				}
				if(1 == showpackets)
				{
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
					putchar('\n');
				}
				if(1 == showhex)
				{
					hexdump(&packet);
				}
				if(1 == showdetail)
				{
					if(packet.curtable)
					{
						dumptable(stream, packet.curtable, 0);
					}
				}
				if(packet.curtable && TID_PMT == packet.curtable->tableid)
				{
					pmtcount++;
				}
				else if(packet.curtable && TID_PAT == packet.curtable->tableid)
				{
					patseen = 1;
				}
				if(1 == quick && patseen && pmtcount)
				{
					/* We've read a PAT and at least one PMT, bail out */
					c = argc;
					break;
				}
			}
			if(fin != stdin)
			{
				fclose(fin);
			}
		}
	}
	if(1 == showsummary)
	{
		printf("Stream summary:\n");
		if(NULL == stream->pat)
		{
			printf("- Stream has no Program Association Table\n");
		}
		else
		{
			dumptable(stream, stream->pat, (quick ? 2 : 1));
		}
	}
	return r;
}
