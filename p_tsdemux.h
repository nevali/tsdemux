#ifndef P_LIBTS_H_
# define P_LIBTS_H_                    1

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <inttypes.h>
# include <ctype.h>
# include <arpa/inet.h>

# include "tsdemux.h"

# define TS_PACKET_LENGTH              188
# define TS_SYNC_BYTE                  0x47

extern size_t ts__stream_addprogs(ts_stream_t *stream, size_t nprogs);

extern int ts__packet_decode(ts_packet_t *packet);
extern int ts__table_decode(ts_packet_t *packet, uint16_t progid);
extern int ts__pat_decode(ts_packet_t *packet);
extern int ts__pmt_decode(ts_packet_t *packet);
extern int ts__nit_decode(ts_packet_t *packet);
extern int ts__cat_decode(ts_packet_t *packet);

#endif /*!P_LIBTS_H_*/