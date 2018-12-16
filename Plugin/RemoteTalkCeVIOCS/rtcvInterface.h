#pragma once

class rtcvTalkInterface : public rtcvITalkInterface
{
public:
    rtDefSingleton(rtcvTalkInterface);

    rtcvTalkInterface();
    ~rtcvTalkInterface() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    bool setParams(const rt::TalkParams& params) override;
    int getNumCasts() const override;
    bool getCastInfo(int i, rt::CastInfo *dst) const override;
    bool setText(const char *text) override;

    bool ready() const override;
    bool talk(rt::TalkSampleCallback cb, void *userdata) override;
    bool stop() override;
    bool wait() override;

    void onUpdateBuffer(rt::AudioData& ad) override;
#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::CastList m_casts;
    rt::TalkParams m_params;
    std::atomic_bool m_is_playing{ false };

    rt::TalkSampleCallback m_sample_cb = nullptr;
    void *m_sample_cb_userdata = nullptr;
};