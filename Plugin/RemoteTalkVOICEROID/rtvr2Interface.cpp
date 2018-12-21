#include "pch.h"
#include "rtvr2Common.h"
#include "rtvr2InterfaceManaged.h"
#include "rtvr2Interface.h"


rtvr2TalkInterface::rtvr2TalkInterface()
{
}

rtvr2TalkInterface::~rtvr2TalkInterface()
{
    auto mod = ::GetModuleHandleA(rtvr2HookDll);
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void rtvr2TalkInterface::release() { /*do nothing*/ }
const char* rtvr2TalkInterface::getClientName() const { return rtvr2HostName; }
int rtvr2TalkInterface::getPluginVersion() const { return rtPluginVersion; }
int rtvr2TalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool rtvr2TalkInterface::getParams(rt::TalkParams& params) const
{
    return rtvr2InterfaceManaged::getInstance()->getParams(params);
}

bool rtvr2TalkInterface::setParams(const rt::TalkParams& params)
{
    return rtvr2InterfaceManaged::getInstance()->setParams(params);
}

int rtvr2TalkInterface::getNumCasts() const
{
    m_casts = rtvr2InterfaceManaged::getInstance()->getCastList();
    return (int)m_casts.size();
}

const rt::CastInfo* rtvr2TalkInterface::getCastInfo(int i) const
{
    if (i >= 0 && i < (int)m_casts.size())
        return &m_casts[i];
    return nullptr;
}

bool rtvr2TalkInterface::setText(const char *text)
{
    return rtvr2InterfaceManaged::getInstance()->setText(text);
}

bool rtvr2TalkInterface::isReady() const
{
    return !m_is_playing;
}

bool rtvr2TalkInterface::isPlaying() const
{
    return m_is_playing;
}

bool rtvr2TalkInterface::play()
{
    if (m_is_playing)
        return false;

    if (rtvr2InterfaceManaged::getInstance()->talk()) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}

bool rtvr2TalkInterface::stop()
{
    if (!m_is_playing)
        return false;
    return rtvr2InterfaceManaged::getInstance()->stop();
}


bool rtvr2TalkInterface::setCast(int v)
{
    return rtvr2InterfaceManaged::getInstance()->setCast(v);
}

bool rtvr2TalkInterface::prepareUI()
{
    return rtvr2InterfaceManaged::getInstance()->prepareUI();
}

void rtvr2TalkInterface::onPlay()
{
    m_is_playing = true;
}

void rtvr2TalkInterface::onStop()
{
    m_is_playing = false;
}


#ifdef rtDebug
static void PrintControlInfo(System::Windows::DependencyObject^ obj, int depth = 0)
{
    std::string t;
    for (int i = 0; i < depth; ++i)
        t += "  ";
    t += ToStdString(obj->GetType()->FullName);
    t += "\n";
    ::OutputDebugStringA(t.c_str());

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        PrintControlInfo(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), depth + 1);
}

bool rtvr2TalkInterface::onDebug()
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            PrintControlInfo(w);
        }
    }
    return true;
}
#endif

