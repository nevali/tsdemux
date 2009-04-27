#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts__table_decode(ts_packet_t *packet, uint16_t progid)
{
	uint8_t *bufp;
	size_t skip;
	ts_table_t table, *tp, *tpp;
	
	memset(&table, 0, sizeof(table));
	bufp = &(packet->payload[packet->plofs]);
	if(0 == packet->plofs && 1 == packet->unitstart)
	{
		packet->plofs += bufp[0] + 1;
		bufp += bufp[0] + 1;
	}
	/* First, read the table header */
	table.pid = packet->pid;
	table.progid = progid;
	table.tableid = bufp[0];
	if(0xff == table.tableid)
	{
		/* End of sections in this packet */
		return 0;
	}
	/* If the high bit of the next byte is 1 (the section syntax indicator),
	 * it indicates that the full table header is present.
	 */
	table.syntax = (bufp[1] & 0x80) >> 7;
	table.seclen = ((bufp[1] & 0x0f) << 8) | bufp[2];
	table.datalen = table.seclen;
	bufp += 3;
	packet->plofs += 3 + table.seclen;
	/* Point packet->payload at the first byte after the table */
	if(table.syntax)
	{
		table.program = (bufp[0] << 8) | bufp[1];
		table.version = (bufp[2] & 0x3e) >> 1;
		table.curnext = (bufp[2] & 0x01);
		table.section = bufp[3];
		table.last = bufp[4];
		table.datalen -= 5;
		bufp += 5;
/*		table.crc32 = (bufp[datalen] << 24) | (bufp[datalen + 1] << 16) | (bufp[datalen + 2] << 8) | (bufp[datalen + 3]); */
/*		printf(" - Table id 0x%02x, syntax %d, seclen 0x%02x, tsid/progid 0x%04x, version 0x%02x, curnext %d, section 0x%02x, last 0x%02x\n",
			table.tableid, table.syntax, table.seclen, table.program, table.version,
			table.curnext, table.section, table.last); */
	}
/*	else
	{
		printf(" - Table id 0x%02x, syntax %d, seclen 0x%02x\n",
			table.tableid, table.syntax, table.seclen);
	} */
	table.data = bufp;
	/* Now check if we have an existing table which matches */
	if(NULL != (tp = ts_stream_table_get(packet->stream, table.tableid, packet->pid)))
	{
		/* If tp->expected is 1, ts__stream_table_add will replace *tp with
		 * the table data we supply, rather than adding our data to the end
		 * of a list.
		 */
		if(0 == table.progid)
		{
			/* Preserve any program number associations from the PAT */
			table.progid = tp->progid;
		}
		if(0 == tp->expected)
		{
			for(tpp = tp; NULL != tpp; tpp = tpp->next)
			{
				if(tpp->version == table.version)
				{
					tpp->occurrences++;
					if(1 == table.curnext && 0 == tpp->curnext)
					{
						printf(" - Activating table\n");
						ts__stream_table_activate(packet->stream, tpp);
						return 1;
					}
					packet->tables[packet->ntables] = tp;
					packet->ntables++;
/*					printf(" - Repeated table version 0x%02x, skipping\n", table.version); */
					return 1;
				}
			}
			/* This shouldn't ever happen, but we might as well check */
			for(tpp = tp; NULL != tp; tp = tp->prev)
			{
				if(tpp->version == table.version)
				{
					tpp->occurrences++;
					printf(" - Repeated out of date table version 0x%02x, skipping\n", table.version);
					return 1;
				}
			}
		}
		/* We're carrying a new version of the table, add it below */
	}
	tp = ts__stream_table_add(packet->stream, &table, tp);
	packet->tables[packet->ntables] = tp;
	packet->ntables++;
	packet->curtable = tp;
	tp++;
	switch(table.tableid)
	{
		case TID_PAT:
			return ts__pat_decode(packet);
		case TID_PMT:
			return ts__pmt_decode(packet);
			break;
		default:
			printf(" -- Unhandled table 0x%02x\n", table.tableid);
	}
	return 1;
}

ts_table_t *
ts_stream_table_get(ts_stream_t *stream, uint8_t tableid, uint16_t pid)
{
	size_t c;

	for(c = 0; c < stream->ntables; c++)
	{
		if(tableid == stream->tables[c]->tableid)
		{
			if(PID_UNSPEC == pid || pid == stream->tables[c]->pid)
			{
				return stream->tables[c];
			}
		}
	}
	return NULL;
}

ts_table_t *
ts__stream_table_add(ts_stream_t *stream, const ts_table_t *src, ts_table_t *ref)
{
	ts_table_t *p, **q, *n;
	size_t c;
	
	if(NULL != ref && 1 == ref->expected)
	{
		p = ref->prev;
		n = ref->next;
		memcpy(ref, src, sizeof(ts_table_t));
		ref->prev = p;
		ref->next = n;
		if(TID_PAT == ref->tableid && 1 == ref->curnext)
		{
			stream->pat = ref;
		}
		return ref;
	}
	p = (ts_table_t *) stream->allocmem(sizeof(ts_table_t));
	memcpy(p, src, sizeof(ts_table_t));
	if(NULL == ref)
	{
		q = (ts_table_t **) stream->reallocmem(stream->tables, sizeof(ts_table_t *) * (stream->ntables + 1));
		q[stream->ntables] = p;
		stream->tables = q;
		stream->ntables++;
	}
	else
	{
		p->prev = ref;
		ref->next = p;
		if(1 == p->curnext)
		{
			for(n = ref; NULL != n && 0 == n->curnext; n = n->prev);
			if(NULL == n)
			{
				for(n = ref; NULL != n->prev; n = n->prev);
			}
			n->curnext = 0;
			for(c = 0; c < stream->ntables; c++)
			{
				if(stream->tables[c] == n)
				{
					stream->tables[c] = p;
					break;
				}
			}
		}
	}
	if(TID_PAT == p->tableid && 1 == p->curnext)
	{
		stream->pat = p;
	}
	return p;
}

ts_table_t *
ts__stream_table_expect(ts_stream_t *stream, uint8_t tableid, uint16_t pid)
{
	ts_table_t *p, **q;
	
	p = (ts_table_t *) stream->allocmem(sizeof(ts_table_t));
	p->tableid = tableid;
	p->pid = pid;
	p->expected = 1;
	q = (ts_table_t **) stream->reallocmem(stream->tables, sizeof(ts_table_t *) * (stream->ntables + 1));
	q[stream->ntables] = p;
	stream->tables = q;
	stream->ntables++;
	return p;
}

int
ts__stream_table_activate(ts_stream_t *stream, ts_table_t *table)
{
	printf("Activating table 0x%02x (PID 0x%04x)\n", table->tableid, table->pid);
	return 0;
}
