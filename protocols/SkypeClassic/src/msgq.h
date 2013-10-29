#include <time.h>

// ----------------------------------------------------------------------
// Stolen from *nix sys/queue.h
// ----------------------------------------------------------------------
/*
 * Tail queue definitions.
 */
#define TAILQ_HEAD(name, type)                                          \
struct name {                                                           \
        struct type *tqh_first; /* first element */                     \
        struct type **tqh_last; /* addr of last next element */         \
}

#define TAILQ_ENTRY(type)                                               \
struct {                                                                \
        struct type *tqe_next;  /* next element */                      \
        struct type **tqe_prev; /* address of previous next element */  \
}


/*
 * Tail queue functions.
 */
#define TAILQ_INIT(head) {                                              \
        (head)->tqh_first = NULL;                                       \
        (head)->tqh_last = &(head)->tqh_first;                          \
}

#define TAILQ_INSERT_TAIL(head, elm, field) {                           \
        (elm)->field.tqe_next = NULL;                                   \
        (elm)->field.tqe_prev = (head)->tqh_last;                       \
        *(head)->tqh_last = (elm);                                      \
        (head)->tqh_last = &(elm)->field.tqe_next;                      \
}

#define TAILQ_REMOVE(head, elm, field) {                                \
        if (((elm)->field.tqe_next) != NULL)                            \
                (elm)->field.tqe_next->field.tqe_prev =                 \
                    (elm)->field.tqe_prev;                              \
        else                                                            \
                (head)->tqh_last = (elm)->field.tqe_prev;               \
        *(elm)->field.tqe_prev = (elm)->field.tqe_next;                 \
}
// ----------------------------------------------------------------------

struct MsgQueue {
	TAILQ_ENTRY(MsgQueue) l;
	char *message;
	time_t tAdded;
	time_t tReceived;
};
typedef struct
{
	TAILQ_HEAD(tag_msgq, MsgQueue) l;
	CRITICAL_SECTION cs;
} TYP_MSGQ;

void MsgQ_Init(TYP_MSGQ *q);
void MsgQ_Exit(TYP_MSGQ *q);
BOOL MsgQ_Add(TYP_MSGQ *q, char *msg);
char *MsgQ_RemoveMsg(TYP_MSGQ *q, struct MsgQueue *ptr);
char *MsgQ_Get(TYP_MSGQ *q);
int MsgQ_CollectGarbage(TYP_MSGQ *q, time_t age);
