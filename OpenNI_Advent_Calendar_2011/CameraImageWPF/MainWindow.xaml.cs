using System.Windows;
using System.Windows.Media;

using OpenNI;
using System.Threading;
using System.Windows.Media.Imaging;
using System;
using System.Windows.Threading;

namespace CameraImageWPF
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        Context context;
        ImageGenerator image;

        private Thread readerThread;
        private bool shouldRun;

        public MainWindow()
        {
            InitializeComponent();

            try {
                // ContextとImageGeneratorの作成
                ScriptNode node;
                context = Context.CreateFromXmlFile( "SamplesConfig.xml", out node );
                context.GlobalMirror = false;
                image = context.FindExistingNode( NodeType.Image ) as ImageGenerator;

                // 画像更新のためのスレッドを作成
                shouldRun = true;
                readerThread = new Thread( new ThreadStart( () =>
                {
                    while ( shouldRun ) {
                        context.WaitAndUpdateAll();
                        ImageMetaData imageMD = image.GetMetaData();

                        // ImageMetaDataをBitmapSourceに変換する(unsafeにしなくてもOK!!)
                        this.Dispatcher.BeginInvoke( DispatcherPriority.Background, new Action( () =>
                        {
                            image1.Source = BitmapSource.Create( imageMD.XRes, imageMD.YRes,
                                96, 96, PixelFormats.Bgr24, null, imageMD.ImageMapPtr,
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
