#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts__packet_decode(ts_packet_t *packet)
{
	uint16_t progid;
	int n;
	
	progid = 0;
	if(packet->pid == PID_NULL)
	{
		return 0;
	}
	if(NULL == (packet->pidinfo = ts_stream_pid_get(packet->stream, packet->pid)))
	{
		packet->pidinfo = ts__stream_pid_add(packet->stream, packet->pid);
	}
	packet->pidinfo->seen = 1;
	if(PT_SECTIONS == packet->pidinfo->pidtype)
	{
		while(packet->plofs < packet->payloadlen)
		{
			if(1 != ts__table_decode(packet, progid))
			{
				break;
			}
		}
		return 0;
	}
	/* Elementary stream? */
	return 0;
}