#include "participant.h"

CParticipant::CParticipant(unsigned int oid, SERootObject* root) : Participant(oid, root) { }

void CParticipant::OnChange(int prop)
{
	int i = 0;
}