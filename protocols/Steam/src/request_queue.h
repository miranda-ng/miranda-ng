#ifndef _REQUEST_QUEUE_H_
#define _REQUEST_QUEUE_H_

typedef void(*HttpResponseCallback)(const NETLIBHTTPREQUEST *response, void *arg);
typedef void(*HttpFinallyCallback)(void *arg);

struct RequestQueueItem
{
	void *arg;
	HttpRequest *request;
	HttpResponseCallback responseCallback;
	HttpFinallyCallback finallyCallback;

	RequestQueueItem(HttpRequest *request, void *arg, HttpFinallyCallback finallyCallback) :
		request(request), responseCallback(NULL), arg(arg), finallyCallback(finallyCallback)
	{
	}

	RequestQueueItem(HttpRequest *request, HttpResponseCallback response, void *arg, HttpFinallyCallback finallyCallback) :
		request(request), responseCallback(response), arg(arg), finallyCallback(finallyCallback)
	{
	}

	~RequestQueueItem()
	{
		delete request;
		request = NULL;
		responseCallback = NULL;
		finallyCallback = NULL;
	}
};

class RequestQueue
{
private:
	bool isTerminated;
	HANDLE hConnection;
	mir_cs requestQueueLock;
	LIST<RequestQueueItem> requests;
	HANDLE hRequestQueueEvent, hRequestQueueThread;

	void Execute(RequestQueueItem *item);

	static unsigned int __cdecl AsyncSendThread(void*, void*);
	static unsigned int __cdecl WorkerThread(void*);

public:
	RequestQueue(HANDLE hConnection);
	~RequestQueue();

	void Start();
	void Stop();

	void Push(HttpRequest *request, HttpResponseCallback response = NULL, void *arg = NULL, HttpFinallyCallback last = NULL);
	void Send(HttpRequest *request, HttpResponseCallback response = NULL, void *arg = NULL, HttpFinallyCallback last = NULL);

};

#endif //_REQUEST_QUEUE_H_