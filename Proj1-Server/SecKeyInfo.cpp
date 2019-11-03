#include "SecKeyInfo.h"
#include <string.h>


SecKeyInfo::SecKeyInfo() : status(false), seckeyID(0)
{
	bzero(clientID, sizeof(clientID));
	bzero(serverID, sizeof(serverID));
	bzero(seckey, sizeof(seckey));
}


SecKeyInfo::~SecKeyInfo()
{
}
