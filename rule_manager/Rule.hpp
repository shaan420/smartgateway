#ifndef __RULE_HPP__
#define __RULE_HPP__
#include <vector>
#include <string>
#include "Condition.hpp"

using namespace std;

class Rule
{
	private:
		int m_id;
		/*
		 * A rule-head is also a type of condition
		 * Instead of a "get" its a "do"
		 */
		Condition *m_head;
		vector<Condition *> m_condVec;
		int m_isOneShot;
		string m_assertedRuleStr;

		bool ParseRule(string& ruleStr, string& head, vector<string>& condStrVec);

	public:
		Rule()
		{
			m_head = NULL;
			m_isOneShot = 0;
		}

		Rule(const char *ruleStr)
		{
			if (ruleStr)
				StringToRule(ruleStr);
		}

		~Rule()
		{
			while (!m_condVec.empty())
			{
				delete m_condVec.back();
				m_condVec.pop_back();
			}

			if (m_head)
				delete m_head;
		}

		Condition *Head()
		{
			return m_head;
		}

		int id()
		{
			return m_id;
		}

		void id(int i)
		{
			m_id = i;
		}

		int IsOneShot()
		{
			return m_isOneShot;
		}

		void IsOneShot(int y)
		{
			m_isOneShot = y;
		}

		int StringToRule(const char *str);
		string String();

		string& AssertedRuleStr()
		{
			return m_assertedRuleStr;
		}

		vector<Condition *>& CondVec()
		{
			return m_condVec;
		}
};
#endif
