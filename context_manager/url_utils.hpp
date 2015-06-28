#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <iostream>
#include <map>

namespace client
{
	namespace qi = boost::spirit::qi;

	typedef std::map<std::string, std::string> pairs_type;

	template <typename Iterator>
		struct key_value_sequence 
		: qi::grammar<Iterator, pairs_type()>
		{
			key_value_sequence()
				: key_value_sequence::base_type(query)
			{
				query =  pair >> *((qi::lit(';') | '&') >> pair);
				pair  =  key >> -('=' >> value);
				key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
				value = +qi::char_("a-zA-Z_0-9");
			}

			qi::rule<Iterator, pairs_type()> query;
			qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
			qi::rule<Iterator, std::string()> key, value;
		};
}

int url_key_value_to_map(const char *query, client::pairs_type& m);
