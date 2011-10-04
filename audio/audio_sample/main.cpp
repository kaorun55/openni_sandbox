#include <iostream>
#include <XnCppWrapper.h>

void main()
{
    try {
        // コンテキストを初期化する
        xn::Context context;
        XnStatus nRetVal = context.Init();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // オーディオジェネレータを初期化する
        xn::AudioGenerator audio;
        nRetVal = audio.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // WAVEデータの設定をする
        XnWaveOutputMode waveMode;
        waveMode.nSampleRate = 44100;
        waveMode.nChannels = 2;
        waveMode.nBitsPerSample = 16;
        nRetVal = audio.SetWaveOutputMode(waveMode);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // データの取得を開始する
        context.StartGeneratingAll();

        std::cout << "Press any key to exit..." << std::endl;

        while ( !xnOSWasKeyboardHit() ) {
            // データの更新を待つ
            nRetVal = context.WaitOneUpdateAll(audio);
            if (nRetVal != XN_STATUS_OK) {
                throw std::runtime_error(xnGetStatusString(nRetVal));
            }

            // データを取得する
            const XnUChar* pAudioBuf = audio.GetAudioBuffer();
            XnUInt32 nBufSize = audio.GetDataSize();
        }

        std::cout << "Success!!" << std::endl;
    }
    catch ( std::exception& ex ) {
        std::cout << ex.what() << std::endl;
    }
}
