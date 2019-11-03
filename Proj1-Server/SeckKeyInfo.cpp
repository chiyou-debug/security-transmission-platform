#include "SeckKeyInfo.h"
#include <string.h>

SeckKeyInfo::SeckKeyInfo() : status(false), seckeyID(0)
{
	bzero(clientID, sizeof(clientID));
	bzero(serverID, sizeof(serverID));
	bzero(seckey, sizeof(seckey));
}

SeckKeyInfo::~SeckKeyInfo()
{
}
