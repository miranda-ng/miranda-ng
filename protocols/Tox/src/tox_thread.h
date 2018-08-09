#ifndef _TOX_THREAD_H_
#define _TOX_THREAD_H_

class CToxThread
{
private:
	Tox *tox;
	bool isTerminated;

public:
	CToxThread(Tox_Options *options, TOX_ERR_NEW *error = nullptr)
		: tox(nullptr), isTerminated(false)
	{
		tox = tox_new(options, error);
	}

	~CToxThread()
	{

		if (tox)
		{
			tox_kill(tox);
			tox = nullptr;
		}
	}

	Tox* Tox()
	{
		return tox;
	}

	bool IsTerminated()
	{
		return isTerminated;
	}

	void Terminate()
	{
		isTerminated = true;
	}
};

#endif //_TOX_THREAD_H_