#ifndef cserver_h__
#define cserver_h__

class IConnectionProcessor
{
public:
	virtual ~IConnectionProcessor() {}
	virtual void ProcessConnection() = 0;
};

class IConnectionProcessorFactory
{
public:
	virtual IConnectionProcessor *Create(CSocket *s) = 0;
};

class CServer
{
private:
	SOCKET m_socket;
	IConnectionProcessorFactory *m_connectionProcessorFactory;

	DWORD ConnectionAcceptThread();
	DWORD ConnectionProcessThread(SOCKET s);

	static void GlobalConnectionAcceptThread(void *arg);

	struct GlobalConnectionProcessThreadArgs
	{
		CServer *m_server;
		SOCKET m_socket;

		GlobalConnectionProcessThreadArgs(CServer *server, SOCKET s): m_server(server), m_socket(s) {}
	};
	static void GlobalConnectionProcessThread(void *arg);

public:
	void Start(int port, IConnectionProcessorFactory *connectionProcessorFactory, bool background);
	void Stop();
};

#endif // cserver_h__
