#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvHookHandler.h"
#include "rtcvTalkServer.h"


void rtcvWindowMessageHandler::afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret)
{
    auto& server = rtcvTalkServer::getInstance();
    server.start();
}


void rtcvWaveOutHandler::clearCallbacks()
{
    onUpdate = {};
}

void rtcvWaveOutHandler::afterWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret)
{
    if (FAILED(ret))
        return;
    auto& rec = m_records[*phwo];
    rec.is_opened = true;
    rec.wave_format = *pwfx;

    rec.data.frequency = rec.wave_format.nSamplesPerSec;
    rec.data.channels = rec.wave_format.nChannels;
    switch (rec.wave_format.wBitsPerSample) {
    case 8: rec.data.format = rt::AudioFormat::U8; break;
    case 16: rec.data.format = rt::AudioFormat::S16; break;
    case 24: rec.data.format = rt::AudioFormat::S24; break;
    case 32: rec.data.format = rt::AudioFormat::S32; break;
    }
}

void rtcvWaveOutHandler::beforeWaveOutClose(HWAVEOUT& hwo)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing) {
        rec.is_playing = false;
    }
    rec.is_opened = false;
}

void rtcvWaveOutHandler::beforeWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    rec.data.data.assign(pwh->lpData, pwh->lpData + pwh->dwBufferLength);
    if (!rec.is_playing) {
        rec.is_playing = true;
    }
    if (onUpdate)
        onUpdate(rec.data);
    if (mute)
        memset(pwh->lpData, 0, pwh->dwBufferLength);
}

void rtcvWaveOutHandler::beforeWaveOutReset(HWAVEOUT& hwo)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing)
        rec.is_playing = false;
    mute = false;
}
