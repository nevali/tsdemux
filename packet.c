#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts_decode_nit(ts_packet_t *packet)
{
	printf(" -- Network Information Table\n");
	return 0;
}

int
ts__packet_decode(ts_packet_t *packet)
{
	size_t c;
	uint16_t progid;
	int table;
	
	table = 0;
	progid = 0;
	if(packet->pid == PID_PAT || packet->pid == PID_CAT)
	{
		table = 1;
	}
	else if(packet->pid == PID_NULL)
	{
		/* Do nothing */
		return 0;
	}
	else if(packet->pid == packet->stream->nit)
	{
		table = 1;
	}
	else
	{
		for(c = 0; c < packet->stream->nprogs; c++)
		{
			if(packet->stream->progs[c].pid == packet->pid)
			{
				table = 1;
				progid = packet->stream->progs[c].progid;
				break;
			}
		}
	}
	if(1 == table)
	{
		return ts__table_decode(packet, progid);
	}
	/* Elementary stream? */
	return 0;
}