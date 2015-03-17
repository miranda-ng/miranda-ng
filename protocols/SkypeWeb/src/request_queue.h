#ifndef _SKYPE_REQUEST_QUEUE_H_
#define _SKYPE_REQUEST_QUEUE_H_

typedef void (*HttpResponseCallback)(const NETLIBHTTPREQUEST *response, void *arg);

struct RequestQueueItem
{
	void *arg;
	HttpRequest *request;
	HttpResponseCallback responseCallback;

	RequestQueueItem(HttpRequest *request, void *arg) :
		request(request), responseCallback(NULL), arg(arg) { }

	RequestQueueItem(HttpRequest *request, HttpResponseCallback response, void *arg) :
		request(request), responseCallback(response), arg(arg) { }

	~RequestQueueItem()
	{
		delete request;
		request = NULL;
		responseCallback = NULL;
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

	static unsigned int __cdecl WorkerThread(void*, void*);

public:
	RequestQueue(HANDLE hConnection);
	~RequestQueue();

	void Start();
	void Stop();

	void Push(HttpRequest *request, HttpResponseCallback response = NULL, void *arg = NULL);

};

#endif //_SKYPE_REQUEST_QUEUE_H_