using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using OpenNI;

namespace SkeletonFullWPF
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        Context context;
        ImageGenerator image;
        DepthGenerator depth;
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
                depth = context.FindExistingNode( NodeType.Depth ) as DepthGenerator;
                user = context.FindExistingNode( NodeType.User ) as UserGenerator;

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

                // ImageMetaDataをBitmapSourceに変換する(unsafeにしなくてもOK!!)
                this.Dispatcher.BeginInvoke( DispatcherPriority.Background, new Action( Draw ) );
            }
        }

        static Color[] userColor= new Color[] {
            Colors.Red, Colors.Blue, Colors.Green, Colors.Pink, Colors.Yellow,
            Colors.Gold, Colors.Silver
        };

        private void Draw()
        {
            ImageMetaData imageMD = image.GetMetaData();
            DepthMetaData depthMD = depth.GetMetaData();

            // 描画可能なビットマップを作る
            // http://stackoverflow.com/questions/831860/generate-bitmapsource-from-uielement
            RenderTargetBitmap bitmap =
                        new RenderTargetBitmap( depthMD.XRes, depthMD.YRes, 96, 96, PixelFormats.Default );

            DrawingVisual drawingVisual = new DrawingVisual();
            using ( DrawingContext drawingContext = drawingVisual.RenderOpen() ) {
                // 画像の描画
                drawingContext.DrawImage( DrawRgb( imageMD ), new Rect( 0, 0, depthMD.XRes, depthMD.YRes ) );

                // 骨格の描画
                var users = user.GetUsers();
                foreach ( var u in users ) {
                    if ( !user.SkeletonCapability.IsTracking( u ) ) {
                        continue;
                    }

                    foreach ( SkeletonJoint s in Enum.GetValues( typeof( SkeletonJoint ) ) ) {
                        if ( !user.SkeletonCapability.IsJointAvailable( s ) ) {
                            continue;
                        }
                        var joint = user.SkeletonCapability.GetSkeletonJoint( u, s );
                        var point = depth.ConvertRealWorldToProjective( joint.Position.Position );
                        drawingContext.DrawEllipse( new SolidColorBrush( Colors.Red ),
                            new Pen( Brushes.Red, 1 ), new Point( point.X, point.Y ), 5, 5 );
                    }
                }
            }

            bitmap.Render( drawingVisual );

            image1.Source = bitmap;
        }

        private WriteableBitmap DrawRgb( ImageMetaData imageMD )
        {
            // 描画可能なビットマップを作る
            // http://msdn.microsoft.com/ja-jp/magazine/cc534995.aspx
            var bitmap = new WriteableBitmap( imageMD.XRes, imageMD.YRes, 96, 96, PixelFormats.Rgb24, null );

            byte[] rgb = new byte[imageMD.DataSize];
            Marshal.Copy( imageMD.ImageMapPtr, rgb, 0, imageMD.DataSize );

            var users = user.GetUserPixels( 0 );

            for ( int i = 0; i < imageMD.XRes * imageMD.YRes; i++ ) {
                if ( users[i] != 0 ) {
                    int rgbIndex = i * imageMD.BytesPerPixel;
                    rgb[rgbIndex] = userColor[users[i]].R;
                    rgb[rgbIndex + 1] = userColor[users[i]].G;
                    rgb[rgbIndex + 2] = userColor[users[i]].B;
                }
            }

            bitmap.WritePixels( new Int32Rect( 0, 0, imageMD.XRes, imageMD.YRes ), rgb,
                                imageMD.XRes * imageMD.BytesPerPixel, 0 );
            return bitmap;
        }

        private void Window_Closing( object sender, System.ComponentModel.CancelEventArgs e )
        {
            shouldRun = false;
        }
    }
}
