#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

ts_pidinfo_t *
ts_stream_pid_get(ts_stream_t *stream, uint16_t pid)
{
	size_t c;
	
	for(c = 0; c < stream->npids; c++)
	{
		if(stream->pidinfo[c]->pid == pid)
		{
			return stream->pidinfo[c];
		}
	}
	return NULL;
}

ts_pidinfo_t *
ts__stream_pid_add(ts_stream_t *stream, uint16_t pid)
{
	ts_pidinfo_t *p, **q;
	
	p = stream->allocmem(sizeof(ts_pidinfo_t));
	q = stream->reallocmem(stream->pidinfo, sizeof(ts_pidinfo_t *) * (stream->npids + 1));
	stream->pidinfo = q;
	stream->pidinfo[stream->npids] = p;
	stream->npids++;
	p->pid = pid;
	p->pidtype = PT_UNSPEC;
	/* For certain PIDs, we know that they will always contain sections rather
	 * than PES packets.
	 */
	switch(pid)
	{
		case PID_PAT:
			p->defined = 1;
			/* Fallthrough */
		case PID_CAT:
		case PID_TSDT:
		case PID_DVB_NIT:
			p->pidtype = PT_SECTIONS;
			break;
		case PID_NULL:
			p->pidtype = PT_NULL;
			break;
	}
	return p;
}

int
ts__stream_pid_reset(ts_pidinfo_t *info)
{
	uint16_t pid;
	
	pid = info->pid;
	memset(info, 0, sizeof(ts_pidinfo_t));
	info->pid = pid;
	info->pidtype = PT_UNSPEC;
	return 0;
}

