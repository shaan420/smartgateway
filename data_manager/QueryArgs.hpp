#ifndef __QUERY_ARGS_H__
#define __QUERY_ARGS_H__

typedef enum
{
	QUERY_OP_TYPE_CUR = 0,
	QUERY_OP_TYPE_AVG,
	QUERY_OP_TYPE_SUM,
	QUERY_OP_TYPE_IND,
	QUERY_OP_TYPE_UNKNOWN
} QueryOpType_t;

typedef struct QueryArgs
{
	private:
		int m_start_time;
		int m_end_time;
		QueryOpType_t m_op_type;

	public:
		QueryArgs() : m_start_time(0), m_end_time(0), m_op_type(QUERY_OP_TYPE_UNKNOWN)
		{
		}

		QueryArgs(int start, int end, QueryOpType_t op_type) 
			: m_start_time(start), m_end_time(end), m_op_type(op_type)
		{
		}

		int GetStartTime()
		{
			return m_start_time;
		}

		int GetEndTime()
		{
			return m_end_time;
		}

		QueryOpType_t GetOpType()
		{
			return m_op_type;
		}	
};

#endif
