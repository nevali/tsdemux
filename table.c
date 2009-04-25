#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts__table_decode(ts_packet_t *packet, uint16_t progid)
{
	uint8_t *bufp;
	size_t datalen;
	
	bufp = packet->payload;
	datalen = packet->payloadlen;
	if(packet->unitstart)
	{
		datalen -= bufp[0] + 1;
		bufp += bufp[0] + 1;
	}
	if(NULL == packet->table)
	{
		packet->table = packet->stream->allocmem(sizeof(ts_table_t));
	}
	else
	{
		memset(&(packet->table), 0, sizeof(ts_table_t));
	}
	packet->table->progid = progid;
	packet->table->tableid = bufp[0];
	/* If the high bit of the next byte is 1 (the section syntax indicator),
	 * it indicates that the full table header is present.
	 */
	packet->table->syntax = (bufp[1] & 0x80) >> 7;
	packet->table->seclen = ((bufp[1] & 0x0f) << 8) | bufp[2];
	bufp += 3;
	datalen -= 3;
	if(packet->table->syntax)
	{
		packet->table->program = (bufp[0] << 8) | bufp[1];
		packet->table->version = (bufp[2] & 0x3e) >> 1;
		packet->table->curnext = (bufp[2] & 0x01);
		packet->table->section = bufp[3];
		packet->table->last = bufp[4];
		bufp += 5;
		datalen -= 5 + 4;
		packet->table->crc32 = (bufp[datalen] << 24) | (bufp[datalen + 1] << 16) | (bufp[datalen + 2] << 8) | (bufp[datalen + 3]);
		printf(" - Table id 0x%02x, syntax %d, seclen 0x%02x, tsid/progid 0x%04x, version 0x%02x, curnext %d, section 0x%02x, last 0x%02x\n",
			packet->table->tableid, packet->table->syntax, packet->table->seclen, packet->table->program, packet->table->version,
			packet->table->curnext, packet->table->section, packet->table->last);
	}
	else
	{
		printf(" - Table id 0x%02x, syntax %d, seclen 0x%02x\n",
			packet->table->tableid, packet->table->syntax, packet->table->seclen);
	}
	packet->table->data = bufp;
	packet->table->datalen = datalen;
	switch(packet->table->tableid)
	{
		case TID_PAT:
			return ts__pat_decode(packet);
		case TID_PMT:
			return ts__pmt_decode(packet);
			break;
		default:
			printf(" -- Unhandled table 0x%02x\n", packet->table->tableid);
	}
	return 0;
}