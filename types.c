
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

static const ts_streamtype_t streamtypes[] = {
	{ 0, PT_PES, PST_SI, "Standalone PCR", NULL, NULL },
	{ ES_TYPE_MPEG1V, PT_PES, PST_VIDEO, "MPEG-1", "video/mpeg", "mp1" },
	{ ES_TYPE_MPEG2V, PT_PES, PST_VIDEO, "MPEG-2", "video/mpeg", "mp2" },
	{ ES_TYPE_MPEG1A, PT_PES, PST_AUDIO, "MPEG-1", "audio/mp3", "mp3" },
	{ ES_TYPE_MPEG2A, PT_PES, PST_AUDIO, "MPEG-2", "audio/x-mpeg2", "mp2a" },
	{ ES_TYPE_PRIVATESECTS, PT_SECTIONS, PST_UNSPEC, "Private", NULL, NULL },
	{ ES_TYPE_PRIVATEDATA, PT_PES, PST_UNSPEC, "Private", NULL, NULL },
	{ ES_TYPE_MHEG, PT_PES, PST_INTERACTIVE, "ISO 13522 MHEG", "application/x-mheg", "mheg" },
	{ ES_TYPE_DSMCC, PT_DATA, PST_INTERACTIVE, "ISO 13818-1 DSM-CC", NULL, NULL },
	{ ES_TYPE_AUXILIARY, PT_DATA, PST_UNSPEC, "ISO 13818-1 DSM-CC", NULL, NULL },
	{ ES_TYPE_DSMCC_ENCAP, PT_DATA, PST_UNSPEC, "ISO 13818-1 DSM-CC encap", NULL, NULL },
	{ ES_TYPE_DSMCC_UN, PT_DATA, PST_UNSPEC, "ISO 13818-6 DSM-CC User/Network", NULL, NULL },
	{ ES_TYPE_AAC, PT_PES, PST_AUDIO, "ISO 13818-7 AAC", "audio/aac", "aac" },
	{ ES_TYPE_MPEG4V, PT_PES, PST_VIDEO, "MPEG-4", "video/mpeg4", "m4v" },
	{ ES_TYPE_LATM_AAC, PT_PES, PST_AUDIO, "MPEG-4 LATM AAC", "audio/aac", "aac" },
	{ ES_TYPE_MPEG4_GENERIC, PT_PES, PST_UNSPEC, "MPEG-4 generic", NULL, NULL },
	{ ES_TYPE_DSMCC_DOWNLOAD, PT_DATA, PST_UNSPEC, "ISO 13818-1 DSM-CC download", NULL, NULL },
	{ ES_TYPE_H264, PT_PES, PST_VIDEO, "H.264", "video/mp4", "h264" },
	{ ES_TYPE_AC3, PT_PES, PST_AUDIO, "AC-3", "audio/ac3", "ac3" },
	{ ES_TYPE_EAC3, PT_PES, PST_AUDIO, "E-AC-3", "audio/x-eac3", "eac3" },
	{ ES_TYPE_DVB_SLICE, PT_DATA, PST_UNSPEC, "DVB slice", NULL, NULL },
	{ ES_TYPE_DIRAC, PT_PES, PST_VIDEO, "Dirac/VC-2", "video/dirac", "drc" },
	{ ES_TYPE_VC1, PT_PES, PST_VIDEO, "WMV9/VC-1", "video/vc1", "vc1" },
	{ 0, PT_UNSPEC, PST_UNSPEC, NULL, NULL, NULL }
};

const ts_streamtype_t *
ts_typeinfo(uint8_t stype)
{
	size_t c;
	
	for(c = 0; streamtypes[c].name; c++)
	{
		if(streamtypes[c].stype == stype)
		{
			return &(streamtypes[c]);
		}
	}
	return NULL;
}
