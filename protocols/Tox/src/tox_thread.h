#ifndef _TOX_THREAD_H_
#define _TOX_THREAD_H_

class CToxThread
{
public:
	Tox *tox;
	ToxAV *toxAV;
	bool isConnected;
	bool isTerminated;

	mir_cs toxLock;

	CToxThread() : tox(NULL), toxAV(NULL),
		isConnected(false), isTerminated(false) { }

	void Do()
	{
		{
			mir_cslock lock(toxLock);
			tox_iterate(tox);
			//if (toxAV)
			//	toxav_iterate(toxAV);
		}
		uint32_t interval = tox_iteration_interval(tox);
		interval ++; /* Prevent zero sleep interval. */
		Sleep(interval);
	}

	void Stop() { isTerminated = true; }
};

#endif //_TOX_THREAD_H_