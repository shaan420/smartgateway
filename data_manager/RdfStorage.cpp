#include "RdfStorage.hpp"
#include "TimeManager.hpp"

#define NUM_BUCKETS 60

librdf_world *RdfStorage::sm_world = NULL;
librdf_parser *RdfStorage::sm_parser = NULL;

int RdfStorage::Init()
{
	if (NULL == sm_world)
	{
		sm_world = librdf_new_world();
		if (NULL == sm_world)
		{
			printf("ERR: Could not create librdf_world.\n");
			return -1;
		}

		librdf_world_open(sm_world);
	}

	if (NULL == sm_parser)
	{
		sm_parser=librdf_new_parser(sm_world, "rdfxml", NULL, NULL);
		if(NULL == sm_parser) 
		{
			printf("Failed to create new parser 'rdfxml'\n");
			return -1;
		}
	}

	m_storage = librdf_new_storage(sm_world, "memory", "test", NULL);
	if(NULL == m_storage) 
	{
		printf("Failed to create new storage\n");
		return -1;
	}

	m_model = librdf_new_model(sm_world, m_storage, NULL);
	if(NULL == m_model) 
	{
		printf("Failed to create model\n");
		return -1;
	}

		return 0;
}

int RdfStorage::GetCurBucketNumber(int ts)
{
	int buc_num = (ts / 60 + 1) % NUM_BUCKETS;

	return buc_num;
}

int RdfStorage::Insert(const char *data, int data_len)
{
	char str1[32], str2[32] = {0};
	librdf_statement *statement = NULL;
	librdf_world *world = sm_world;

	/* Find the bucket number based on the cur minute */
	snprintf(str1, 32, "%ld", TimeManager::GetCurUpTime().tv_sec);
	//itoa(GetCurBucketNumber(TimeManager::GetCurUpTime().tv_sec), str1, 10);

	/* Find the timestamp based on the msec value */
	//itoa(TimeManager::GetCurUpTime().tv_usec/1000000, str2, 10);
	snprintf(str2, 32, "%ld", TimeManager::GetCurUpTime().tv_usec/1000000);

	//librdf_node *context_node = librdf_new_node_from_literal(world, (const unsigned char*)m_name.c_str(), NULL, 0);
	librdf_node *context_node = librdf_new_node_from_literal(world, (const unsigned char*)str1, NULL, 0);
	statement = librdf_new_statement_from_nodes(world, 
			//librdf_new_node_from_literal(world, (const unsigned char*)TIME_MANAGER->GetCurrentTimeStr().c_str(), NULL, 0),
			librdf_new_node_from_literal(world, (const unsigned char*)str2, NULL, 0),
			librdf_new_node_from_uri_string(world, (const unsigned char*)"http://asu.edu.smarthome.smartgateway/DataManager/hasValue"),
			librdf_new_node_from_typed_counted_literal(world, (const unsigned char*)data, data_len, NULL, 0, NULL));
	librdf_model_context_add_statement(m_model, context_node, statement);
	return 0;
}

/*
 * The key here should be the start or end timestamp
 */
RdfStorage::Iterator RdfStorage::Find(const char *key)
{
	librdf_world *world = sm_world;
	librdf_statement *statement = librdf_new_statement_from_nodes(world,
			//librdf_new_node_from_literal(world, (const unsigned char*)TIME_MANAGER->GetCurrentTimeStr().c_str(), NULL, 0),
			librdf_new_node_from_literal(world, (const unsigned char*)"timestamp", NULL, 0),
			librdf_new_node_from_uri_string(world, (const unsigned char*)"http://asu.edu.smarthome.smartgateway/DataManager/hasValue"),
			librdf_new_node_from_literal(world, (const unsigned char*)"tmp", NULL, 0));

	librdf_stream *stream = librdf_model_find_statements(m_model, statement);

	return RdfStorage::Iterator(stream);
}

template <>
RdfStorage::Iterator& RdfStorage::Iterator::operator=(const RdfStorage::Iterator& other)
{
	SetElem(other.GetElem());
	return *this;
}

template <>
bool RdfStorage::Iterator::operator==(const RdfStorage::Iterator& other)
{
	librdf_statement *s1, *s2;
	s1 = librdf_stream_get_object(GetElem());
	s2 = librdf_stream_get_object(other.GetElem());
	return librdf_statement_equals(s1, s2);
}

template <>
bool RdfStorage::Iterator::operator!=(const RdfStorage::Iterator& other)
{
	librdf_statement *s1, *s2;
	s1 = librdf_stream_get_object(GetElem());
	s2 = librdf_stream_get_object(other.GetElem());
	return !librdf_statement_equals(s1, s2);
}

template <>
RdfStorage::Iterator& RdfStorage::Iterator::operator++()
{
	librdf_stream_next(GetElem());
	return *this;
}

template <>
RdfStorage::Iterator& RdfStorage::Iterator::operator++(int)
{
	//Iterator tmp(*this);
	//++(*this);
	librdf_stream_next(GetElem());
	return *this;
}

/*
 * Use this to directly get the value of the object in the 
 * statement pointed to by this iterator.
 */
template <>
string RdfStorage::Iterator::operator*()
{
	string str;
	librdf_statement *s = librdf_stream_get_object(GetElem());
	if (NULL == s)
	{
		return NULL;
	}

	librdf_node *node = librdf_statement_get_object(s);
	str.assign((const char *)librdf_node_get_literal_value(node));
	return str;
}

/*
 * Use this instead of the above, to get values of the subject 
 * or the object, as this will return the librdf_statement* pointed to by 
 * this iterator.
 */
template <>
void *RdfStorage::Iterator::operator->()
{
	librdf_statement *s;
	s = librdf_stream_get_object(GetElem());
	return s;
}


