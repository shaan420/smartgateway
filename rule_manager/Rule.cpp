#include "Rule.hpp"
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

int Rule::StringToRule(const char *str)
{
	string ruleStr(str);
	string headStr;
	vector<string> condStrVec;

	ParseRule(ruleStr, headStr, condStrVec);

	if (headStr.empty() || condStrVec.empty())
	{
		cout << "ERROR: could not parse rule string " << str << endl;
		return -1;
	}

	/* set the head of the rule */
	m_head = new Condition(headStr);
	if (string::npos != m_head->CondStr().find("do"))
	{
		cout << "DO cond\n";
		m_head->type(CONDITION_TYPE_DO);
	}
	else if (string::npos != m_head->CondStr().find("event"))
	{
		cout << "EVENT cond\n";
		m_head->type(CONDITION_TYPE_EVENT);
	}
	else if (string::npos != m_head->CondStr().find("ont"))
	{
		cout << "ONT cond\n";
		m_head->type(CONDITION_TYPE_ONT);
	}
	else
	{
		cout << "ERROR: Head contains unsupported predicate\n";
		delete m_head;
		return -1;
	}

	BOOST_FOREACH(string& s, condStrVec)
	{
		cout << "Found cond: " << s << endl;
		Condition *c = new Condition(s);
		if (string::npos != c->CondStr().find("get"))
		{
			cout << "GET cond\n";
			c->type(CONDITION_TYPE_GET);
		}
		else if (string::npos != c->CondStr().find("do"))
		{
			cout << "DO cond\n";
			c->type(CONDITION_TYPE_DO);
		}
		else if (string::npos != c->CondStr().find("event"))
		{
			cout << "EVENT cond\n";
			c->type(CONDITION_TYPE_EVENT);
		}
		else if (string::npos != c->CondStr().find("ont"))
		{
			cout << "ONT cond\n";
			c->type(CONDITION_TYPE_ONT);
		}
		else
		{
			cout << "GENERAL cond\n";
			c->type(CONDITION_TYPE_GENERAL);
		}

		m_condVec.push_back(c);
	}

	return 0;
}

string Rule::String()
{
	if (m_condVec.empty())
	{
		return NULL;
	}

	string str;

	str += m_head->CondStr() + " :- ";
	BOOST_FOREACH(Condition *c, m_condVec)
	{
		str += c->CondStr() + ", ";
	}

	/* replace the last occurence of a "," with a "." */
	str.replace(str.find_last_of(","), 1, ".");

	return str;
}

bool Rule::ParseRule(std::string& ruleStr, std::string& headStr, std::vector<std::string>& vec)
{
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	namespace phoenix = boost::phoenix;

	using boost::spirit::qi::_1;
	using boost::spirit::qi::phrase_parse;
	using boost::spirit::ascii::space;
	using boost::phoenix::ref;
	using qi::int_;
	using qi::lit;
	using qi::lexeme;
	using ascii::char_;
	using phoenix::push_back;

	qi::rule<std::string::iterator, std::string(), ascii::space_type> groundTerm, variable, value, ts, body;
	qi::rule<std::string::iterator, std::string(), ascii::space_type> headCondition, tailCondition, doCondition;
	qi::rule<std::string::iterator, std::string(), ascii::space_type> getCondition, eventConditionVar, eventConditionVal;
	qi::rule<std::string::iterator, std::string(), ascii::space_type> generalCondition, ontCondition;

	headCondition = doCondition | eventConditionVal | ontCondition;
	doCondition = char_("d") >> char_("o") >>
		char_("(") >> groundTerm >> char_(",") >> groundTerm >> char_(",") >> value >> char_(",") >> ts >> char_(")");

	eventConditionVar = char_("e") >> char_("v") >> char_("e") >> char_("n") >> char_("t") >>
		char_("(") >> groundTerm >> char_(",") >> groundTerm >> char_(",") >> variable >> char_(",") >> ts >> char_(")");

	eventConditionVal = char_("e") >> char_("v") >> char_("e") >> char_("n") >> char_("t") >>
		char_("(") >> groundTerm >> char_(",") >> groundTerm >> char_(",") >> value >> char_(",") >> ts >> char_(")");

	ontCondition = char_("o") >> char_("n") >> char_("t") >>
		char_("(") >> groundTerm >> char_(",") >> groundTerm >> char_(",") >> value >> char_(",") >> ts >> char_(")");

	groundTerm = qi::char_("a-z") >> *qi::char_("a-zA-Z_0-9");
	value = +qi::char_("0-9") | +(qi::char_ - '&' - ';' - '(' - ')' - ',');
	ts = +qi::char_("0-9") | variable;

	tailCondition = getCondition | eventConditionVal | generalCondition;
	getCondition = char_("g") >> char_("e") >> char_("t") >>
		char_("(") >> groundTerm >> char_(",") >> groundTerm >> char_(",") >> variable >> char_(",") >> ts >> char_(")");
	generalCondition = +(qi::char_ - '(' - ')' - ',' - '.');
	variable = qi::char_("A-Z") >> *qi::char_("a-zA-Z_0-9");

	bool ret = phrase_parse(ruleStr.begin(), ruleStr.end(),
			//  Begin grammar
			(
			 headCondition[ref(headStr) = _1] >> qi::lit(":-") >>
			 (tailCondition[push_back(ref(vec), _1)] >> *(',' >> tailCondition[push_back(ref(vec), _1)])) >> '.'
			),
			//  End grammar

			space);

	return ret;
}

