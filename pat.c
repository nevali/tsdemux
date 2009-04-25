#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts__pat_decode(ts_packet_t *packet)
{
	uint8_t *bufp;
	uint16_t prog, pid;
	size_t c, pc, nentry;
	ts_pat_t *pat;
	
	printf("Program Association Table:\n");

	if(NULL != packet->stream->pat && packet->stream->pat->version == packet->table->version)
	{
		printf(" -- Repeated PAT ver 0x%02x\n", packet->table->version);
		packet->stream->pat->occurrences++;
		return 0;
	}
	pat = (ts_pat_t *) packet->stream->allocmem(sizeof(ts_pat_t));
	pat->prev = packet->stream->pat;
	pat->occurrences = 1;
	pat->version = packet->table->version;
	
	pat->nprogs = pat->count = nentry = (packet->table->seclen - 9) / 4;;
	pat->progs = (ts_prog_t **) packet->stream->allocmem(pat->nprogs * sizeof(ts_prog_t *));
	pc = ts__stream_addprogs(packet->stream, pat->nprogs);
	printf(" -- Section length is 0x%02x, payload size is 0x%02x, %u entries\n", packet->table->seclen, (unsigned) packet->table->datalen, (unsigned) nentry);
	c = 0;
	bufp = packet->table->data; 
	while(nentry)
	{
		prog = (bufp[0] << 8) | bufp[1];
		pid = ((bufp[2] << 8) | bufp[3]) & 0x1fff;
		bufp += 4;
		nentry--;
		packet->stream->progs[pc].progid = prog;
		packet->stream->progs[pc].pid = pid;
		pat->progs[c] = &(packet->stream->progs[pc]);
		c++;
		pc++;
		if(0 == prog)
		{
			printf(" -- Network Information Table has PID 0x%04x\n", pid);
			packet->stream->nit = pid;
		}
		else
		{
			printf(" -- Program 0x%04x has PID 0x%04x\n", prog, pid);
		}
	}
	packet->stream->pat = pat;
	return 0;
}
