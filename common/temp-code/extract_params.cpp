#define ATTR_SET ".<xmlattr>"
#define XML_PATH1 "./lighting-dbus-interface.xml"

#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace boost;
using namespace boost::property_tree;

const ptree& empty_ptree()
{
	static ptree t;
	return t;
}

int main() 
{
	ptree tree;
	read_xml(XML_PATH1, tree);
	const ptree& methods = tree.get_child("node.interface", empty_ptree());
	string iface = methods.get<string>("<xmlattr>.name");
	cout << iface << endl;
	BOOST_FOREACH(const ptree::value_type &method, methods)
	{
		if ((method.first == "method"))
		{
			cout << "Extracting attributes from " << method.first << ":" << endl;
			const string& method_name = method.second.get<string>("<xmlattr>.name");
			cout << "method: " << method_name << endl;
			const ptree& args = method.second;
			BOOST_FOREACH(const ptree::value_type &arg, args)
			{
				if (arg.first == "arg")
				{
					const string arg_type = arg.second.get<string>("<xmlattr>.type");
					const string arg_dir = arg.second.get<string>("<xmlattr>.direction");
					cout << " arg_type: " << arg_type << " arg_dir: " << arg_dir << endl;
				}
			}
		}
	}

	return 0;
}
