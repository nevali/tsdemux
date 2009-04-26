#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

static int ts__pmt_addpid(ts_stream_t *stream, ts_table_t *pmt, ts_pidinfo_t *info);

int
ts__pmt_decode(ts_packet_t *packet)
{
	uint16_t pcrpid, epid;
	uint8_t *bufp, stype, ptype, pstype;
	size_t datalen, desclen, infolen;
	ts_pidinfo_t *pidinfo;
	const ts_streamtype_t *st;
	
	bufp = packet->curtable->data;
	datalen = packet->curtable->seclen - 9;
	packet->curtable->d.pmt.pcrpid = pcrpid = ((bufp[0] & 0x1f) << 8) | bufp[1];
	if(NULL == (pidinfo = ts_stream_pid_get(packet->stream, pcrpid)))
	{
		pidinfo = ts__stream_pid_add(packet->stream, pcrpid);
	}
	if((PT_UNSPEC == pidinfo->pidtype) || packet->pid != pidinfo->pmtpid)
	{
		ts__stream_pid_reset(pidinfo);
		pidinfo->pidtype = PT_PES;
		pidinfo->subtype = PST_SI;
	}
	pidinfo->defined = 1;
	pidinfo->pcr = 1;
	pidinfo->pmtpid = packet->pid;
	ts__pmt_addpid(packet->stream, packet->curtable, pidinfo);
	desclen = ((bufp[2] & 0x0f) << 8) | bufp[3];
	bufp += 4;
	datalen -= 4;
	/* Descriptors go here */
	bufp += desclen;
	datalen -= desclen;
	while(datalen)
	{
		stype = bufp[0];
		bufp++;
		datalen--;
		epid = ((bufp[0] & 0x1f) << 8) | bufp[1];
		bufp += 2;
		datalen -= 2;
		infolen = ((bufp[0] & 0x0f) << 8) | bufp[1];
		bufp += 2;
		datalen -= 2;
		/* Add the PID entry */
		if(NULL == (st = ts_typeinfo(stype)))
		{
			ptype = PT_UNSPEC;
			pstype = PST_UNSPEC;
		}
		else
		{
			ptype = st->pidtype;
			pstype = st->subtype;
		}
		if(NULL == (pidinfo = ts_stream_pid_get(packet->stream, epid)))
		{
			pidinfo = ts__stream_pid_add(packet->stream, epid);
		}
		if((ptype != pidinfo->pidtype && PT_UNSPEC != pidinfo->pidtype) ||
			packet->pid != pidinfo->pmtpid)
		{
			ts__stream_pid_reset(pidinfo);
		}
		pidinfo->defined = 1;
		pidinfo->stype = stype;
		pidinfo->pidtype = ptype;
		pidinfo->subtype = pstype;
		pidinfo->pmtpid = packet->pid;
		ts__pmt_addpid(packet->stream, packet->curtable, pidinfo);
/*		printf(" -- ES: type 0x%02x, PID 0x%04x, info length 0x%04x\n", stype, epid, infolen); */
		bufp += infolen;
		if(infolen > datalen)
		{
			printf(" - infolen (0x%02x) is greater than datalen (0x%02x)\n", infolen, datalen);
			return -1;
		}
		datalen -= infolen;
	}
	return 0;
}

static int
ts__pmt_addpid(ts_stream_t *stream, ts_table_t *table, ts_pidinfo_t *info)
{
	ts_pidinfo_t **q;
	size_t c;
	
	for(c = 0; c < table->d.pmt.nes; c++)
	{
		if(table->d.pmt.es[c] == info)
		{
			return 0;
		}
	}
	q = (ts_pidinfo_t **) stream->reallocmem(table->d.pmt.es, sizeof(ts_pidinfo_t *) * (table->d.pmt.nes + 1));
	q[table->d.pmt.nes] = info;
	table->d.pmt.es = q;
	table->d.pmt.nes++;
	return 0;
}

