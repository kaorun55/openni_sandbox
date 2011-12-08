using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Threading;
using OpenNI;
using System.Windows.Threading;
using System.Runtime.InteropServices;

namespace DepthWPF
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        Context context;
        DepthGenerator depth;

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
                depth = context.FindExistingNode( NodeType.Depth ) as DepthGenerator;

                // 画像更新のためのスレッドを作成
                shouldRun = true;
                readerThread = new Thread( new ThreadStart( () =>
                {
                    while ( shouldRun ) {
                        context.WaitAndUpdateAll();
                        DepthMetaData depthMD = depth.GetMetaData();

                        // ImageMetaDataをBitmapSourceに変換する(unsafeにしなくてもOK!!)
                        this.Dispatcher.BeginInvoke( DispatcherPriority.Background, new Action( () =>
                        {
                            Int16[] depthArray = new Int16[depthMD.XRes * depthMD.YRes];
                            Marshal.Copy( depthMD.DepthMapPtr, depthArray, 0, depthArray.Length );
                            for  ( int i = 0; i < depthArray.Length; i++ ) {
                                depthArray[i] = (Int16)(0xffff - (0xffff * depthArray[i] / depth.DeviceMaxDepth));
                            }

                            image1.Source = BitmapSource.Create( depthMD.XRes, depthMD.YRes,
                                96, 96, PixelFormats.Gray16, null, depthArray,
                                depthMD.XRes * depthMD.BytesPerPixel );
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
