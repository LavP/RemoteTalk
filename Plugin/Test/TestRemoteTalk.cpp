#include "pch.h"
#include "Test.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

static rt::TalkClientSettings GetClientSettings()
{
    rt::TalkClientSettings ret;
    GetArg("server", ret.server);
    int port;
    if (GetArg("port", port))
        ret.port = (uint16_t)port;
    return ret;
}


TestCase(RemoteTalkClient)
{
    rt::TalkClient client(GetClientSettings());
    client.setText("���[�A���[�A�܂����Ă���");
    client.send();
}