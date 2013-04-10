#include "assocvector.h"
#include <vector>
#include <cstdio>

struct src {
	typedef std::vector<int> row_t;
	std::vector<row_t> m_rows;

	void dump ()
	{
		for (size_t y = 0, ye = m_rows.size(); y < ye; ++y)
		{
			for (size_t x = 0, xe = m_rows[y].size(); x < xe; ++x)
			{
				if (m_rows[y][x] == 0)
					printf("... |");
				else
					printf("%03i |", m_rows[y][x]);
			}
			printf("\n");
		}
	}

	void appendTableXY (int x, int y, int val)
	{
		printf("append x=%i y=%i val=%i\n", x, y, val);

		{   
			if (y >= m_rows.size())
			{   
				size_t const curr_sz = m_rows.size();
				m_rows.resize(y + 1);
			}
			else
			{
				if (x >= m_rows[y].size())
				{
					m_rows[y].resize(x + 1);
				}
			}
				
			{
				if (x + 1 >= m_rows.back().size())
				{
					m_rows.back().resize(x + 1);
				}
			}
		}
		
		for (int ix = x, ixe = x + 1; ix < ixe; ++ix)
		{   
			m_rows[y][ix] = val;
		}
	}
};

src s;

struct pxy {

	typedef Loki::AssocVector<int, int> map_t;
	Loki::AssocVector<int, int> m_from_src;
	std::vector<int> m_from_tgt;
	//Loki::AssocVector<int, int> m_from_tgt;

	void setData (int x, int y, int val)
	{
		bool acc_c = filterAcceptsColumn(x);
		if (acc_c)
		{
			//printf("\t### yay\n");
			insertColumn(x);
		}
		else
			printf("\t### nay\n");

		//else
		//	removeColumn(x);
	}

	bool filterAcceptsColumn (int sourceColumn)
	{
		bool empty = true;
		for (size_t y = 0, ye = s.m_rows.size(); y < ye; ++y)
			if (sourceColumn < s.m_rows[y].size())
				if (s.m_rows[y][sourceColumn] != 0)
				{
					empty = false;
					break;
				}
	}

	void beginInsertColumns (int first, int last)
	{
		printf("\tbeginInsertColumns(%i, %i)\n", first, last);
	}

	void insertColumn (int c)
	{
		int pos0 = 0;
		int pos1 = m_from_src.size();
		map_t::iterator it0 = m_from_src.lower_bound(c);
		map_t::iterator it1 = m_from_src.upper_bound(c);


		if (it0 != m_from_src.end() && (it1 != m_from_src.end() && it0->first == c))
		{
			//printf("\t!!!already there\n");
			return;
		}

		if (it0 != m_from_src.end())
		{
			//printf("\t* LW (%i, %i)\n", it0->first, it0->second);
			pos0 = it0->second;
		}
		if (it1 != m_from_src.end())
		{
			//printf("\t* UP (%i, %i)\n", it1->first, it1->second);
			pos1 = it1->second;

			for (map_t::iterator iit = it1, iite = m_from_src.end(); iit != iite; ++iit)
			{
				iit->second += 1;
			}
		}

		//printf("\t* pos0=%i pos1=%i\n", pos0, pos1);
		beginInsertColumns(pos0, pos1);
		m_from_src.insert(std::make_pair(c, pos1));

		m_from_tgt.resize(m_from_src.size()); // ugh
		for (map_t::const_iterator it = m_from_src.begin(), ite = m_from_src.end(); it != ite; ++it)
			m_from_tgt[it->second] = it->first;
	}

	void insertRow (int r)
	{
	}

	void dump()
	{
		for (map_t::const_iterator it = m_from_src.begin(), ite = m_from_src.end(); it != ite; ++it)
		{
			printf("(%i -> %i)\n", it->first, it->second);
		}

		for (size_t i = 0, ie = m_from_tgt.size(); i != ie; ++i)
			printf("from_tgt[%i]=%i\n", i, m_from_tgt[i]);
	}
};

pxy p;

int main ()
{

	for (;;)
	{
		static size_t i	 = 0;
		s.appendTableXY((2 * i) % 16, i * 2, i + 100);
		p.setData((2 * i) % 16, i * 2, i + 100);
		//p.setData(i, i, i + 100);
		printf("\n");
		//p.setData((2 * i) % 16, i * 2, i + 100);
		++i;


		if (i == 6)
		{
			printf("### 666 ###\n");
			s.appendTableXY(3, 1, 666);
			p.setData(3, 1, 666);
			printf("\n");

			s.appendTableXY(7, 2, 777);
			p.setData(7, 2, 777);
			printf("\n");

			s.appendTableXY(8, 1, 888);
			p.setData(7, 1, 888);
			printf("\n");

		}
		
		if (i == 6)
			break;
	}


	s.dump();
	p.dump();

}
