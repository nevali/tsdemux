#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

int
ts__pmt_decode(ts_packet_t *packet)
{
	uint16_t pcrpid, epid;
	uint8_t *bufp, stype;
	size_t datalen, desclen, infolen;
	
	printf("Program mapping table:\n");	
	bufp = packet->table->data;
	datalen = packet->table->seclen - 9;
	pcrpid = ((bufp[0] & 0x3f) << 8) | bufp[1];
	printf(" -- PCR PID is 0x%04x\n", pcrpid);
	desclen = ((bufp[2] & 0x0f) << 8) | bufp[3];
	bufp += 4;
	datalen -= 4;
	printf(" -- Descriptor length is 0x%04x\n", desclen);
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
		printf(" -- ES: type 0x%02x, PID 0x%04x, info length 0x%04x\n", stype, epid, infolen);
		bufp += infolen;
		datalen -= infolen;
	}
	return 0;
}
