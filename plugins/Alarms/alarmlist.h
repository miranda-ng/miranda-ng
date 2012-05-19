#ifndef _ALARMLIST_INC
#define _ALARMLIST_INC

#include "options.h"
#include "time_utils.h"
#include "alarm_win.h"

#include "m_alarms.h"

void free_alarm_data(ALARM *alarm);

void copy_alarm_data(ALARM *dest, ALARM *src);

int MinutesInFuture(SYSTEMTIME time, Occurrence occ);
void TimeForMinutesInFuture(int mins, SYSTEMTIME *time);

static bool operator<(const ALARM &a1, const ALARM &a2) {
	return MinutesInFuture(a1.time, a1.occurrence) < MinutesInFuture(a2.time, a2.occurrence); // less-than inverted 'cause we want ascending order
}

class AlarmList {
public:
	AlarmList(): head(0), tail(0), count(0) {}

	virtual ~AlarmList() {clear();}

	void sort() {
		if(count < 2) return;

		Node *c1 = head, *c2;
		// bubble sort...hey, i'm lazy :)
		while(c1) {
			c2 = c1->next;
			while(c2) {
				if(c2->alarm < c1->alarm) {
					swap (c1, c2);
				}
				c2 = c2->next;
			}
			c1 = c1->next;
		}
	}

	void clear() {
		Node *current;
		while(head) {
			current = head;
			head = head->next;
			free_alarm_data(&current->alarm);
			delete current;
		}
		count = 0;
		tail = 0;
		reset();
	}

	int size() {return count;}

	ALARM &at(int index) {
		int i = 0;
		Node *current = head;
		while(i < index && i < count && current) {
			current = current->next;
			i++;
		}
		return current->alarm;
	}

	void reset() {it_current = head;}
	ALARM *current() {return (it_current ? &it_current->alarm : 0);}
	void next() {it_current = it_current->next;}
	void erase() {
		if(it_current) {
			if(it_current->next) it_current->next->prev = it_current->prev;
			if(it_current->prev) it_current->prev->next = it_current->next;

			if(tail == it_current) tail = tail->prev;
			if(head == it_current) head = head->next;

			free_alarm_data(&it_current->alarm);
			delete it_current;
			count--;
			reset();
		}
	}

	// copies the alarm into the list
	void push_back(ALARM *alarm) {
		Node *nn = new Node;
		memset(&nn->alarm, 0, sizeof(ALARM));
		copy_alarm_data(&nn->alarm, alarm);

		nn->prev = tail;
		if(tail) tail->next = nn;
		tail = nn;

		if(!head) head = tail;
		count++;
	}

protected:
	class Node {
	public:
		Node(): next(0), prev(0) {}
		ALARM alarm;
		Node *next, *prev;
	};

	Node *head, *tail, *it_current;
	int count;

	void swap(Node *n1, Node *n2) {
		ALARM temp = n1->alarm;
		n1->alarm = n2->alarm;
		n2->alarm = temp;
	}
};

//extern AlarmList alarms;

void LoadAlarms();
void SaveAlarms();

void InitList();
void DeinitList();

void copy_list(AlarmList &copy);
void copy_list(AlarmList &copy, SYSTEMTIME &start, SYSTEMTIME &end);

void set_list(AlarmList &copy);

void append_to_list(ALARM *alarm);
void alter_alarm_list(ALARM *alarm);
void remove(unsigned short alarm_id);

void suspend(unsigned short alarm_id);

void GetPluginTime(SYSTEMTIME *t);

// increase 'time' to next occurrence
bool UpdateAlarm(SYSTEMTIME &time, Occurrence occ);


const ULARGE_INTEGER mult = { 600000000, 0}; // number of 100 microsecond blocks in a minute

extern unsigned short next_alarm_id;

#endif
