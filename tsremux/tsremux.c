/* tsdump - Output a compliant stream from an autosynced source */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "tsdemux.h"

int
main(int argc, char **argv)
{
	ts_stream_t *stream;
	ts_options_t options;
	ts_packet_t packet;
	uint8_t *bp;
	
	(void) argc;
	(void) argv;
	
	memset(&options, 0, sizeof(options));
	options.progname = argv[0];
	options.timecode = 0;
	options.autosync = 1;
	options.synclimit = 256;
	options.prepad = 0;
	options.postpad = 0;
	options.filename = "*Standard input*";
	stream = ts_stream_create(&options);
	while(-1 != ts_stream_read_packetf(stream, &packet, stdin))
	{
		bp = &(packet.payload[packet.plstart]);
		fwrite(bp, 188, 1, stdout);
	}
	return 0;
}
