#include "url_utils.hpp"

int url_key_value_to_map(const char *query, client::pairs_type& m)
{
	namespace qi = boost::spirit::qi;

	std::string resp;
	std::string input(query);
	std::string::iterator begin = input.begin();
	std::string::iterator end = input.end();

	client::key_value_sequence<std::string::iterator> p;

	if (!qi::parse(begin, end, p, m))
	{
		std::cout << "-------------------------------- \n";
		std::cout << "Parsing failed\n";
		std::cout << "-------------------------------- \n";
		return -1;
	}
	else
	{
		std::cout << "-------------------------------- \n";
		std::cout << "Parsing succeeded, found entries:\n";
		client::pairs_type::iterator end = m.end();
		for (client::pairs_type::iterator it = m.begin(); it != end; ++it)
		{
			std::cout << (*it).first;
			if (!(*it).second.empty())
				std::cout << "=" << (*it).second;
			std::cout << std::endl;
		}
		std::cout << "---------------------------------\n";
	}
		
	return 0;
}
