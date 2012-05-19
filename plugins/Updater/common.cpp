#include "common.h"

UpdateList::UpdateList(): count(0), head(0), tail(0) {
}

UpdateList::~UpdateList() {
	clear();
}

UpdateList::UpdateList(UpdateList &source): count(0), head(0), tail(0) {
	for(source.reset(); source.current(); source.next()) 
		push_back(*source.current());
}

void UpdateList::clear() {
	Node *current;
	while(head) {
		current = head;
		head = head->next;
		delete current;
	}

	count = 0;
	head = tail = 0;
	reset();
}

int UpdateList::size() {
	return count;
}

void UpdateList::reset() {
	it_current = head;
}

void UpdateList::erase() {
	if(it_current) {
		if(head == it_current) head = head->next;
		if(tail == it_current) tail = tail->prev;

		if(it_current->next) it_current->next->prev = it_current->prev;
		if(it_current->prev) it_current->prev->next = it_current->next;

		delete it_current;
		count--;
		reset();
	}
}

void UpdateList::next() {
	if(it_current) it_current = (Node *)it_current->next;
}

UpdateInternal *UpdateList::current() {
	return (it_current ? &it_current->ui : 0);
}

void UpdateList::push_back(UpdateInternal &update) {
	if(tail) {
		tail->next = new Node;
		tail->next->prev = tail;
		tail = tail->next;
	} else {
		head = tail = new Node;
	}
	tail->ui = update;
	count++;
}

UpdateInternal &UpdateList::back() {
	return tail->ui;
}
