using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using OpenNI;

namespace UserWPF
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        Context context;
        ImageGenerator image;
        UserGenerator user;

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

                // ユーザーの作成
                user = new UserGenerator(context);

                context.StartGeneratingAll();

                // 画像更新のためのスレッドを作成
                shouldRun = true;
                readerThread = new Thread( new ThreadStart( ReaderThread ) );
                readerThread.Start();
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
            }
        }

        private void ReaderThread()
        {
            while ( shouldRun ) {
                context.WaitAndUpdateAll();

                this.Dispatcher.BeginInvoke( DispatcherPriority.Background, new Action( Draw ) );
            }
        }

        // ユーザーにつける色
        static Color[] userColor= new Color[] {
            Color.FromRgb( 0, 0, 0 ),   // ユーザーなし
            Color.FromRgb( 1, 0, 0 ),
            Color.FromRgb( 0, 1, 0 ),
            Color.FromRgb( 0, 0, 1 ),
            Color.FromRgb( 1, 1, 0 ),
            Color.FromRgb( 1, 0, 1 ),
            Color.FromRgb( 0, 1, 1 ),
        };

        private void Draw()
        {
            // マネージドのバッファにする
            ImageMetaData imageMD = image.GetMetaData();
            byte[] rgb = new byte[imageMD.DataSize];
            Marshal.Copy( imageMD.ImageMapPtr, rgb, 0, imageMD.DataSize );

            // ユーザーを検出したピクセルの色を変える
            var users = user.GetUserPixels(0);
            for (int i = 0; i < imageMD.XRes * imageMD.YRes; i++)
            {
                if ( users[i] != 0 ) {
                    int rgbIndex = i * imageMD.BytesPerPixel;
                    rgb[rgbIndex] = (byte)(rgb[rgbIndex] * userColor[users[i]].R);
                    rgb[rgbIndex + 1] = (byte)(rgb[rgbIndex + 1] * userColor[users[i]].G);
                    rgb[rgbIndex + 2] = (byte)(rgb[rgbIndex + 2] * userColor[users[i]].B);
                }
            }

            // バイト列をビットマップに展開
            // 描画可能なビットマップを作る
            // http://msdn.microsoft.com/ja-jp/magazine/cc534995.aspx
            var bitmap = new WriteableBitmap(imageMD.XRes, imageMD.YRes, 96, 96, PixelFormats.Rgb24, null);
            bitmap.WritePixels(new Int32Rect(0, 0, imageMD.XRes, imageMD.YRes), rgb,
                                imageMD.XRes * imageMD.BytesPerPixel, 0 );

            image1.Source = bitmap;
        }

        private void Window_Closing( object sender, System.ComponentModel.CancelEventArgs e )
        {
            shouldRun = false;
        }
    }
}
