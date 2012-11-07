#pragma once
#include <tlv_parser/tlv.h>
#include <tlv_parser/tlv_cmd_qstring.h>
#include <vector>
#include <string>

struct DecodedCommand : tlv::StringCommand
{
	enum { e_max_sz = 2048 };
	char orig_message[e_max_sz];
	bool written_hdr;
	bool written_payload;

	DecodedCommand () : StringCommand() { Reset(); }

	void Reset ()
	{
		written_hdr = written_payload = false;
		hdr.Reset();
		tvs.clear();
	}
};

