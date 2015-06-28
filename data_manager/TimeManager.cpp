#include "TimeManager.hpp"

#include <iostream>
using namespace std;

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)

#define TIMER_FREQ_SEC 1

TimeManager *s_timeManager = NULL;

TimeManager *TimeManager::getInstance()
{
	if (NULL == s_timeManager)
	{
		s_timeManager = new TimeManager;
	}

	return s_timeManager;
}



static void *timer_func(void *data)
{
	struct itimerspec its;
	int s;

	/* Unblock the timer in this thread context */
	sigset_t sig_mask;
	sigemptyset(&sig_mask);
	sigaddset(&sig_mask, SIGRTMIN);
	pthread_sigmask(SIG_UNBLOCK, &sig_mask, NULL);

	/* Start the timer */
	its.it_value.tv_sec = TIMER_FREQ_SEC;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = TIMER_FREQ_SEC;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(TIME_MANAGER->GetTimerId(), 0, &its, NULL) == -1)
		errExit("timer_settime");

	while (1) 
	{
		while ((s = sem_wait(&TIME_MANAGER->GetSem())) == -1 && errno == EINTR)
			continue; 
		gettimeofday(&TIME_MANAGER->GetCurTime(), NULL);
	}

	return NULL;
}

static void timer_sig_handler(int sig, siginfo_t *si, void *uc)
{
	sem_post(&TIME_MANAGER->GetSem());
}

int TimeManager::CreateTimer()
{
	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;

	/* Establish handler for timer signal */
	printf("Establishing handler for signal %d\n", SIGRTMIN);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_sig_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGRTMIN, &sa, NULL) == -1)
		errExit("sigaction");

	/* Block timer from the calling main thread */
	printf("Blocking signal %d\n", SIGRTMIN);
	sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
		errExit("sigprocmask");

	/* Create the timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &m_timerId;
	if (timer_create(CLOCK_REALTIME, &sev, &m_timerId) == -1)
		errExit("timer_create");

	printf("timer ID is 0x%lx\n", (long) m_timerId);

	return 0;
}

int TimeManager::Init()
{
	/* Create timer */
	CreateTimer();

	if (0 == m_timerId)
	{
		cout << "ERR: could not create timer ID\n";
		return -1;
	}

	/* Create timer thread */
	int ret = pthread_create(&m_timerThread, NULL, &timer_func, &m_timerId);

	if (0 != ret)
	{
		cout << "ERR: creating timer thread\n";
		return -1;
	}

	/* Set the start time */
	gettimeofday(&m_start_time, NULL);

	/* Initialize the timer semaphore */
	if (sem_init(&GetSem(), 0, 0) == -1)
	{
		cout << "ERR: sem_init()\n";
		return -1;
	}

	return 0;
}

unsigned long long int TimeManager::GetCurTimeSec()
{
	return m_cur_time.tv_sec;
}

unsigned long long int TimeManager::GetCurTimeMSec()
{
	return (m_cur_time.tv_sec*1000 + m_cur_time.tv_usec/1000000);
}

unsigned long long int TimeManager::GetStartTimeSec()
{
	return m_start_time.tv_sec;
}

unsigned long long int TimeManager::GetStartTimeMSec()
{
	return (m_start_time.tv_sec*1000 + m_start_time.tv_usec/1000000);
}

struct timeval TimeManager::GetStartTime()
{
	return m_start_time;
}

struct timeval& TimeManager::GetCurTime()
{
	return m_cur_time;
}

struct timeval TimeManager::GetCurUpTime()
{
	int msec = GetCurTimeMSec() - GetStartTimeMSec();

	struct timeval t;
	t.tv_sec = msec / 1000;
	t.tv_usec = (msec * 1000000) % 1000000000;

	return t;
}
