#include <boost/circular_buffer.hpp>

using namespace std;

int main()
{
	int i;
	boost::circular_buffer<int> cb(10);
	boost::circular_buffer<int>::reverse_iterator rit;

	for (i = 0; i < 100; i++)
	{
		cb.push_back(i);
	}

	rit = cb.rbegin();

	while (rit != cb.rend())
	{
		cout << *rit << endl;
		rit++;
	}

	return 0;
}
