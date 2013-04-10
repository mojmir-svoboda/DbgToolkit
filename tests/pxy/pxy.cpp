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
				printf("%03i |", m_rows[y][x]);
			printf("\n");
		}
	}

	void appendTableXY (int x, int y, int val)
	{
		printf("append x=%i y=%i val=%i\n", x, y, val);

		{   
			if (y >= m_rows.size())
			{   
				printf("x1\n");
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
					printf("x2\n");
					printf("  append: x>=0  resize %i -> %i\n", m_rows.back().size(), x + 1);
					m_rows.back().resize(x + 1);
				}
			}
		}
		
		for (int ix = x, ixe = x + 1; ix < ixe; ++ix)
		{   
			printf("x3\n");
			m_rows[y][ix] = val;
		}
	}
};

src s;

struct pxy {

	Loki::AssocVector<int, int> m_from_src;
	std::vector<int> m_to_src;

	void setData (int x, int y, int val)
	{
		bool acc_c = filterAcceptsColumn(x);
		if (acc_c)
		{
			printf("\t### yay\n");
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

	void insertColumn (int c)
	{
	}

	void insertRow (int r)
	{
	}
};

pxy p;

int main ()
{

	for (;;)
	{
		static size_t i	 = 0;
		s.appendTableXY((2 * i) % 16, i * 2, i + 100);
		p.setData(i, i, i + 100);
		//p.setData((2 * i) % 16, i * 2, i + 100);
		++i;


		if (i == 4)
		{
			s.appendTableXY(1, 1, 666);
			p.setData(1, 1, 666);
		}
		
		if (i == 6)
			break;
	}


	s.dump();

}
