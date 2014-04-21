#include "common.h"

PopupHistoryList::PopupHistoryList(int renderer)
{
	this->renderer = renderer;
	size = HISTORY_SIZE; //fixed size (at least for now)
	historyData = (PopupHistoryData *) malloc(size * sizeof(PopupHistoryData)); //alloc space for data
	count = 0;
}

PopupHistoryList::~PopupHistoryList()
{
	Clear(); //clear the data strings
	free(historyData); //deallocate the data list
}

void PopupHistoryList::Clear()
{
	for (int i = 0; i < count; i++)
	{
		DeleteData(i);
	}
	count = 0;
}

void PopupHistoryList::RemoveItem(int index)
{
	DeleteData(index); //free the mem for that particular item
	for (int i = index + 1; i < count; i++)
	{
		historyData[i - 1] = historyData[i]; //shift all items to the left
	}
}

void PopupHistoryList::DeleteData(int index)
{
	PopupHistoryData *item = &historyData[index];
	mir_free(item->titleT);
	mir_free(item->messageT);
	item->timestamp = 0; //invalidate item
	item->title = NULL;
	item->message = NULL;
	item->flags = 0;
}

void PopupHistoryList::AddItem(PopupHistoryData item)
{
	if (count >= size)
	{
		RemoveItem(0); //remove first element - the oldest
		count--; //it will be inc'ed later
	}
	historyData[count++] = item; //item has it's relevant strings dupped()
	RefreshPopupHistory(hHistoryWindow, GetRenderer());
}

void PopupHistoryList::Add(char *title, char *message, time_t timestamp)
{
	PopupHistoryData item = {0}; //create a history item
	item.timestamp = timestamp;
	item.title = mir_strdup(title);
	item.message = mir_strdup(message);
	AddItem(item); //add it (flags = 0)
}

void PopupHistoryList::Add(wchar_t *title, wchar_t *message, time_t timestamp)
{
	PopupHistoryData item = {0}; //create an unicode history item
	item.flags = PHDF_UNICODE; //mark it as unicode
	item.timestamp = timestamp;
	item.titleT = mir_u2t(title);
	item.messageT = mir_u2t(message);
	AddItem(item); //add it
}

PopupHistoryData *PopupHistoryList::Get(int index)
{
	if ((index < 0) || (index >= count)) //a bit of sanity check
	{
		return NULL;
	}
	
	return &historyData[index];
}
