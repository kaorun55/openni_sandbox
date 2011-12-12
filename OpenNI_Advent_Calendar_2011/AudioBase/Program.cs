using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using OpenNI;

namespace AudioBase
{
    class Program
    {
        static void Main( string[] args )
        {
            try {
                // ContextとImageGeneratorの作成
                ScriptNode node;
                Context context = Context.CreateFromXmlFile( "../../SamplesConfig.xml", out node );

                AudioGenerator audio = context.FindExistingNode( NodeType.Audio ) as AudioGenerator;

                Console.WriteLine( "SanpleRate    : " + audio.WaveOutputMode.SampleRate );
                Console.WriteLine( "Channels      : " + audio.WaveOutputMode.Channels );
                Console.WriteLine( "BitsPerSample : " + audio.WaveOutputMode.BitsPerSample );
                Console.WriteLine( "Press any key" );

                while ( !Console.KeyAvailable ) {
                    context.WaitAndUpdateAll();

                    Console.WriteLine( "SanpleRate    : " + audio.AudioBufferPtr + ", DataSize : " + audio.DataSize );
                }
            }
            catch ( Exception ex ) {
                Console.WriteLine( ex.Message );
            }
        }
    }
}
