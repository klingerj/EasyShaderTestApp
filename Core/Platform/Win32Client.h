#pragma once

namespace Tk
{
namespace Platform
{
namespace Network
{

int InitClient();
void CleanupClient();
int DisconnectFromServer();
int SendMessageToServer();

}
}
}
