#pragma once

#include "common.h"

class CParticipant : public Participant
{
public:
	typedef DRef<CParticipant, Participant> Ref;
	typedef DRefs<CParticipant, Participant> Refs;

	CParticipant(unsigned int oid, SERootObject* root);

	static SEString GetRankName(CParticipant::RANK rank);
};