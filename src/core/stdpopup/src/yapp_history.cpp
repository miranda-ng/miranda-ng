#include "stdafx.h"

PopupHistoryList::PopupHistoryList(int renderer)
{
	this->renderer = renderer;
	size = HISTORY_SIZE; //fixed size (at least for now)
	historyData = (PopupHistoryData *)malloc(size * sizeof(PopupHistoryData)); //alloc space for data
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
		DeleteData(i);

	count = 0;
}

void PopupHistoryList::RemoveItem(int index)
{
	DeleteData(index); //free the mem for that particular item
	for (int i = index + 1; i < count; i++) {
		historyData[i - 1] = historyData[i]; //shift all items to the left
	}
}

void PopupHistoryList::DeleteData(int index)
{
	PopupHistoryData *item = &historyData[index];
	replaceStrW(item->titleW, nullptr);
	replaceStrW(item->messageW, nullptr);
	item->timestamp = 0; // invalidate item
	item->flags = 0;
}

void PopupHistoryList::AddItem(PopupHistoryData item)
{
	if (count >= size) {
		RemoveItem(0); //remove first element - the oldest
		count--; //it will be inc'ed later
	}
	historyData[count++] = item; //item has it's relevant strings dupped()
	RefreshPopupHistory(hHistoryWindow, GetRenderer());
}

void PopupHistoryList::Add(wchar_t *title, wchar_t *message, time_t timestamp)
{
	PopupHistoryData item = { 0 }; //create an unicode history item
	item.flags = PHDF_UNICODE; //mark it as unicode
	item.timestamp = timestamp;
	item.titleW = mir_wstrdup(title);
	item.messageW = mir_wstrdup(message);
	AddItem(item); //add it
}

PopupHistoryData* PopupHistoryList::Get(int index)
{
	if ((index < 0) || (index >= count)) //a bit of sanity check
		return nullptr;

	return &historyData[index];
}
