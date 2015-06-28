#include "OntologyManager.hpp"
#include "boost/foreach.hpp"
#include "boost/filesystem.hpp"

namespace bfs = boost::filesystem;

#define GET_FRAGMENT(IRI_STR) (IRI_STR.substr((IRI_STR.find("#")+1)))

/* Internal helper macros */
#define OWL_CONCEPT(X) (m_kernel.getExpressionManager()->Concept(X))
#define OWL_PROPERTY(X) (m_kernel.getExpressionManager()->Property(X))
#define OWL_INSTANCE(X) (m_kernel.getExpressionManager()->Individual(X))

#define EXP_MANAGER m_kernel.getExpressionManager()

OntologyManager *OntologyManager::GetInstance()
{
	static OntologyManager *s_ont_manager = NULL;

	if (NULL == s_ont_manager)
	{
		s_ont_manager = new OntologyManager;

		if (NULL == s_ont_manager)
		{
			cout << "Error allocating memory for OntologyManager" << endl;
			return NULL;
		}
	}

	return s_ont_manager;
}

int OntologyManager::Init(const char *owl_file_path)
{
	bfs::path owl_file(owl_file_path);

	/* Load the triples in the owl file into the triple_store */
	owlcpp::load_file(owl_file, m_store);

	/* Submit the triples to the reasoning kernel */
	int n = submit(m_store, m_kernel, 0, 0);

	m_ns = m_store.map_ns().at(*(m_store.map_ns().begin())).str() + "#";

	cout << "Ontology initialized with NS: " << m_ns << endl;

	return n;
}

int OntologyManager::GetInstances(vector<string>& results, 
								  string& name,
		                          OwlNodeType_t type,
								  bool direct)
{
	results.clear();
	string node = m_ns + name;
	cout << node << endl << endl;

	switch (type)
	{
		case OWL_NODE_TYPE_CONCEPT:
			BOOST_FOREACH(std::string const& s, 
						  owlcpp::get_instances(OWL_CONCEPT(node), m_kernel, direct))
			{
				cout << s << endl;
				results.push_back(GET_FRAGMENT(s));
				cout << s << endl;
			}
			break;

		case OWL_NODE_TYPE_PROPERTY://doesn't make sense
		/*	BOOST_FOREACH(std::string const& s, 
						  get_instances(OWL_CONCEPT(node), m_kernel, direct))
			{
				results.push_back(s);
			}*/
			break;

		case OWL_NODE_TYPE_INSTANCE://doesn't make sense
		/*	BOOST_FOREACH(std::string const& s, 
						  get_instances(OWL_INSTANCE(node), m_kernel, direct))
			{
				results.push_back(s);
			}*/
			break;
			
		default:
			break;		
	}
		
	return results.size();
}

int OntologyManager::GetSubClasses(vector<string>& results, 
								  string& name,
		                          OwlNodeType_t type,
								  bool direct)
{
	results.clear();
	string node = m_ns + name;

	switch (type)
	{
		case OWL_NODE_TYPE_CONCEPT:
			BOOST_FOREACH(std::string const& s, 
						  owlcpp::get_sub(OWL_CONCEPT(node), m_kernel, direct))
			{
				results.push_back(GET_FRAGMENT(s));
			}
			break;

		case OWL_NODE_TYPE_PROPERTY:
		/*	BOOST_FOREACH(std::string const& s, 
						  get_sub(OWL_CONCEPT(node), m_kernel, direct))
			{
				results.push_back(s);
			}*/
			break;

		case OWL_NODE_TYPE_INSTANCE://doesn't make sense
		/*	BOOST_FOREACH(std::string const& s, 
						  get_sub(OWL_INSTANCE(node), m_kernel, direct))
			{
				results.push_back(s);
			}*/
			break;
			
		default:
			break;		
	}
		
	return results.size();
}

int OntologyManager::GetSuperClasses(vector<string>& results, 
								  string& name,
		                          OwlNodeType_t type,
								  bool direct)
{
	results.clear();
	string node = m_ns + name;

	switch (type)
	{
		case OWL_NODE_TYPE_CONCEPT:
			BOOST_FOREACH(std::string const& s, 
						  owlcpp::get_super(OWL_CONCEPT(node), m_kernel, direct))
			{
				results.push_back(GET_FRAGMENT(s));
			}
			break;

		case OWL_NODE_TYPE_PROPERTY:
		/*	BOOST_FOREACH(std::string const& s, 
			   			  get_super(OWL_CONCEPT(node), m_kernel, direct))
			{
				results.push_back(s);
			}*/
			break;

		case OWL_NODE_TYPE_INSTANCE:
			BOOST_FOREACH(std::string const& s, 
						  owlcpp::get_super(OWL_INSTANCE(node), m_kernel, direct))
			{
				results.push_back(GET_FRAGMENT(s));
			}
			break;
			
		default:
			break;		
	}
		
	return results.size();
}

bool OntologyManager::GetDeviceListByFilter(const map<string, string>& keyvals, vector<string>& devList)
{
	bool valid = true; /* To show whether the node is valid (ie. is a concept or an individual ) in the Ontology file */ 
	TDLConceptExpression *filterConcept = NULL;
	map<string, string>::const_iterator cit;
	map<string, string>::iterator it;
	string devclass;

	cit = keyvals.find("DeviceClass");
	if (it == keyvals.end())
	{
		cout << "Error: Could not find device name in keyvals map\n";
		return false;
	}

	devclass.assign(cit->second);

	if (IsInstance(devclass, valid))
	{
		/*
		 * Not a concept. return 
		 */
		cout << "Error: Not a concept\n";
		return false;
	}

	/*
	 * Create a new arg list to create the targetConcept by combining 
	 * the filterConcepts.
	 */
	EXP_MANAGER->newArgList();

	TDLConceptExpression *uriConcept = EXP_MANAGER->Concept(m_ns + devclass);
	EXP_MANAGER->addArg(uriConcept);

	for (cit = keyvals.begin(); cit != keyvals.end(); cit++)
	{
		/* skip the "DeviceClass" key itself */
		if (cit->first == "DeviceClass")
		{
			continue;
		}

		/* "role" is an object role that occurs in the URL as key-value args.
		 * For eg: hasLocation() is the property that we will use to filter the devices */
		TDLObjectRoleExpression *filterRole = EXP_MANAGER->ObjectRole(m_ns + cit->first);

		/* 
		 * "filterExpression" occurs as a URL arg (key-value pair).
		 * It represents a the concept that the filter-role should be applied to.
		 * There can be multiple filterConcepts in which case we would need 
		 * to do a conjunction of all the filterConcepts with their corresponding role-filters
		 * For example: we might want to identify a lighting device by its location (eg: bedroom) and 
		 * color (eg: blue light). Therefore, we would need to generate:
		 * 1. Concept of "Exists hasLightColor.blue" <-- Note that this maybe a DataValueRestriction instead of ObjectRestriction.
		 * 2. Concept of "Exists hasLocation.Bedroom"
		 * 3. And() above concepts and the resultant concept would be our filterConcept.
		 */
		if (IsInstance(cit->second.c_str(), valid))
		{
			filterConcept = EXP_MANAGER->Value(filterRole, EXP_MANAGER->Individual(m_ns + cit->second));
		}
		else
		{
			if (valid)
			{
				/* Is a Concept node */
				filterConcept = EXP_MANAGER->Exists(filterRole, EXP_MANAGER->Concept(m_ns + cit->second));
			}
			else
			{
				/* Not a valid node in the ontology */ 
				return false;
			}
		}

		/* 
		 * Insert the partial filter concepts into the arg list
		 */
		EXP_MANAGER->addArg(filterConcept);		
	}

	/* 
	 * After the filterConcept is ready, simply "AND" everything in the arg list to get the final target concept.
	 * Instances of the target concept would be the physical devices on which we would want to perform the requested 
	 * commands.
	 * In case we get multiple such instances/physical devices, we would simply send back the list to the user.
	 * The user would then select one of them to perform the command on.
	 */

	TDLConceptExpression *targetConcept = EXP_MANAGER->And();

	BOOST_FOREACH(std::string const& dev, owlcpp::get_instances(targetConcept, m_kernel, false))
	{
		std::cout << "Device Found: " << GET_FRAGMENT(dev) << std::endl;
		devList.push_back(GET_FRAGMENT(dev));
	}

	return true;
}

bool OntologyManager::IsInstance(const string& name, bool& valid)
{
	const owlcpp::Node_id *node = m_store.find_node_iri(m_ns + name);

	if (NULL == node)
	{
		cout << name << " is an invalid node" << endl;
		valid = false;
		return false;
	}

	owlcpp::Triple_store::query_b<1,1,0,0>::range r = m_store.find_triple(
			*node,
			owlcpp::terms::rdf_type::id(),
			owlcpp::any(),
			owlcpp::any()
			);

	BOOST_FOREACH(owlcpp::Triple const& t, r)
	{
		if (t.obj_ == owlcpp::terms::owl_NamedIndividual::id())
		{
			cout << name << " is an instance node" << endl;
			return true;
		}
		else
		{	
			cout << name << " is an concept node" << endl;
			return false;
		}
	}
	
	cout << name << " is an unknown node" << endl;
	valid = false;
	return false;
}

int OntologyManager::GetData(const string& subject, const string& pred, string& object)
{
	const owlcpp::Node_id *subj_id = m_store.find_node_iri(m_ns + subject);
	const owlcpp::Node_id *pred_id = m_store.find_node_iri(m_ns + pred);

	if ((NULL == subj_id) || (NULL == pred_id))
	{
		cout << subject  << " " << pred << " is an invalid node" << endl;
		return -1;
	}

	cout << "Querying Ontology for " << subject << " and " << pred << endl;

	BOOST_FOREACH(owlcpp::Triple const& t, 
				m_store.find_triple(*subj_id, *pred_id, owlcpp::any(), owlcpp::any())) 
	{
		object.assign(to_string(t.obj_, m_store));
		cout << object << endl;
	}

	return 0;
}
