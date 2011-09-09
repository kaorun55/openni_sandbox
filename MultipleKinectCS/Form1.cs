using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using OpenNI;

namespace MultipleKinectCS
{
    public partial class Form1 : Form
    {
        private Bitmap bitmap;
        private Thread readerThread;
        private bool shouldRun;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load( object sender, EventArgs e )
        {
            try {
                // 初期化
                xnInitialize();

                // カメラサイズのイメージを作成(8bitのRGB)
                MapOutputMode mapMode = image.MapOutputMode;
                bitmap = new Bitmap( (int)mapMode.XRes, (int)mapMode.YRes,
                            System.Drawing.Imaging.PixelFormat.Format24bppRgb );

                // ウィンドウサイズをカメラサイズに合わせる
                ClientSize = new Size( (int)mapMode.XRes, (int)mapMode.YRes );

                // 画像更新のためのスレッドを作成
                //shouldRun = true;
                //readerThread = new Thread( ReaderThread );
                //readerThread.Start();
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
                Close();
            }
        }

        // 画像データ更新スレッド
        private void ReaderThread()
        {
            try {
                while ( shouldRun ) {
                    // 描画したら、画面を無効にして再描画する
                    xnDraw();
                    Invalidate();
                }
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
            }
        }

        // フォームが閉じられる
        private void Form1_FormClosing( object sender, FormClosingEventArgs e )
        {
            try {
                // スレッドの終了
                shouldRun = false;
                if ( readerThread != null ) {
                    readerThread.Join();
                }
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
            }
        }

        // 再描画
        private void Form1_Paint( object sender, PaintEventArgs e )
        {
            try {
                lock ( this ) {
                    if ( bitmap != null ) {
                        e.Graphics.DrawImage( bitmap, 0, 0 );
                    }
                }
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
            }
        }

        // このメソッドをオーバーライドすることでちらつきがなくなる
        protected override void OnPaintBackground( PaintEventArgs pevent )
        {
        }

        // キーイベント
        private void Form1_KeyDown( object sender, KeyEventArgs e )
        {
            // "Q"で終了。それ以外はアプリケーション固有の処理とする
            if ( e.KeyCode == Keys.Q ) {
                Close();
            }
            else {
                xnKeyDown( e.KeyCode );
            }
        }
    }
}
