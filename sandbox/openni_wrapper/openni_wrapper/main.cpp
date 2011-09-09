#include "App.h"

const char* CONFIG_XML_PATH = "SamplesConfig.xml";
const char* RECORDE_PATH = "record.oni";

int main (int argc, char * argv[])
{
    try {
        App app;
        if ( argc == 1 ) {
            app.InitFromXml( CONFIG_XML_PATH, RECORDE_PATH );
        }
        else {
            app.InitFromRecord( argv[1] );
        }

        app.Run();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
