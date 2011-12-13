using System.Windows;
using System.Windows.Media;

using OpenNI;
using System.Threading;
using System.Windows.Media.Imaging;
using System;
using System.Windows.Threading;
using Win32;

namespace AudioWPF
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        Context context;
        ImageGenerator image;
        AudioGenerator audio;

        StreamingWavePlayer wavePlayer;

        private Thread readerThread;
        private bool shouldRun;

        public MainWindow()
        {
            InitializeComponent();

            try {
                // ContextとImageGeneratorの作成
                ScriptNode node;
                context = Context.CreateFromXmlFile( "../../SamplesConfig.xml", out node );
                context.GlobalMirror = false;
                image = context.FindExistingNode( NodeType.Image ) as ImageGenerator;
                audio = context.FindExistingNode( NodeType.Audio ) as AudioGenerator;

                wavePlayer = new StreamingWavePlayer( audio.WaveOutputMode.SampleRate,
                    audio.WaveOutputMode.BitsPerSample, audio.WaveOutputMode.Channels, 100 );

                // 画像更新のためのスレッドを作成
                shouldRun = true;
                readerThread = new Thread( new ThreadStart( () =>
                {
                    while ( shouldRun ) {
                        context.WaitAndUpdateAll();
                        ImageMetaData imageMD = image.GetMetaData();

                        // WAVEデータの出力
                        wavePlayer.Output( audio.AudioBufferPtr, audio.DataSize );

                        // ImageMetaDataをBitmapSourceに変換する(unsafeにしなくてもOK!!)
                        this.Dispatcher.BeginInvoke( DispatcherPriority.Background, new Action( () =>
                        {
                            image1.Source = BitmapSource.Create( imageMD.XRes, imageMD.YRes,
                                96, 96, PixelFormats.Rgb24, null, imageMD.ImageMapPtr,
                                imageMD.DataSize, imageMD.XRes * imageMD.BytesPerPixel );
                        } ) );
                    }
                } ) );
                readerThread.Start();
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
            }
        }

        private void Window_Closing( object sender, System.ComponentModel.CancelEventArgs e )
        {
            shouldRun = false;
        }
    }
}
