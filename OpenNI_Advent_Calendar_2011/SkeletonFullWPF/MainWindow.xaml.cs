using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using OpenNI;
using System.Diagnostics;

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
                context.GlobalMirror = true;
                image = context.FindExistingNode( NodeType.Image ) as ImageGenerator;
                depth = context.FindExistingNode( NodeType.Depth ) as DepthGenerator;
                depth.AlternativeViewpointCapability.SetViewpoint( image );

                // ユーザーの作成
                user = context.FindExistingNode( NodeType.User ) as UserGenerator;

                // ユーザー認識のコールバックを登録
                user.NewUser += new EventHandler<NewUserEventArgs>( user_NewUser );

                //キャリブレーションにポーズが必要か確認
                if ( user.SkeletonCapability.DoesNeedPoseForCalibration ) {
                    // ポーズ検出のサポートチェック
                    if ( !user.IsCapabilitySupported( "User::PoseDetection" ) ) {
                        throw new Exception( "ポーズ検出をサポートしていません" );
                    }

                    // ポーズ検出のコールバックを登録
                    user.PoseDetectionCapability.PoseDetected +=
                        new EventHandler<PoseDetectedEventArgs>( poseDetect_PoseDetected );
                }

                // スケルトン検出機能をサポートしているか確認
                if ( !user.IsCapabilitySupported( "User::Skeleton" ) ) {
                    throw new Exception( "ユーザー検出をサポートしていません" );
                }

                // キャリブレーションのコールバックを登録
                user.SkeletonCapability.CalibrationEnd +=
                    new EventHandler<CalibrationEndEventArgs>( skelton_CalibrationEnd );

                // すべてをトラッキングする
                user.SkeletonCapability.SetSkeletonProfile( SkeletonProfile.HeadAndHands );

                // ジェスチャーの検出開始
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

        // ユーザーの検出通知
        void user_NewUser( object sender, NewUserEventArgs e )
        {
            // キャリブレーションポーズが必要な場合は、ポーズの検出を開始する
            if ( user.SkeletonCapability.DoesNeedPoseForCalibration ) {
                user.PoseDetectionCapability.StartPoseDetection( user.SkeletonCapability.CalibrationPose, e.ID );
            }
            // キャリブレーションポーズが不要な場合は、キャリブレーションを開始する
            else {
                user.SkeletonCapability.RequestCalibration( e.ID, true );
            }
        }

        // ポーズの検出通知
        void poseDetect_PoseDetected( object sender, PoseDetectedEventArgs e )
        {
            // ポーズの検出を停止し、キャリブレーションを開始する
            user.PoseDetectionCapability.StopPoseDetection( e.ID );
            user.SkeletonCapability.RequestCalibration( e.ID, true );
        }

        // キャリブレーションの完了
        void skelton_CalibrationEnd( object sender, CalibrationEndEventArgs e )
        {
            // キャリブレーション成功
            if ( e.Success ) {
                user.SkeletonCapability.StartTracking( e.ID );
            }
            else {
                user.PoseDetectionCapability.StartPoseDetection( user.SkeletonCapability.CalibrationPose, e.ID );
            }
        }

        private void ReaderThread()
        {
            while ( shouldRun ) {
                context.WaitAndUpdateAll();

                this.Dispatcher.BeginInvoke( DispatcherPriority.Input, new Action( Draw ) );
            }
        }

        // ユーザーにつける色
        static Color[] userColor = new Color[] {
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
            ImageMetaData imageMD = image.GetMetaData();

            DrawingVisual drawingVisual = new DrawingVisual();
            using ( DrawingContext drawingContext = drawingVisual.RenderOpen() ) {
                drawingContext.DrawImage( DrawPixels( imageMD ), new Rect( 0, 0, imageMD.XRes, imageMD.YRes ) );

                // 骨格の描画
                var users = user.GetUsers();
                foreach ( var u in users ) {
                    if ( !user.SkeletonCapability.IsTracking( u ) ) {
                        continue;
                    }

                    foreach ( SkeletonJoint s in Enum.GetValues( typeof( SkeletonJoint ) ) ) {
                        if ( !user.SkeletonCapability.IsJointActive( s ) ||
                             !user.SkeletonCapability.IsJointAvailable( s ) ) {
                            continue;
                        }

                        var joint = user.SkeletonCapability.GetSkeletonJoint( u, s );
                        if ( joint.Orientation.Confidence <= 0.5 ) {
                            continue;
                        }

                        var point = depth.ConvertRealWorldToProjective( joint.Position.Position );
                        drawingContext.DrawEllipse( new SolidColorBrush( Colors.Red ),
                            new Pen( Brushes.Red, 1 ), new Point( point.X, point.Y ), 5, 5 );
                    }
                }
            }

            // 描画可能なビットマップを作る
            // http://stackoverflow.com/questions/831860/generate-bitmapsource-from-uielement
            RenderTargetBitmap bitmap =
                    new RenderTargetBitmap( imageMD.XRes, imageMD.YRes, 96, 96, PixelFormats.Default );
            bitmap.Render( drawingVisual );

            image1.Source = bitmap;
        }

        private WriteableBitmap DrawPixels( ImageMetaData imageMD )
        {
            // マネージドのバッファにする
            byte[] rgb = new byte[imageMD.DataSize];
            Marshal.Copy( imageMD.ImageMapPtr, rgb, 0, imageMD.DataSize );

            // ユーザーを検出したピクセルの色を変える
            SceneMetaData users = user.GetUserPixels( 0 );
            for ( int i = 0; i < imageMD.XRes * imageMD.YRes; i++ ) {
                if ( users[i] != 0 ) {
                    int rgbIndex = i * imageMD.BytesPerPixel;
                    rgb[rgbIndex] = (byte)(rgb[rgbIndex] * userColor[users[i]].R);
                    rgb[rgbIndex + 1] = (byte)(rgb[rgbIndex + 1] * userColor[users[i]].G);
                    rgb[rgbIndex + 2] = (byte)(rgb[rgbIndex + 2] * userColor[users[i]].B);
                }
            }

             //バイト列をビットマップに展開
             //描画可能なビットマップを作る
             //http://msdn.microsoft.com/ja-jp/magazine/cc534995.aspx
            WriteableBitmap bitmap = new WriteableBitmap( imageMD.XRes, imageMD.YRes, 96, 96,
                                                          PixelFormats.Rgb24, null );
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
