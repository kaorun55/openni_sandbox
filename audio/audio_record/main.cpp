#include <iostream>
#include <vector>

#include <XnCppWrapper.h>
#include <mmsystem.h>

#define NUMBER_OF_AUDIO_BUFFERS 100

const char* RECORDE_PATH = "record.oni";

class AudioOutput
{
    xn::Context context;
    xn::AudioGenerator audio;
    XnWaveOutputMode waveMode;

    xn::Recorder recorder;
    xn::Player player;

    HWAVEOUT hWaveOut;
    std::vector<WAVEHDR>  AudioBuffers;

public:

    // OpenNIの初期化
    void initOpenNI()
    {
        // コンテキストの初期化
        XnStatus nRetVal = context.Init();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // デバイスを作成する(XMLからの生成だと、デバイスがないといわれる)
        nRetVal = audio.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // 取得するWAVEデータの設定
        waveMode.nSampleRate = 44100;
        waveMode.nChannels = 2;
        waveMode.nBitsPerSample = 16;
        nRetVal = audio.SetWaveOutputMode(waveMode);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // レコーダーの作成
        nRetVal = recorder.Create( context );
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // 出力先の設定
        nRetVal = recorder.SetDestination( XN_RECORD_MEDIUM_FILE, RECORDE_PATH );
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // 記録するノードの追加
        nRetVal = recorder.AddNodeToRecording(audio, XN_CODEC_UNCOMPRESSED);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // 記録の開始
        nRetVal = recorder.Record();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // データの取得を開始する
        context.StartGeneratingAll();
    }

    // OpenNIの初期化
    void initOpenNI( const std::string& recordFileName )
    {
        // コンテキストの初期化
        XnStatus nRetVal = context.Init();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // 記録したファイルを開く
        nRetVal = context.OpenFileRecording( recordFileName.c_str() );
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // プレーヤーを作成する
        nRetVal = context.FindExistingNode(XN_NODE_TYPE_PLAYER, player);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // デバイスを作成する
        nRetVal = audio.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // WAVEデータを取得する
        nRetVal = audio.GetWaveOutputMode(waveMode);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // データの取得を開始する
        context.StartGeneratingAll();
    }

    // WAVEの初期化
    void initWave()
    {
        // WAVEデータの設定
        WAVEFORMATEX wf;
        wf.wFormatTag = 0x0001; // PCM
        wf.nChannels = waveMode.nChannels;
        wf.nSamplesPerSec = waveMode.nSampleRate;
        wf.wBitsPerSample = waveMode.nBitsPerSample;
        wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
        wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
        MMRESULT mmRes = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, NULL, NULL, CALLBACK_NULL);
        if (mmRes != MMSYSERR_NOERROR)
        {
            throw std::runtime_error( "Warning: Failed opening wave out device. Audio will not be played!\n" );
        }

        // 音声データ用のバッファの作成と初期化
        AudioBuffers.resize( NUMBER_OF_AUDIO_BUFFERS );
        xnOSMemSet(&AudioBuffers[0], 0, sizeof(WAVEHDR)*AudioBuffers.size());

        const XnUInt32 nMaxBufferSize = 2 * 1024 * 1024;
        for (int i = 0; i < NUMBER_OF_AUDIO_BUFFERS; ++i)
        {
            AudioBuffers[i].lpData = new XnChar[nMaxBufferSize];
            AudioBuffers[i].dwUser = i;
            AudioBuffers[i].dwFlags = WHDR_DONE; // mark this buffer as empty (already played)
        }
    }

    // メインループ
    void run()
    {
        int nAudioNextBuffer = 0;

        printf ("Press any key to exit...\n");

        // 今のデータを捨てる
        audio.WaitAndUpdateData();

        while (!xnOSWasKeyboardHit()) {
            // データの更新
            XnStatus nRetVal = context.WaitAndUpdateAll();
            if (nRetVal != XN_STATUS_OK) {
                throw std::runtime_error(xnGetStatusString(nRetVal));
            }

            // バッファの取得
            WAVEHDR* pHeader = &AudioBuffers[nAudioNextBuffer];
            if ((pHeader->dwFlags & WHDR_DONE) == 0) {
                printf("No audio buffer is available!. Audio buffer will be lost!\n");
                continue;
            }

            // WAVEヘッダのクリーンアップ
            MMRESULT mmRes = waveOutUnprepareHeader(hWaveOut, pHeader, sizeof(WAVEHDR));
            if ( mmRes != MMSYSERR_NOERROR ) {
                OutputErrorText( mmRes );
            }

            // WAVEデータの取得
            pHeader->dwBufferLength = audio.GetDataSize();
            pHeader->dwFlags = 0;
            xnOSMemCopy(pHeader->lpData, audio.GetAudioBuffer(), pHeader->dwBufferLength);

            // WAVEヘッダの初期化
            mmRes = waveOutPrepareHeader(hWaveOut, pHeader, sizeof(WAVEHDR));
            if ( mmRes != MMSYSERR_NOERROR ) {
                OutputErrorText( mmRes );
                continue;
            }

            // WAVEデータを出力キューに入れる
            mmRes = waveOutWrite(hWaveOut, pHeader, sizeof(WAVEHDR));
            if ( mmRes != MMSYSERR_NOERROR ) {
                OutputErrorText( mmRes );
                continue;
            }

            // 次のバッファインデックス
            nAudioNextBuffer = (nAudioNextBuffer + 1) % NUMBER_OF_AUDIO_BUFFERS;
        }
    }

private:

    // エラーメッセージの出力
    void OutputErrorText( MMRESULT mmRes )
    {
        CHAR msg[250];
        waveOutGetErrorText(mmRes, msg, 250);
        std::cout << msg << std::endl;
    }
};

void main( int argc, char* argv[] )
{
    try {
        AudioOutput audio;
        if ( argc == 1 ) {
            audio.initOpenNI();
        }
        else {
            audio.initOpenNI( argv[1] );
        }

        audio.initWave();
        audio.run();
    }
    catch ( std::exception& ex ) {
        std::cout << ex.what() << std::endl;
    }
}
