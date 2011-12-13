using System;
using OpenNI;
using Win32;
using System.Runtime.InteropServices;

namespace AudioBase2
{
    class Program
    {
        static void Main( string[] args )
        {
            try {
                ScriptNode node;
                Context context = Context.CreateFromXmlFile( "../../SamplesConfig.xml", out node );

                AudioGenerator audio = context.FindExistingNode( NodeType.Audio ) as AudioGenerator;

                Console.WriteLine( "SanpleRate    : " + audio.WaveOutputMode.SampleRate );
                Console.WriteLine( "Channels      : " + audio.WaveOutputMode.Channels );
                Console.WriteLine( "BitsPerSample : " + audio.WaveOutputMode.BitsPerSample );
                Console.WriteLine( "Press any key" );

                StreamingWavePlayer wavePlayer = new StreamingWavePlayer( audio.WaveOutputMode.SampleRate, audio.WaveOutputMode.BitsPerSample, audio.WaveOutputMode.Channels, 100 );

                while ( !Console.KeyAvailable ) {
                    context.WaitAndUpdateAll();

                    Console.WriteLine( "SanpleRate    : " + audio.AudioBufferPtr + ", DataSize : " + audio.DataSize );

                    wavePlayer.Output( audio.AudioBufferPtr, audio.DataSize );
                }
            }
            catch ( Exception ex ) {
                Console.WriteLine( ex.Message );
            }
        }
    }
}
