#include "participant.h"

CParticipant::CParticipant(unsigned int oid, SERootObject* root) : Participant(oid, root) { }

SEString CParticipant::GetRankName(CParticipant::RANK rank)
{
	char *result = NULL;
	switch (rank)
	{
	case CParticipant::CREATOR:
		result = "Creator";
		break;
	case CParticipant::ADMIN:
		result = "Admin";
		break;
	case CParticipant::SPEAKER:
		result = "Speaker";
		break;
	case CParticipant::WRITER:
		result = "Writer";
		break;
	case CParticipant::SPECTATOR:
		result = "Spectator";
		break;
	case CParticipant::RETIRED:
		result = "Retried";
		break;
	case CParticipant::OUTLAW:
		result = "Outlaw";
		break;
	}
	return result;
}