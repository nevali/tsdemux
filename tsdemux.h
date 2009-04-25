#ifndef TSDEMUX_H_
# define TSDEMUX_H_                    1

# include <stdio.h>
# include <inttypes.h>

# define PID_PAT                       0x0000
# define PID_CAT                       0x0001
# define PID_TSDT                      0x0002

# define PID_DVB_NIT                   0x0010
# define PID_DVB_SDT                   0x0011
# define PID_DVB_EIT                   0x0012
# define PID_DVB_RST                   0x0013
# define PID_DVB_TDT                   0x0014
# define PID_DVB_SYNC                  0x0015
# define PID_DVB_INBAND                0x001c
# define PID_DVB_MEASUREMENT           0x001d
# define PID_DVB_DIT                   0x001e
# define PID_DVB_SIT                   0x001f

# define PID_NULL                      0x1fff

# define PID_UNSPEC                    0xffff

# define ES_TYPE_MPEG1V                0x01
# define ES_TYPE_MPEG2V                0x02
# define ES_TYPE_MPEG1A                0x03
# define ES_TYPE_MPEG2A                0x04
# define ES_TYPE_PRIVATESECTS          0x05
# define ES_TYPE_PRIVATEDATA           0x06
# define ES_TYPE_MHEG                  0x07
# define ES_TYPE_DSMCC_UN              0x0b
# define ES_TYPE_AAC                   0x0f
# define ES_TYPE_H264                  0x1b
# define ES_TYPE_AC3                   0x81
# define ES_TYPE_EAC3                  0x87
# define ES_TYPE_DVB_SLICE             0x90
# define ES_TYPE_DIRAC                 0xd1
# define ES_TYPE_VC1                   0xea
# define TID_PAT                       0x00
# define TID_CAT                       0x01
# define TID_PMT                       0x02

# define DESC_VIDEO                    0x02
# define DESC_AUDIO                    0x03
# define DESC_HIERARCHY                0x04
# define DESC_REGISTRATION             0x05
# define DESC_DSA                      0x06 /* data stream alignment */
# define DESC_TBG                      0x07 /* target background grid */
# define DESC_VIDEOWINDOW              0x08
# define DESC_CA                       0x09
# define DESC_LANGUAGE                 0x0a
# define DESC_CLOCK                    0x0b
# define DESC_MBU                      0x0c /* multiplex buffer utilisation */
# define DESC_COPYRIGHT                0x0d
# define DESC_MAXBITRATE               0x0e

typedef struct ts_stream_struct ts_stream_t;
typedef struct ts_options_struct ts_options_t;
typedef struct ts_rawpacket_struct ts_rawpacket_t;
typedef struct ts_adapt_struct ts_adapt_t;
typedef struct ts_packet_struct ts_packet_t;
typedef struct ts_esinfo_struct ts_esinfo_t;
typedef struct ts_pat_struct ts_pat_t;
typedef struct ts_prog_struct ts_prog_t;
typedef struct ts_table_struct ts_table_t;

struct ts_options_struct
{
	unsigned int timecode:1;
	unsigned int autosync:1;
	uint8_t prepad;
	uint8_t postpad;
	uint16_t synclimit;
	const char *filename;
	void *(*allocmem)(size_t nbytes);
	void (*freemem)(void *ptr);
	void *(*reallocmem)(void *ptr, size_t nbytes);
};

struct ts_stream_struct
{
	const ts_options_t *opts;
	uint64_t seq;
	size_t lastsync;
	ts_pat_t *pat;
	size_t nesinfo;
	ts_esinfo_t *esinfo;
	uint16_t nit;
	uint16_t tdt;
	size_t nes;
	ts_esinfo_t *es;
	size_t nprogs;
	ts_prog_t *progs;
	void *(*allocmem)(size_t nbytes);
	void (*freemem)(void *ptr);
	void *(*reallocmem)(void *ptr, size_t nbytes);
};

struct ts_pat_struct
{
	ts_pat_t *prev;
	uint16_t version;
	size_t occurrences;
	size_t nprogs;
	size_t count;
	ts_prog_t **progs;
};

struct ts_prog_struct
{
	uint16_t progid;
	uint16_t pid;
	ts_esinfo_t **es;
	size_t nes;
};

struct ts_esinfo_struct
{
	uint16_t progid;
	uint8_t stype;
	uint16_t pid;
};

struct ts_adapt_struct
{
	uint8_t len;
	int discontinuity:1;
	int random:1;
	int priority:1;
	int pcr:1;
	int opcr:1;
	int splicepoint:1;
	int privdata:1;
	int extensions:1;
};

struct ts_table_struct
{
	uint16_t progid;
	uint8_t tableid;
	unsigned int syntax:1;
	uint16_t seclen;
	uint16_t program;
	uint8_t version;
	unsigned int curnext:1;
	uint8_t section;
	uint8_t last;
	uint8_t *data;
	size_t datalen;
	uint32_t crc32;
};

struct ts_packet_struct
{
	ts_stream_t *stream;
	uint32_t timecode;
	uint8_t sync;
	int transerr;
	int unitstart;
	int priority;
	uint16_t pid;
	int sc;
	int hasaf;
	int haspd;
	unsigned int continuity:1;
	ts_adapt_t af; /* If hasaf is set */
	ts_table_t *table;
	size_t payloadlen;
	uint8_t payload[184]; /* If haspd is set */
};


# ifdef __cplusplus
extern "C" {
# endif

	ts_stream_t *ts_stream_create(const ts_options_t *opts);
	
	/* Read a packet from a file and then decode it, dealing with padding
	 * based upon the options associated with the stream.
	 */
	int ts_stream_read_packetf(ts_stream_t *stream, ts_packet_t *dest, FILE *src);

	/* Process a pre-read packet. The packet must be 188 bytes long and start
	 * with a sync byte, unless the stream options indicate that a timecode
	 * is present, in which case the packet will be prepended with a 32-bit
	 * timecode value, resulting in a total of 192 bytes.
	 */
	int ts_stream_read_packet(ts_stream_t *stream, ts_packet_t *dest, const uint8_t *bufp);
	

# ifdef __cplusplus
}
# endif

#endif /*!TSDEMUX_H_ */