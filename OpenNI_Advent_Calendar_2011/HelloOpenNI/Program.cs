// see:http://arena.openni.org/OpenNIArena/Applications/ViewApp.aspx?app_id=424
using System;
using System.Reflection;
using NITE;
using OpenNI;

namespace HelloOpenNI
{
    class Program
    {
        static void Main( string[] args )
        {
            ScriptNode node;
            Context context = Context.CreateFromXmlFile( "../../SamplesConfig.xml", out node );

            SessionManager sessionManager = new SessionManager( context, "Wave", "RaiseHand" );
            sessionManager.SessionStart +=
                new EventHandler<PositionEventArgs>( sessionManager_SessionStart );

            WaveDetector wave = new WaveDetector();
            wave.Wave += new EventHandler( wave_Wave );
            sessionManager.AddListener( wave );

            Console.WriteLine( "Start gesture recognize." );

            while ( !Console.KeyAvailable ) {
                context.WaitAndUpdateAll();
                sessionManager.Update( context );
            }
        }

        static void sessionManager_SessionStart( object sender, PositionEventArgs e )
        {
            Console.WriteLine( MethodBase.GetCurrentMethod().Name );
        }

        static void wave_Wave( object sender, EventArgs e )
        {
            Console.WriteLine( MethodBase.GetCurrentMethod().Name + ": Hello OpenNI!!" );
        }
    }
}
