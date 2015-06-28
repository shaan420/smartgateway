#ifndef __TIME_MANAGER_HPP__
#define __TIME_MANAGER_HPP__

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>

#define TIME_MANAGER s_timeManager->getInstance()

class TimeManager
{
	private:
		int CreateTimer();
		struct timeval m_start_time;
		struct timeval m_cur_time;
		sem_t m_timer_sem;
		timer_t m_timerId;
		pthread_t m_timerThread;

	public:

		TimeManager() {}
		~TimeManager() {}

		timer_t& GetTimerId()
		{
			return m_timerId;
		}

		TimeManager *getInstance();
		
		unsigned long long int GetStartTimeSec();
		unsigned long long int GetStartTimeMSec();
		struct timeval GetStartTime();
		
		unsigned long long int GetCurTimeSec();
		unsigned long long int GetCurTimeMSec();
		struct timeval& GetCurTime();

		struct timeval GetCurUpTime();

		sem_t& GetSem()
		{
			return m_timer_sem;
		}
		
		int Init();
};

extern TimeManager *s_timeManager;

#endif
