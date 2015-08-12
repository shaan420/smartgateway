#ifndef __ONTOLOGY_MANAGER_HPP__
#define __ONTOLOGY_MANAGER_HPP__
#include <vector>
#include <string>
#include <map>
#include "boost/range.hpp"
#include "owlcpp/rdf/triple_store.hpp"
#include "owlcpp/io/input.hpp"
#include "owlcpp/io/catalog.hpp"
#include "owlcpp/terms/node_tags_owl.hpp"
#include "owlcpp/rdf/print_node.hpp"

#include "factpp/Kernel.hpp"
#include "owlcpp/logic/triple_to_fact.hpp"
#include "owlcpp/logic/reasoner/query_fact.hpp"
#include "owlcpp/logic/detail/triple_to_fact_adaptor.hpp"

using namespace std;

#define ONT_MANAGER (OntologyManager::GetInstance())

#define GET_FRAGMENT(IRI_STR) (IRI_STR.substr((IRI_STR.find("#")+1)))

typedef enum
{
	OWL_NODE_TYPE_CONCEPT = 0,
	OWL_NODE_TYPE_PROPERTY,
	OWL_NODE_TYPE_INSTANCE,
	OWL_NODE_TYPE_UNKNOWN
} OwlNodeType_t;

class OntologyManager
{
	private:
		/* Namespace prefix */
		string m_ns;

		/* RDF Triple store */
		owlcpp::Triple_store m_store;

		/* 
		 * Triple update map 
		 * The OWLCPP lib currently does not support 
		 * updating/erasing triples. So as a workaround
		 * maintain a map that contains all the updates.
		 * 
		 * GetData() will first check the Ontology with 
		 * the subj and pred. If the obj says "in-memory"
		 * then lookup this map for the value, otherwise return 
		 * obj.
		 * SetData() will insert the following triple
		 * subj-pred-"in-memory" always.
		 * TODO: remove this workaround once the library supports
		 * updating triples.
		 * 
		 * The key is basically the concatenation of the
		 * subj and the pred. The obj is the value.
		 */
		map<string, string> m_tripleUpdateMap;

		/* Reasoning Kernel */
		ReasoningKernel m_kernel;

	public:
		static OntologyManager *GetInstance();

		int Init(const char *);
		
		string& GetNs()
		{
			return m_ns;
		}

		map<string, string>& TripleUpdateMap()
		{
			return m_tripleUpdateMap;
		}

		bool GetDeviceListByFilter(const map<string, string>& keyvals, 
								   vector<string>& devList);

		int GetData(const string& subject, 
			    	const string& pred, 
					string& object);

		int SetData(const string& subject, 
					const string& pred, 
					const string& object);

		bool IsInstance(const string& name, bool &valid);

		int GetSubClasses(vector<string>& results, 
						  string& node, 
						  OwlNodeType_t type, 
						  bool direct);

		int GetSuperClasses(vector<string>& results, 
							string& node, 
							OwlNodeType_t type,
							bool direct);

		int GetInstances(vector<string>& results, 
						 string& node,
						 OwlNodeType_t type,
						 bool direct);
};
#endif /* __ONTOLOGY_MANAGER_HPP__ */
