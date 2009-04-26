#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts__packet_decode(ts_packet_t *packet)
{
/*	size_t c; */
	uint16_t progid;
	int n;
	
	progid = 0;
	if(packet->pid == PID_NULL)
	{
		/* Do nothing */
		return 0;
	}
	if(NULL == (packet->pidinfo = ts_stream_pid_get(packet->stream, packet->pid)))
	{
		packet->pidinfo = ts__stream_pid_add(packet->stream, packet->pid);
	}
/*	else
	{
		printf("Found previously-defined PID 0x%04x of type %d\n", packet->pid, packet->pidinfo->pidtype);
	} */
	packet->pidinfo->seen = 1;
	if(PT_SECTIONS == packet->pidinfo->pidtype)
	{
		for(n = 0; packet->plofs < packet->payloadlen; n++)
		{
			if(1 != ts__table_decode(packet, progid))
			{
				break;
			}
		}
		return 0;
	}
/*
		for(c = 0; c < packet->stream->nprogs; c++)
		{
			if(packet->stream->progs[c].pid == packet->pid)
			{
				table = 1;
				progid = packet->stream->progs[c].progid;
				break;
			}
		}
	} */
	/* Elementary stream? */
	return 0;
}