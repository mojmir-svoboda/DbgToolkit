#include "connection.h"
#include <boost/tokenizer.hpp>
#include "../tlv_parser/tlv_encoder.h"

bool Connection::tryHandleCommand (DecodedCommand const & cmd)
{
	if (cmd.hdr.cmd == tlv::cmd_setup)
		handleSetupCommand(cmd);
	else if (cmd.hdr.cmd == tlv::cmd_log)
		handleLogCommand(cmd);
	else if (cmd.hdr.cmd == tlv::cmd_scope_entry || cmd.hdr.cmd == tlv::cmd_scope_exit)
		handleLogCommand(cmd);
	return true;
}

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	printf("handle setup command\n");
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
		if (cmd.tvs[i].m_tag == tlv::tag_app)
			printf("received setup command from application: %s\n", cmd.tvs[i].m_val.c_str());
	return true;
}

bool Connection::handleLogCommand (DecodedCommand const & cmd)
{
	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		char const * const fmts[tlv::tag_max_value] = { "%-16s|", "%-32s|", "%-6s|", "%-10s|", "%-5s|", "%-32s|", "%-4s|", "%-24s|", "%s", "%-2s|", "%-8s|" };
		for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
		{
			tlv::tag_t const tag = cmd.tvs[i].m_tag;
			std::string const & val = cmd.tvs[i].m_val;

			if (cmd.tvs[i].m_tag == tlv::tag_file) continue;
			if (cmd.tvs[i].m_tag == tlv::tag_line) continue;
			if (cmd.tvs[i].m_tag == tlv::tag_ctx) continue;

			printf(fmts[tag], val.c_str());
		}
		printf("\n"); fflush(stdout);
	}
	return true;
}

