#ifndef _TOX_THREAD_H_
#define _TOX_THREAD_H_

class CToxThread
{
private:
	Tox *tox;

public:
	CToxThread(Tox_Options *options, TOX_ERR_NEW *error = NULL)
		: tox(NULL)
	{
		tox = tox_new(options, error);
	}

	~CToxThread()
	{

		if (tox)
		{
			tox_kill(tox);
			tox = NULL;
		}
	}

	Tox* Tox()
	{
		return tox;
	}
};

#endif //_TOX_THREAD_H_