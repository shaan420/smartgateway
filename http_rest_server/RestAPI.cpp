#include <string.h>

#include <boost/foreach.hpp>

#include "RestAPI.hpp"
#include "strutil.hpp"

using namespace REST;

struct validate_data
{
	string api;
	set <string>* params; 
};

RestAPI::RestAPI()
{
	set<string> params;
	string sysinfoparams[] = {"cpus", "memory", "os"}; 
	string processinfoparams[] = {"percentmemory", "percentcpu" };
	string diskinfoparamas[] = {"totalparts", "spaceinfo" };

	_apiparams["/sysinfo"] =  set<string>(sysinfoparams, sysinfoparams + 3);
	_apiparams["/procinfo"] = set<string>(processinfoparams, processinfoparams  + 2);
	_apiparams["/diskinfo"] = set<string>(diskinfoparamas, diskinfoparamas + 2);
}

bool RestAPI::executeRestAPI(const string& url, 
							 string& url_string, 
							 const map<string, string>& argvals, 
							 const char *upload_data,
							 int upload_data_size,
							 string& response)
{
	// Ignore all the args except the "fields" param 
	validate_data vdata ;
	vdata.api = url;
	Executor::outputType type = Executor::TYPE_JSON;
	vector<string> params;
	set<string> uniqueparams;
	map<string,string>::const_iterator it1 = argvals.find("fields");

	if (it1 != argvals.end()) 
	{
		string prms = it1->second;
		StrUtil::eraseWhiteSpace(prms);
		StrUtil::splitString(prms, ",", params);   
	}

	BOOST_FOREACH( string pr, params ) 
	{
		uniqueparams.insert(pr);
	}

	vdata.params = &uniqueparams;

	if ( !_validate(&vdata)) {
//		_getInvalidResponse(response);
//		return false;
	}

	it1 = argvals.find("type");
	if (it1 != argvals.end()){
		const string outputtype = it1->second;
		if (strcasecmp(outputtype.c_str(), "xml") == 0 ) {
			type = Executor::TYPE_XML;
		}
	}

	if ((upload_data != NULL) && (upload_data_size > 0))
	{
		/* 
		 * The request is contained within the HTTP POST payload
		 * So send that instead of the url_string.
		 */
		string upload_data_string(upload_data, upload_data_size);
		return _executeAPI(url, upload_data_string, uniqueparams, argvals, type, response);
	}

	return _executeAPI(url, url_string, uniqueparams, argvals, type, response);
}

bool RestAPI::_executeAPI(const string& url, 
						  string& url_string,
		           		  const set<string>& argvals, 
						  const map<string, string>& keyvals, 
						  Executor::outputType type, 
						  string& response)
{
	bool ret = false;
	if (url == "/sysinfo") 
	{
		ret = _executor.sysinfo(argvals, type,  response);
	}
	else if (url == "/diskinfo")
	{
		ret = _executor.diskinfo(argvals, type, response);
	}
	else if (url == "/procinfo")
	{
		ret = _executor.procinfo(argvals, type, response);
	}
	else
	{
		/* 
		 * Device command. 
		 */
		ret = _executor.SendToContextManager(url, url_string, keyvals, type, response);
	}

	return ret;
}

bool RestAPI::_validate(const void *data)
{
	const validate_data *vdata = static_cast<const validate_data *>(data );
	map<string, set<string> > ::iterator it =  _apiparams.find(vdata->api);

	it = _apiparams.find(vdata->api);

	if ( it == _apiparams.end()){
		return false;
	}
	set<string>::iterator it2 = vdata->params->begin();
	while (it2 != vdata->params->end()) {
		if (it->second.find(*it2) == it->second.end()) 
			return false;
		++it2;
	}

	return true;
}

void RestAPI::_getInvalidResponse(string& response)
{
	response = "Some error in your data ";
}
