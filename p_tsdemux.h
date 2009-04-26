#ifndef P_TSDEMUX_H_
# define P_TSDEMUX_H_                  1

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <inttypes.h>
# include <ctype.h>

# include "tsdemux.h"

# define TS_PACKET_LENGTH              188
# define TS_SYNC_BYTE                  0x47

extern ts_pidinfo_t *ts__stream_pid_add(ts_stream_t *stream, uint16_t pid);
extern int ts__stream_pid_reset(ts_pidinfo_t *info);

extern int ts__stream_table_activate(ts_stream_t *stream, ts_table_t *table);
extern ts_table_t *ts__stream_table_add(ts_stream_t *stream, const ts_table_t *table, ts_table_t *prevver);
extern ts_table_t *ts__stream_table_expect(ts_stream_t *stream, uint8_t tableid, uint16_t pid);

extern int ts__packet_decode(ts_packet_t *packet);
extern int ts__table_decode(ts_packet_t *packet, uint16_t progid);
extern int ts__pat_decode(ts_packet_t *packet);
extern int ts__pmt_decode(ts_packet_t *packet);
extern int ts__nit_decode(ts_packet_t *packet);
extern int ts__cat_decode(ts_packet_t *packet);

#endif /*!P_LIBTS_H_*/