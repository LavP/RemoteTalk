#include "pch.h"
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalk/rtAudioData.h"
#include "RemoteTalk/rtTalkInterfaceImpl.h"
#include "RemoteTalk/rtSerialization.h"
#include "RemoteTalkVOICEROIDCommon.h"
#include <atomic>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Controls;

ref class rtvr2InterfaceManaged
{
public:
    static rtvr2InterfaceManaged^ getInstance();

    bool prepareUI();
    rt::AvatorList getAvatorList();

    bool getParams(rt::TalkParams& params);
    bool setParams(const rt::TalkParams& params);
    bool setText(const char *text);

    bool stop();
    bool talk();

private:
    static rtvr2InterfaceManaged s_instance;

    bool setupControls();

    TextBox^ m_tb_text;
    Button^ m_bu_play;
    Button^ m_bu_stop;
    Button^ m_bu_rewind;
    ListView^ m_lv_avators;
    Slider ^m_sl_volume, ^m_sl_speed, ^m_sl_pitch, ^m_sl_intonation, ^m_sl_joy, ^m_sl_anger, ^m_sl_sorrow;

    ref class AvatorInfo
    {
    public:
        int id;
        String^ name;

        AvatorInfo(int i,  String^ n) : id(i), name(n) {}
    };
    List<AvatorInfo^>^ m_avators;
};

class rtvr2TalkInterfaceImpl : public rtvr2TalkInterface
{
public:
    rtDefSingleton(rtvr2TalkInterfaceImpl);

    rtvr2TalkInterfaceImpl();
    ~rtvr2TalkInterfaceImpl() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    bool setParams(const rt::TalkParams& params) override;
    int getNumAvators() const override;
    bool getAvatorInfo(int i, rt::AvatorInfo *dst) const override;
    bool setText(const char *text) override;

    bool ready() const override;
    bool talk(rt::TalkSampleCallback cb, void *userdata) override;
    bool stop() override;

    bool prepareUI() override;
    void onPlay() override;
    void onStop() override;
    void onUpdateBuffer(const rt::AudioData& ad) override;

#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::AvatorList m_avators;
    std::atomic_bool m_is_playing{ false };
    rt::TalkSampleCallback m_sample_cb = nullptr;
    void *m_sample_cb_userdata = nullptr;
};


static void SelectControlsByTypeNameImpl(System::Windows::DependencyObject^ obj, String^ name, List<System::Windows::DependencyObject^>^ dst, bool one)
{
    if (obj->GetType()->FullName == name) {
        dst->Add(obj);
        if (one)
            return;
    }

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        SelectControlsByTypeNameImpl(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), name, dst, one);
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(System::Windows::DependencyObject^ obj, String^ name, bool one)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    SelectControlsByTypeNameImpl(obj, name, ret, one);
    return ret;
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(String^ name, bool one)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows)
            SelectControlsByTypeNameImpl(w, name, ret, one);
    }
    return ret;
}

static bool EmulateClick(Button^ button)
{
    if (!button)
        return true;
    if (!button->IsEnabled)
        return false;
    using namespace System::Windows::Automation;
    auto peer = gcnew Peers::ButtonAutomationPeer(button);
    auto invoke = (Provider::IInvokeProvider^)peer->GetPattern(Peers::PatternInterface::Invoke);
    invoke->Invoke();
    return true;
}

static void UpdateValue(Slider^ slider, double v)
{
    if (!slider)
        return;
    slider->Value = v;
}

static std::string ToStdString(String^ str)
{
    IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
    return rt::ToUTF8((const char*)ptr.ToPointer());
}


rtvr2InterfaceManaged^ rtvr2InterfaceManaged::getInstance()
{
    return %s_instance;
}

bool rtvr2InterfaceManaged::prepareUI()
{
    auto ttcs = SelectControlsByTypeName("AI.Framework.Wpf.Controls.TitledTabControl", true);
    if (ttcs->Count < 2)
        return true;

    auto tc = dynamic_cast<TabControl^>(ttcs[1]);
    // select "voice" tab
    if (tc->SelectedIndex != 1) {
        tc->SelectedIndex = 1;
        return false;
    }

    setupControls();
    return true;
}

rt::AvatorList rtvr2InterfaceManaged::getAvatorList()
{
    setupControls();

    rt::AvatorList ret;
    if (m_avators) {
        for each(auto ti in m_avators)
            ret.push_back({ ti->id, ToStdString(ti->name) });
    }
    return ret;
}

bool rtvr2InterfaceManaged::getParams(rt::TalkParams& params)
{
    if (m_sl_volume)    params.setVolume((float)m_sl_volume->Value);
    if (m_sl_speed)     params.setSpeed((float)m_sl_speed->Value);
    if (m_sl_pitch)     params.setPitch((float)m_sl_pitch->Value);
    if (m_sl_intonation)params.setIntonation((float)m_sl_intonation->Value);
    if (m_sl_joy)       params.setJoy((float)m_sl_joy->Value);
    if (m_sl_anger)     params.setAnger((float)m_sl_anger->Value);
    if (m_sl_sorrow)    params.setSorrow((float)m_sl_sorrow->Value);
    if (m_lv_avators)   params.setAvator(m_lv_avators->SelectedIndex);
    return true;
}

bool rtvr2InterfaceManaged::setParams(const rt::TalkParams& params)
{
    if (params.flags.avator && m_lv_avators)
        m_lv_avators->SelectedIndex = params.avator;

#define DoSetParam(N) if (params.flags.N && m_sl_##N) UpdateValue(m_sl_##N, params.N);
    DoSetParam(volume);
    DoSetParam(speed);
    DoSetParam(pitch);
    DoSetParam(intonation);
    DoSetParam(joy);
    DoSetParam(anger);
    DoSetParam(sorrow);
#undef DoSetParam

    return true;
}

bool rtvr2InterfaceManaged::setText(const char *text)
{
    if (!m_tb_text)
        return true;
    m_tb_text->Text = gcnew String(text);
    return true;
}

bool rtvr2InterfaceManaged::setupControls()
{
    if (!m_tb_text) {
        auto tev = SelectControlsByTypeName("AI.Talk.Editor.TextEditView", true);
        if (tev->Count == 0)
            return false;

        auto tb = SelectControlsByTypeName(tev[0], "AI.Framework.Wpf.Controls.TextBoxEx", true);
        if (tb->Count == 0)
            return false;

        m_tb_text = dynamic_cast<TextBox^>(tb[0]);

        auto buttons = SelectControlsByTypeName(tev[0], "System.Windows.Controls.Button", false);
        if (buttons->Count == 0)
            return false;

        m_bu_play = dynamic_cast<Button^>(buttons[0]);
        if (buttons->Count >= 1)
            m_bu_stop = dynamic_cast<Button^>(buttons[1]);
        if (buttons->Count >= 2)
            m_bu_rewind = dynamic_cast<Button^>(buttons[2]);

        auto vplv = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetListView", true);
        if (vplv->Count >= 1) {
            auto listview = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListView", true);
            if (listview->Count >= 1) {
                m_lv_avators = dynamic_cast<ListView^>(listview[0]);

                m_avators = gcnew List<AvatorInfo^>();
                int index = 0;
                auto items = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListViewItem", false);
                for each(ListViewItem^ item in items) {
                    auto tbs = SelectControlsByTypeName(item, "System.Windows.Controls.TextBlock", false);
                    if (tbs->Count >= 2) {
                        auto tb = dynamic_cast<TextBlock^>(tbs[1]);
                        m_avators->Add(gcnew AvatorInfo(index++, tb->Text));
                    }
                }
            }
        }
    }

    {
        auto vpev = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetEditView", true);
        if (vpev->Count >= 1) {
            auto lfs = SelectControlsByTypeName(vpev[0], "AI.Framework.Wpf.Controls.LinearFader", false);
            for (int lfi = 0; lfi < lfs->Count; ++lfi) {
                Slider^ slider = nullptr;
                auto sls = SelectControlsByTypeName(lfs[lfi], "System.Windows.Controls.Slider", true);
                if (sls->Count >= 1)
                    slider = dynamic_cast<Slider^>(sls[0]);

                switch (lfi)
                {
                case 0: m_sl_volume = slider; break;
                case 1: m_sl_speed = slider; break;
                case 2: m_sl_pitch = slider; break;
                case 3: m_sl_intonation = slider; break;
                case 4: break;
                case 5: break;
                case 6: m_sl_joy = slider; break;
                case 7: m_sl_anger = slider; break;
                case 8: m_sl_sorrow = slider; break;
                default: break;
                }
            }
            if (lfs->Count < 6) {
                m_sl_joy = m_sl_anger = m_sl_sorrow = nullptr;
            }
        }
    }
    return true;
}


bool rtvr2InterfaceManaged::stop()
{
    if (!setupControls())
        return true;
    return EmulateClick(m_bu_stop);
}

bool rtvr2InterfaceManaged::talk()
{
    return EmulateClick(m_bu_rewind) && EmulateClick(m_bu_play);
}



rtvr2TalkInterfaceImpl::rtvr2TalkInterfaceImpl()
{
}

rtvr2TalkInterfaceImpl::~rtvr2TalkInterfaceImpl()
{
    auto mod = ::GetModuleHandleA("RemoteTalkVOICEROID2Hook.dll");
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void rtvr2TalkInterfaceImpl::release() { /*do nothing*/ }
const char* rtvr2TalkInterfaceImpl::getClientName() const { return "VOICEROID2"; }
int rtvr2TalkInterfaceImpl::getPluginVersion() const { return rtPluginVersion; }
int rtvr2TalkInterfaceImpl::getProtocolVersion() const { return rtProtocolVersion; }

bool rtvr2TalkInterfaceImpl::getParams(rt::TalkParams& params) const
{
    return rtvr2InterfaceManaged::getInstance()->getParams(params);
}

bool rtvr2TalkInterfaceImpl::setParams(const rt::TalkParams& params)
{
    return rtvr2InterfaceManaged::getInstance()->setParams(params);
}

int rtvr2TalkInterfaceImpl::getNumAvators() const
{
    if (m_avators.empty())
        m_avators = rtvr2InterfaceManaged::getInstance()->getAvatorList();
    return (int)m_avators.size();
}

bool rtvr2TalkInterfaceImpl::getAvatorInfo(int i, rt::AvatorInfo *dst) const
{
    if (i < (int)m_avators.size()) {
        dst->id = m_avators[i].id;
        dst->name = m_avators[i].name.c_str();
        return true;
    }
    return false;
}

bool rtvr2TalkInterfaceImpl::setText(const char *text)
{
    return rtvr2InterfaceManaged::getInstance()->setText(text);
}

bool rtvr2TalkInterfaceImpl::ready() const
{
    return !m_is_playing;
}

bool rtvr2TalkInterfaceImpl::talk(rt::TalkSampleCallback cb, void *userdata)
{
    if (m_is_playing)
        return false;

    m_sample_cb = cb;
    m_sample_cb_userdata = userdata;
    if (rtvr2InterfaceManaged::getInstance()->talk()) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}

bool rtvr2TalkInterfaceImpl::stop()
{
    if (!m_is_playing)
        return false;
    return rtvr2InterfaceManaged::getInstance()->stop();
}


bool rtvr2TalkInterfaceImpl::prepareUI()
{
    return rtvr2InterfaceManaged::getInstance()->prepareUI();
}

void rtvr2TalkInterfaceImpl::onPlay()
{
    m_is_playing = true;
}

void rtvr2TalkInterfaceImpl::onStop()
{
    if (m_sample_cb && m_is_playing) {
        rt::AudioData dummy;
        auto sd = rt::ToTalkSample(dummy);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
    m_is_playing = false;
}

void rtvr2TalkInterfaceImpl::onUpdateBuffer(const rt::AudioData& ad)
{
    if (m_sample_cb && m_is_playing) {
        auto sd = rt::ToTalkSample(ad);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
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

bool rtvr2TalkInterfaceImpl::onDebug()
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            PrintControlInfo(w);
        }
    }
    return true;
}
#endif


rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtvr2TalkInterfaceImpl::getInstance();
}
