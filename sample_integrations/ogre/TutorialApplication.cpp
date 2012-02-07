/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "TutorialApplication.h"

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
    // create your scene here :)
}

#include <trace_client/trace.h>
#include <OgreLogManager.h>

class LogRedir: public Ogre::LogListener
{
public:
    virtual void messageLogged (Ogre::String const & message, Ogre::LogMessageLevel lml, bool maskDebug, Ogre::String const & logName)
    {
		// forwards message to server
		TRACE_MSG(trace::e_Info, trace::CTX_Render, "log=%s lvl=%u msg=%s", logName.c_str(), lml, message.c_str());
    }
};


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
		TRACE_APPNAME("Ogre_App");
		TRACE_CONNECT();
		TRACE_MSG(trace::e_Info, trace::CTX_Default, "Initialized main application");

		// redirect Ogre log to our listener
        LogRedir redir;
        Ogre::LogManager * mLogManager = OGRE_NEW Ogre::LogManager();
        Ogre::Log * log = Ogre::LogManager::getSingleton().createLog("", true, false, false);
        if (log)
            log->addListener(&redir);

        // Create application object
        TutorialApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        if (log)
            log->removeListener(&redir);
        return 0;
    }

#ifdef __cplusplus
}
#endif
