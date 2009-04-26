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
	ts_pidinfo_t *pidinfo;
	
	pat = &(packet->curtable->d.pat);
	pat->nprogs = nentry = (packet->curtable->seclen - 9) / 4;
	pat->progs = (ts_table_t **) packet->stream->allocmem(pat->nprogs * sizeof(ts_table_t *));
	c = 0;
	bufp = packet->curtable->data; 
	while(nentry)
	{
		prog = (bufp[0] << 8) | bufp[1];
		pid = ((bufp[2] << 8) | bufp[3]) & 0x1fff;
		bufp += 4;
		nentry--;
		if(NULL == (pat->progs[c] = ts_stream_table_get(packet->stream, TID_PMT, pid)))
		{
			if(0 == prog)
			{
				pat->progs[c] = ts__stream_table_expect(packet->stream, TID_DVB_NIT, pid);
			}
			else
			{
				pat->progs[c] = ts__stream_table_expect(packet->stream, TID_PMT, pid);
			}
		}
		pat->progs[c]->progid = prog;
		/* Add the PID entry */
		if(NULL == (pidinfo = ts_stream_pid_get(packet->stream, pid)))
		{
			pidinfo = ts__stream_pid_add(packet->stream, pid);
		}
		if(PT_SECTIONS != pidinfo->pidtype && PT_UNSPEC != pidinfo->pidtype)
		{
			ts__stream_pid_reset(pidinfo);
		}
		pidinfo->pidtype = PT_SECTIONS;
		pidinfo->defined = 1;
		pat->progs[c]->progid = prog;
		c++;
		pc++;
		if(0 == prog)
		{
			packet->stream->nitpid = pid;
		}
	}
	packet->stream->pat = packet->curtable;
	return 0;
}
