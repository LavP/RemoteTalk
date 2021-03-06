#pragma once

namespace rtcv {

class TalkInterface : public ITalkInterface
{
public:
    rtDefSingleton(TalkInterface);

    TalkInterface();
    ~TalkInterface() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    bool setParams(const rt::TalkParams& params) override;
    int getNumCasts() const override;
    rt::CastInfo* getCastInfo(int i) const override;
    bool setText(const char *text) override;
    void setTempFilePath(const char *path) override;


    bool isReady() const override;
    bool isPlaying() const override;
    bool play() override;
    bool stop() override;
    bool wait() override;

#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::CastList m_casts;
    rt::TalkParams m_params;
    std::atomic_bool m_is_playing{ false };
};

} // namespace rtcv
