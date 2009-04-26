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
# define ES_TYPE_DSMCC                 0x08
# define ES_TYPE_AUXILIARY             0x09
# define ES_TYPE_DSMCC_ENCAP           0x0a
# define ES_TYPE_DSMCC_UN              0x0b
# define ES_TYPE_AAC                   0x0f
# define ES_TYPE_MPEG4V                0x10
# define ES_TYPE_LATM_AAC              0x11
# define ES_TYPE_MPEG4_GENERIC         0x12
/* Unknown 0x13 */
# define ES_TYPE_DSMCC_DOWNLOAD        0x14
/* Unknown 0x15 */
/* Unknown 0x16 */
/* Unknown 0x17 */
/* Unknown 0x1a */
# define ES_TYPE_H264                  0x1b
# define ES_TYPE_AC3                   0x81
# define ES_TYPE_EAC3                  0x87
# define ES_TYPE_DVB_SLICE             0x90
# define ES_TYPE_DIRAC                 0xd1
# define ES_TYPE_VC1                   0xea

# define TID_PAT                       0x00
# define TID_CAT                       0x01
# define TID_PMT                       0x02
# define TID_DVB_NIT                   0x40
# define TID_DVB_ONIT                  0x41

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

# define PT_UNSPEC                     0
# define PT_SECTIONS                   1
# define PT_PES                        2
# define PT_DATA                       3
# define PT_NULL                       4

# define PST_UNSPEC                    0
# define PST_VIDEO                     1
# define PST_AUDIO                     2
# define PST_INTERACTIVE               3
# define PST_CC                        4
# define PST_IP                        5
# define PST_SI                        6
# define PST_NI                        7

typedef struct ts_stream_struct ts_stream_t;
typedef struct ts_options_struct ts_options_t;
typedef struct ts_rawpacket_struct ts_rawpacket_t;
typedef struct ts_adapt_struct ts_adapt_t;
typedef struct ts_packet_struct ts_packet_t;
typedef struct ts_pidinfo_struct ts_pidinfo_t;
typedef struct ts_pat_struct ts_pat_t;
typedef struct ts_pmt_struct ts_pmt_t;
typedef struct ts_table_struct ts_table_t;
typedef struct ts_streamtype_struct ts_streamtype_t;

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

/* ts_stream_t: represents the entire transport stream */
struct ts_stream_struct
{
	const ts_options_t *opts;
	uint64_t seq;
	size_t lastsync;
	/* The current program association table */
	ts_table_t *pat;
	/* The list of all known tables */
	size_t ntables;
	ts_table_t **tables;
	/* The list of known PIDs */
	size_t npids;
	ts_pidinfo_t **pidinfo;
	/* Well-known PIDs */
	uint16_t nitpid;
	uint16_t tdtpid;
	void *(*allocmem)(size_t nbytes);
	void (*freemem)(void *ptr);
	void *(*reallocmem)(void *ptr, size_t nbytes);
};

struct ts_pat_struct
{
	size_t nprogs;
	ts_table_t **progs; /* Reference the PMT of each program */
};

struct ts_pmt_struct
{
	uint16_t pcrpid;
	size_t nes;
	ts_pidinfo_t **es; /* Reference the elementary streams for this program */
};

struct ts_pidinfo_struct
{
	unsigned int seen:1;
	unsigned int defined:1;
	unsigned int pcr:1;
	unsigned int pidtype;
	unsigned int subtype;
	uint16_t pid;
	/* If it's a PES PID: */
	uint16_t pmtpid; /* PID of the programme this PES relates to */
	uint8_t stype;   /* Stream type from the PMT */
	uint8_t streamid;
};

struct ts_adapt_struct
{
	uint8_t len;
	unsigned int discontinuity:1;
	unsigned int random:1;
	unsigned int priority:1;
	unsigned int pcr:1;
	unsigned int opcr:1;
	unsigned int splicepoint:1;
	unsigned int privdata:1;
	unsigned int extensions:1;
};

struct ts_table_struct
{
	ts_table_t *prev, *next;
	uint16_t pid; /* The PID this table was last carried in */
	unsigned int expected:1; /* This table hasn't been defined yet */
	size_t occurrences;
	uint16_t progid; /* The progid this table was associated with, via the PAT */
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
	union
	{
		ts_pat_t pat;
		ts_pmt_t pmt;
	} d;
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
	ts_pidinfo_t *pidinfo;
	ts_adapt_t af; /* If hasaf is set */
	size_t ntables;
	ts_table_t *tables[8]; /* If this packet contains tables, pointers to them */
	ts_table_t *curtable;
	size_t payloadlen;
	size_t plofs;
	uint8_t payload[184]; /* If haspd is set */
};

struct ts_streamtype_struct
{
	uint8_t stype;
	uint8_t pidtype;
	uint8_t subtype;
	const char *name;
	const char *mime;
	const char *ext;
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
	
	/* Retrieve the table with the given table_id (and optional PID) */
	ts_table_t *ts_stream_table_get(ts_stream_t *stream, uint8_t tableid, uint16_t pid);
	
	/* Retrieve the metadata for a particular PID */
	ts_pidinfo_t *ts_stream_pid_get(ts_stream_t *stream, uint16_t pid);

	/* Retrieve information about a given defined stream type */
	const ts_streamtype_t *ts_typeinfo(uint8_t stype);
	
# ifdef __cplusplus
}
# endif

#endif /*!TSDEMUX_H_ */