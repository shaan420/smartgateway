#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <stdlib.h>
#include <map>
#include <boost/foreach.hpp>
#include "boost/filesystem.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp> 

#define SMARTGATEWAY_HOME_ONTOLOGY_FILEPATH "home.owl"

/* Well-known name for this service. */
#define SMARTGATEWAY_SERVICE_PATH        "org.asu.smarthome.smartgateway"

/* Object path to the provided object. */
#define SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX "/org/asu/smarthome/smartgateway/"

/* 
 * And we're interested in using it through this interface.
 * This must match the entry in the interface definition XML. 
 */
#define SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX   "org.asu.smarthome.smartgateway."

#define SMARTOBJECT_TEMPLATE_DIR "/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/"

using namespace std;
using namespace boost;
using boost::property_tree::ptree;
namespace fs = boost::filesystem;

string nocapital_str;
string allcapital_str;
string firstcapital_str;

const ptree& empty_ptree()
{
	static ptree t;
	return t;
}

class Arg
{
	private:
		string m_name;
		string m_type;
		string m_dir;

	public:
		Arg()
		{
		}

		Arg(const string& name, const string& type, const string& dir)
			:m_name(name), m_dir(dir), m_type(type)
		{
		}

		string& GetName()
		{
			return m_name;
		}

		const char *GetType()
		{
			if (m_type == "i")
				return "gint";
			else if (m_type == "s")
				return "gchar *";
		}

		string& GetDir()
		{
			return m_dir;
		}
};

class Method
{
	private:
		string m_name;
		map<string, Arg*> m_argMap;
	
	public:
		Method(const string& name)
			: m_name(name)
		{
		}

		~Method()
		{
		}

		int InsertArg(const string& name, const string& type, const string& dir)
		{
			Arg *arg = new Arg(name, type, dir);
			m_argMap.insert(make_pair<string, Arg*>(name, arg));
			return 0;
		}

		string& GetName()
		{
			return m_name;
		}

		map<string, Arg*>& GetArgMap()
		{
			return m_argMap;
		}
};

class Signal
{
	private:
		string m_name;
		map<string, Arg*> m_argMap;
	
	public:
		Signal(const string name)
			: m_name(name)
		{
		}

		~Signal()
		{
		}

		int InsertArg(const string name, const string type)
		{
			Arg *arg = new Arg(name, type, "out");
			m_argMap.insert(make_pair<string, Arg*>(name, arg));
			return 0;
		}
};

int readTemplateFile(const char *filename, string& buf)
{
	fs::ifstream istream;
	istream.open(filename, ios::in);

	stringstream ss;
	ss << istream.rdbuf();

	buf.assign(ss.str());
	return 0;
}

int processMethodBody(Method *m, string& buf)
{
	cout << "------Begin processMethodBody------"<< endl;
	map<string, Arg*>::iterator it;
	for (it = m->GetArgMap().begin(); it != m->GetArgMap().end(); it++)
	{
		Arg *arg = it->second;
		string body_str;
		if (arg->GetDir() == "in")
		{
			readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject_method_body_dir_in.template", body_str);
		}
		else
		{
			readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject_method_body_dir_out.template", body_str);
		}

		std::string::const_iterator start, end;
		boost::regex exp("\\?<(\\w+)>");
		boost::match_results<std::string::const_iterator> what;
		boost::match_flag_type flags = boost::match_default;
		start = body_str.begin();
		end = body_str.end();

		while (regex_search(start, end, what, exp, flags))
		{
			string res = string(what[0].first, what[0].second);
			string command = string(what[1].first, what[1].second);
			cout << "res = ";
			cout << res << endl;
			cout << "command = ";
			cout << command << endl;

			/*
			 * Now we have to replace the res with what the command specifies
			 * For eg: 
			 * if res = "?<nocapital>" and command = "nocapital", then
			 * 		replace "?<nocapital>" with "lighting".
			 * if res = "?<allcapital>" and command = "allcapital", then
			 * 		replace "?<allcapital>" with "LIGHTING".
			 */
			if (command == "argname")
			{
				body_str.replace((what[0].first - body_str.begin()), res.size(), arg->GetName());
				start = body_str.begin();
			}
			else
			{
				start = what[0].second;
			}

			end = body_str.end();
		}

		buf.append(body_str);
	}
	cout << "------End processMethodBody------"<< endl;

	return 0;
}

int processMethod(Method *m, string& buf)
{
	cout << "------Begin processMethod------"<< endl;
	readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject_method.template",
	                                 buf);
	/* buf has the file data */
	std::string::const_iterator start, end;
	boost::regex exp("\\?<(\\w+)>");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	start = buf.begin();
	end = buf.end();

	while (regex_search(start, end, what, exp, flags))
	{
		string res = string(what[0].first, what[0].second);
		string command = string(what[1].first, what[1].second);
		cout << "res = ";
		cout << res << endl;
		cout << "command = ";
		cout << command << endl;
		//start = what[0].second;

		/*
		 * Now we have to replace the res with what the command specifies
		 * For eg: 
		 * if res = "?<nocapital>" and command = "nocapital", then
		 * 		replace "?<nocapital>" with "lighting".
		 * if res = "?<allcapital>" and command = "allcapital", then
		 * 		replace "?<allcapital>" with "LIGHTING".
		 */
		if (command == "methodname")
		{
			//buf.replace((what[0].first - start), res.size(), m->GetName());
			buf.replace((what[0].first - buf.begin()), res.size(), m->GetName());
			//start = what[0].first + m->GetName().size();
			start = buf.begin();
			//start = what[0].first;
		}
		else if (command == "arglist")
		{
			string arglist_str;
			map<string, Arg*>::iterator it;
			for (it = m->GetArgMap().begin(); it != m->GetArgMap().end(); it++)
			{
				Arg *arg = it->second;
				if (arg->GetDir() == "in")
				{
					arglist_str = arglist_str + arg->GetType() + " " + arg->GetName() + ",";
				}
				else
				{
					arglist_str = arglist_str + arg->GetType() + " *" + arg->GetName() + ",";
				}
			}

			cout << what[0].first - buf.begin() << endl;
			buf.replace((what[0].first - buf.begin()), res.size(), arglist_str);
			cout << buf << endl;
			start = buf.begin();
		}
		else if (command == "methodbody")
		{
			string body_str;
			processMethodBody(m, body_str);
			buf.replace((what[0].first - buf.begin()), res.size(), body_str);
			start = buf.begin();
		}
		else
		{
			start = what[0].second;
		}

		end = buf.end();
	}

	cout << buf << endl;

	cout << "------End processMethod------"<< endl;
	return 0;
}

int processMethodDecl(Method *m, string& buf)
{
	cout << "------Begin processMethodDecl------"<< endl;
	readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject_method_decl.template",
	                                 buf);
	/* buf has the file data */
	std::string::const_iterator start, end;
	boost::regex exp("\\?<(\\w+)>");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	start = buf.begin();
	end = buf.end();

	while (regex_search(start, end, what, exp, flags))
	{
		string res = string(what[0].first, what[0].second);
		string command = string(what[1].first, what[1].second);
		cout << "res = ";
		cout << res << endl;
		cout << "command = ";
		cout << command << endl;
		//start = what[0].second;

		/*
		 * Now we have to replace the res with what the command specifies
		 * For eg: 
		 * if res = "?<nocapital>" and command = "nocapital", then
		 * 		replace "?<nocapital>" with "lighting".
		 * if res = "?<allcapital>" and command = "allcapital", then
		 * 		replace "?<allcapital>" with "LIGHTING".
		 */
		if (command == "methodname")
		{
			//buf.replace((what[0].first - start), res.size(), m->GetName());
			buf.replace((what[0].first - buf.begin()), res.size(), m->GetName());
			//start = what[0].first + m->GetName().size();
			start = buf.begin();
			//start = what[0].first;
		}
		else if (command == "arglist")
		{
			string arglist_str;
			map<string, Arg*>::iterator it;
			for (it = m->GetArgMap().begin(); it != m->GetArgMap().end(); it++)
			{
				Arg *arg = it->second;
				if (arg->GetDir() == "in")
				{
					arglist_str = arglist_str + arg->GetType() + " " + arg->GetName() + ",";
				}
				else
				{
					arglist_str = arglist_str + arg->GetType() + " *" + arg->GetName() + ",";
				}
			}

			cout << what[0].first - buf.begin() << endl;
			buf.replace((what[0].first - buf.begin()), res.size(), arglist_str);
			cout << buf << endl;
			start = buf.begin();
		}
		else
		{
			start = what[0].second;
		}

		end = buf.end();
	}

	cout << buf << endl;

	cout << "------End processMethodDecl------"<< endl;
	return 0;
}

int processObjectNames(string& buf)
{
	cout << "------Begin processObjectName------"<< endl;
	/* buf has the file data */
	std::string::const_iterator start, end;
	boost::regex exp("\\?<(\\w+)>");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	start = buf.begin();
	end = buf.end();

	while (regex_search(start, end, what, exp, flags))
	{
		string res = string(what[0].first, what[0].second);
		string command = string(what[1].first, what[1].second);
		cout << "res = ";
		cout << res << endl;
		cout << "command = ";
		cout << command << endl;

		/*
		 * Now we have to replace the res with what the command specifies
		 * For eg: 
		 * if res = "?<nocapital>" and command = "nocapital", then
		 * 		replace "?<nocapital>" with "lighting".
		 * if res = "?<allcapital>" and command = "allcapital", then
		 * 		replace "?<allcapital>" with "LIGHTING".
		 */
		if (command == "nocapital")
		{
			buf.replace((what[0].first - buf.begin()), res.size(), nocapital_str);
			start = buf.begin();
		}
		else if (command == "allcapital")
		{
			buf.replace((what[0].first - buf.begin()), res.size(), allcapital_str);
			start = buf.begin();
		}
		else if (command == "firstcapital")
		{
			buf.replace((what[0].first - buf.begin()), res.size(), firstcapital_str);
			start = buf.begin();
		}
		else
		{
			cout << "command not supported: " << command << endl;
			return 0;
		}

		end = buf.end();
	}

	cout << "------End processObjectName------"<< endl;
	return 0;
}

int processMainTemplate(vector<string>& iface_vec, string& buf)
{
	cout << "------Begin processMainTemplate------"<< endl;
	/* buf has the template file data */
	std::string::const_iterator start, end;
	boost::regex exp("\\?<(\\w+)>");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	start = buf.begin();
	end = buf.end();

	while (regex_search(start, end, what, exp, flags))
	{
		string res = string(what[0].first, what[0].second);
		string command = string(what[1].first, what[1].second);
		cout << "res = ";
		cout << res << endl;
		cout << "command = ";
		cout << command << endl;

		/*
		 * Now we have to replace the res with what the command specifies
		 * For eg: 
		 * if res = "?<nocapital>" and command = "nocapital", then
		 * 		replace "?<nocapital>" with "lighting".
		 * if res = "?<allcapital>" and command = "allcapital", then
		 * 		replace "?<allcapital>" with "LIGHTING".
		 */
		if (command == "registerdevices")
		{
			string reg_buf;
			BOOST_FOREACH(string name, iface_vec)
			{
				reg_buf += "DEVICE_FACTORY_REGISTER_DEVICE(" + name + "Device);\n";			
			}

			buf.replace((what[0].first - buf.begin()), res.size(), reg_buf);
			start = buf.begin();
		}
		else if (command == "deviceincludes")
		{
			string inc_buf;
			BOOST_FOREACH(string name, iface_vec)
			{
				inc_buf += "#include \"" + name + "Device.hpp\"\n";
			}

			buf.replace((what[0].first - buf.begin()), res.size(), inc_buf);
			start = buf.begin();
		}
		else
		{
			start = what[0].second;
		}

		end = buf.end();
	}

	cout << buf << endl;

	cout << "------End processMainTemplate------"<< endl;
	return 0;
}
int processCTemplate(map<string, Method*>& methodMap, string& buf)
{
	cout << "------Begin processCTemplate------"<< endl;
	/* buf has the template file data */
	std::string::const_iterator start, end;
	boost::regex exp("\\?<(\\w+)>");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	start = buf.begin();
	end = buf.end();

	while (regex_search(start, end, what, exp, flags))
	{
		string res = string(what[0].first, what[0].second);
		string command = string(what[1].first, what[1].second);
		cout << "res = ";
		cout << res << endl;
		cout << "command = ";
		cout << command << endl;
		//start = what[0].second;

		/*
		 * Now we have to replace the res with what the command specifies
		 * For eg: 
		 * if res = "?<nocapital>" and command = "nocapital", then
		 * 		replace "?<nocapital>" with "lighting".
		 * if res = "?<allcapital>" and command = "allcapital", then
		 * 		replace "?<allcapital>" with "LIGHTING".
		 */
		if (command == "methoddefs")
		{
			string methods_buf;
			map<string, Method*>::iterator m_it;
			for (m_it = methodMap.begin(); m_it != methodMap.end(); m_it++)
			{
				string m_buf;
				Method *m = m_it->second;
				processMethod(m, m_buf);
				methods_buf.append(m_buf);
			}

			buf.replace((what[0].first - buf.begin()), res.size(), methods_buf);
			start = buf.begin();
		}
		else
		{
			start = what[0].second;
		}

		end = buf.end();
	}

	cout << buf << endl;

	cout << "------End processCTemplate------"<< endl;
	return 0;
}

int processHTemplate(map<string, Method*>& methodMap, string& buf)
{
	cout << "------Begin processHTemplate------"<< endl;
	/* buf has the template file data */
	std::string::const_iterator start, end;
	boost::regex exp("\\?<(\\w+)>");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	start = buf.begin();
	end = buf.end();

	while (regex_search(start, end, what, exp, flags))
	{
		string res = string(what[0].first, what[0].second);
		string command = string(what[1].first, what[1].second);
		cout << "res = ";
		cout << res << endl;
		cout << "command = ";
		cout << command << endl;
		//start = what[0].second;

		/*
		 * Now we have to replace the res with what the command specifies
		 * For eg: 
		 * if res = "?<nocapital>" and command = "nocapital", then
		 * 		replace "?<nocapital>" with "lighting".
		 * if res = "?<allcapital>" and command = "allcapital", then
		 * 		replace "?<allcapital>" with "LIGHTING".
		 */
		if (command == "methoddecls")
		{
			string methods_buf;
			map<string, Method*>::iterator m_it;
			for (m_it = methodMap.begin(); m_it != methodMap.end(); m_it++)
			{
				string m_buf;
				Method *m = m_it->second;
				processMethodDecl(m, m_buf);
				methods_buf.append(m_buf);
			}

			buf.replace((what[0].first - buf.begin()), res.size(), methods_buf);
			start = buf.begin();
		}
		else if (command == "structargs")
		{
			string structargs_buf;
			map<string, Arg*> argMap;
			map<string, Method*>::iterator m_it;
			map<string, Arg*>::iterator a_it;
			for (m_it = methodMap.begin(); m_it != methodMap.end(); m_it++)
			{
				string m_buf;
				Method *m = m_it->second;
				for (a_it = m->GetArgMap().begin(); a_it != m->GetArgMap().end(); a_it++)
				{
					Arg *arg = a_it->second;
					argMap.insert(make_pair<string, Arg*>(arg->GetName(), arg));
				}
			}

			for (a_it = argMap.begin(); a_it != argMap.end(); a_it++)
			{
				Arg *arg = a_it->second;
				structargs_buf = structargs_buf + arg->GetType() + " m_" + arg->GetName() + ";\n";
			}
			
			buf.replace((what[0].first - buf.begin()), res.size(), structargs_buf);
			start = buf.begin();
		}
		else
		{
			start = what[0].second;
		}

		end = buf.end();
	}

	cout << buf << endl;

	cout << "------End processCTemplate------"<< endl;
	return 0;
}

int WriteToFile(string& filename, string& contents)
{
	ofstream out;
	out.open(filename.c_str());
	out << contents;
	out.close();
	return 0;
}

int main()
{
	string iface_name;
	string out_filename;
	vector<string> iface_vec;
	ptree tree;

	//fs::path targetDir("/home/ayanami-dummy/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/input"); 
	fs::path targetDir("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/input"); 

	fs::directory_iterator dir_it(targetDir), eod;

	BOOST_FOREACH(fs::path const &p, std::make_pair(dir_it, eod))   
	{ 
		map<string, Method*> methodMap;
		map<string, Signal*> signalMap;

		if(is_regular_file(p))
		{
			/* Load Interface XML file */
			//cout << "Reading XML" << endl;
			cout << "Parsing Device Interface File: " << p.string() << endl;

			read_xml(p.string(), tree);

			const ptree& methods = tree.get_child("node.interface", empty_ptree());
			const string& iface_path = methods.get<string>("<xmlattr>.name");

			iface_name = iface_path.substr(iface_path.find(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX) 
											+ strlen(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX));
			/* Create new device interface */
			//DeviceIface *pIface = new DeviceIface(iface_name);
			cout << iface_name << endl;
			iface_vec.push_back(iface_name);

			/*
			 * Prepare the str for command-replace operation
			 */
			nocapital_str = allcapital_str = firstcapital_str = iface_name;
			boost::algorithm::to_lower(nocapital_str);
			boost::algorithm::to_upper(allcapital_str);

			BOOST_FOREACH(const ptree::value_type &method, methods)
			{
				if ((method.first == "method"))
				{
					/* Get method name from interface file */
					cout << "Extracting attributes from " << method.first << ":" << endl;
					const string& method_name = method.second.get<string>("<xmlattr>.name");
					cout << "method: " << method_name << endl;

					Method *m = new Method(method_name);

					const ptree& args = method.second;
					BOOST_FOREACH(const ptree::value_type &arg, args)
					{
						if (arg.first == "arg")
						{
							const string arg_name = arg.second.get<string>("<xmlattr>.name");
							const string arg_type = arg.second.get<string>("<xmlattr>.type");
							const string arg_dir = arg.second.get<string>("<xmlattr>.direction");
							cout << " arg_name: " << arg_name << " arg_type: " << arg_type << " arg_dir: " << arg_dir << endl;
							m->InsertArg(arg_name, arg_type, arg_dir);
						}
					}

					/* Insert method into methodMap */
					methodMap.insert(make_pair<string, Method*>(method_name, m));
				}
				else if (method.first == "signal")
				{
					/* Get method name from interface file */
					cout << "Extracting attributes from " << method.first << ":" << endl;
					const string& method_name = method.second.get<string>("<xmlattr>.name");
					cout << "method: " << method_name << endl;

					Signal *m = new Signal(method_name);
					const ptree& args = method.second;
					BOOST_FOREACH(const ptree::value_type &arg, args)
					{
						if (arg.first == "arg")
						{
							const string arg_name = arg.second.get<string>("<xmlattr>.name");
							const string arg_type = arg.second.get<string>("<xmlattr>.type");
							const string arg_dir = arg.second.get<string>("<xmlattr>.direction");
							cout << " arg_name: " << arg_name << " arg_type: " << arg_type << " arg_dir: " << arg_dir << endl;
							m->InsertArg(arg_name, arg_type);
						}
					}

					signalMap.insert(make_pair<string, Signal*>(method_name, m));
				}
			}
		} 

		/* MethodMap and SignalMap are ready */

		/*
		 * Open and process the .c template file
		 */
		string ctemplate_buf;
		readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject.c.template",
							ctemplate_buf);

		/*
		 * Now populate the template
		 */
		processCTemplate(methodMap, ctemplate_buf);
		processObjectNames(ctemplate_buf);

		cout << ctemplate_buf << endl;
		out_filename = iface_name + "Object.c";
		WriteToFile(out_filename, ctemplate_buf);
		
		/*
		 * Open and process the .h template file
		 */
		string htemplate_buf;
		readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject.h.template",
							htemplate_buf);

		/*
		 * Now populate the template
		 */
		processHTemplate(methodMap, htemplate_buf);
		processObjectNames(htemplate_buf);

		cout << htemplate_buf << endl;
		out_filename = iface_name + "Object.h";
		WriteToFile(out_filename, htemplate_buf);

	}
	
	/*
	 * Now generate the main.cpp
	 */
	string maintemplate_buf;
	readTemplateFile("/home/shankar/Dropbox/ASU_Courses/SmartHome/home_automation/smartgateway/codegen/template/smartobject_main.template",
			maintemplate_buf);

	processMainTemplate(iface_vec, maintemplate_buf);
	cout << maintemplate_buf << endl;
	out_filename = "main.cpp";
	WriteToFile(out_filename, maintemplate_buf);

	return 0;
}

