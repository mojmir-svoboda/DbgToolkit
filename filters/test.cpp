#include <cstdio>
#include <string>
#include <boost/tokenizer.hpp>

enum E_NodeStates {
	  e_Unchecked
	, e_PartialCheck
	, e_Checked
};

#include "file_filter.hpp"

struct TreeModelItem {
	/*@member	state
	 * duplicates qt enum
	 *	Qt::Unchecked	0	The item is unchecked.
	 *	Qt::PartiallyChecked	1	The item is partially checked. Items in hierarchical models may be partially checked if some, but not all, of their children are checked.
	 *	Qt::Checked	2	The item is checked.
	 */
	int m_state;
	int m_collapsed;

	TreeModelItem () : m_state(e_Unchecked), m_collapsed(true) { }
	TreeModelItem (int s) : m_state(s), m_collapsed(true) { }
	TreeModelItem (int s, bool c) : m_state(s), m_collapsed(c) { }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("state", m_state);
		ar & boost::serialization::make_nvp("collapsed", m_collapsed);
	}
};


std::string s[] = { "c:/devel/aa.cpp:11", "c:/devel/aa.cpp/12", "c:/devel/main.cpp", "c:/devel/bb.cpp" };
//int state[] = { e_Checked, e_Checked, e_PartialCheck, e_Checked };
int state[] = { e_Unchecked, e_Unchecked, e_Unchecked, e_Checked };
//std::string s2[] = { "c:/devel/aa.cpp:21", "c:/devel/aa.cpp/22", "c:/devel/main.cpp/23", "c:/devel/cc.cpp" };
std::string s2[] = { "c:/devel/aa.cpp", "c:/devel/cc.cpp" };
int state2[] = { e_PartialCheck, e_Unchecked, e_Unchecked, e_Checked };

int main ()
{
	typedef tree_filter<TreeModelItem> data_filters_t;
	data_filters_t ff;
	data_filters_t rhs;
	for (size_t i = 0; i < sizeof(s)/sizeof(*s); ++i)
	{
		printf("iteration %i\n", i);
		ff.set_to_state(s[i], TreeModelItem(state[i]));
	}

	for (size_t i = 0; i < sizeof(s2)/sizeof(*s2); ++i)
	{
		printf("iteration %i\n", i);
		rhs.set_to_state(s2[i], TreeModelItem(state2[i]));
	}


	printf("*************\n");
	/*{
		std::string s("c:/devel/aa.cpp");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}

	{
		std::string s("c:/devel/aa.cpp:21");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
		ff.exclude_off(s);
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());

		ff.append(s);
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/devel/aa.cpp/27");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/devel/bb.cpp/27");
		printf("\nexcluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/devel/bb.cpp");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/jewel/bb.cpp");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}*/

	std::string out;
	ff.dump_filter(out);
	printf("\n ff exported: |%s|\n\n", out.c_str());

	std::string out2;
	rhs.dump_filter(out2);
	printf("\nrhs exported: |%s|\n\n", out2.c_str());

	ff.merge_with(rhs);
	printf("merged\n");
	std::string res;
	ff.dump_filter(res);
	printf("dumped\n");
	printf("\n\nresult exported: |%s|\n\n", res.c_str());

}
