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
        DepthGenerator depth;
        UserGenerator user;

        private Thread readerThread;
        private bool shouldRun;

        public MainWindow()
        {
            InitializeComponent();

            try {
                // OpenNIの初期化
                ScriptNode node;
                context = Context.CreateFromXmlFile( "../../SamplesConfig.xml", out node );
                context.GlobalMirror = false;

                // depthの作成
                depth = context.FindExistingNode( NodeType.Depth ) as DepthGenerator;

                // ユーザーの作成
                user = new UserGenerator( context );

                // ポーズが必要な場合は、最新版を入れてもらう
                if ( user.SkeletonCapability.DoesNeedPoseForCalibration ) {
                    throw new Exception( "最新のOpenNIをインストールしてください" );
                }

                // ユーザー検出、キャリブレーション完了のイベントを登録する
                user.NewUser += new EventHandler<NewUserEventArgs>( user_NewUser );
                user.SkeletonCapability.CalibrationComplete += new EventHandler<CalibrationProgressEventArgs>( SkeletonCapability_CalibrationComplete );

                // すべての骨格を追跡する
                user.SkeletonCapability.SetSkeletonProfile( SkeletonProfile.All );

                // 動作を開始する
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

        void SkeletonCapability_CalibrationComplete( object sender, CalibrationProgressEventArgs e )
        {
            // キャリブレーション成功
            if ( e.Status == CalibrationStatus.OK ) {
                Trace.WriteLine( "Calibration Success:" + e.ID );
                user.SkeletonCapability.StartTracking( e.ID );
            }
            // キャリブレーション失敗
            else {
                Trace.WriteLine( "Calibration Failed:" + e.ID );
            }
        }

        void user_NewUser( object sender, NewUserEventArgs e )
        {
            Trace.WriteLine( "New User:" + e.ID );
            user.SkeletonCapability.RequestCalibration( e.ID, true );
        }

        private void Window_Closing( object sender, System.ComponentModel.CancelEventArgs e )
        {
            shouldRun = false;
        }

        private void ReaderThread()
        {
            while ( shouldRun ) {
                context.WaitAndUpdateAll();
                DepthMetaData depthMD = depth.GetMetaData();

                this.Dispatcher.BeginInvoke( DispatcherPriority.Background, new Action( () =>
                {
                    // アンマネージド配列をマネージド配列に変換する
                    Int16[] depthArray = new Int16[depthMD.XRes * depthMD.YRes];
                    Marshal.Copy( depthMD.DepthMapPtr, depthArray, 0, depthArray.Length );
                    for ( int i = 0; i < depthArray.Length; i++ ) {
                        depthArray[i] = (Int16)(0xffff - (0xffff * depthArray[i] / depth.DeviceMaxDepth));
                    }

                    // 描画可能なビットマップを作る
                    // http://stackoverflow.com/questions/831860/generate-bitmapsource-from-uielement
                    RenderTargetBitmap bitmap =
                        new RenderTargetBitmap( depthMD.XRes, depthMD.YRes, 96, 96, PixelFormats.Default );

                    DrawingVisual drawingVisual = new DrawingVisual();
                    using ( DrawingContext drawingContext = drawingVisual.RenderOpen() ) {
                        // グレースケールの描画
                        var xtion = BitmapSource.Create( depthMD.XRes, depthMD.YRes,
                                                        96, 96, PixelFormats.Gray16, null, depthArray,
                                                        depthMD.XRes * depthMD.BytesPerPixel );
                        drawingContext.DrawImage( xtion, new Rect( 0, 0, depthMD.XRes, depthMD.YRes ) );

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
                } ) );
            }
        }
    }
}
