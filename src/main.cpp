// OpenLieroX

// Main entry point
// Created 28/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "console.h"

#ifndef WIN32
#include <sys/dir.h>
#include <sys/stat.h>
#endif

// TODO: i hate globals ...
// we have to create a basic class CGame or something

CClient		*cClient = NULL;
CServer		*cServer = NULL;
lierox_t	*tLX = NULL;
game_t		tGameInfo;
CInput		cTakeScreenshot;
CInput		cSwitchMode;

int         nDisableSound = false;

keyboard_t	*kb;
SDL_Surface	*Screen;

CVec		vGravity = CVec(0,4);



///////////////////
// Main entry point
int main(int argc, char *argv[])
{
    int     startgame = false;
    float   fMaxFPS = 85;

	// this behavior only make sense for a win-system
	// under unix, the bin and the data are seperate
	#ifdef WIN32
		// Reset the current working directory (remove the filename first!!!)
		// Note: Windows give the exe path and name in the first parameter
		char *slashpos = strrchr(argv[0],'\\');
		*slashpos = 0;
		chdir(argv[0]);
	#else
		// TODO: set and handles the following search pathes:
		// . , ~/.OpenLieroX , /usr/share/OpenLieroX
		// for the moment, only the actual path will be used
		// we need a abstract resource system to handle this
		// Quake3 make it very well, should we copy it?
	#endif

	// Load options and other settings
	if(!LoadOptions())
		return -1;

	if	(!LoadNetworkStrings())
		return -1;

    // Parse the arguments
    ParseArguments(argc, argv);

	// Initialize LX
	if(!InitializeLieroX())
		return -1;
	kb = GetKeyboard();
	Screen = SDL_GetVideoSurface();

	// Initialize menu
	if(!Menu_Initialize(&startgame)) {
        SystemError("Error: Could not initialize the menu system.\nError when loading graphics files");
		ShutdownLieroX();
		return -1;
	}

	// TODO: abstract the logging, make an uniform message system
	// Log the game start
	if (tLXOptions->iLogConvos)  {
		FILE *f;

		f = fopen_i("Conversations.log","a");
		if (f)  {	
			char cTime[26];
			GetTime(cTime);
			fputs("<game starttime=\"",f);
			fputs(cTime,f);
			fputs("\">\r\n",f);
			fclose(f);
		}
	}

	// Setup the global keys
	cTakeScreenshot.Setup(tLXOptions->sGeneralControls[SIN_SCREENSHOTS]);
	cSwitchMode.Setup(tLXOptions->sGeneralControls[SIN_SWITCHMODE]);

	while(!tLX->iQuitGame) {

		startgame = false;

		// Start the menu
		Menu_Start();
		
		if(startgame) {
			// Start the game
			// this means, start the local server and connect to it
			StartGame();

		} else {
			// Quit
			ShutdownLieroX();
			return 0;
		}

		// Pre-game initialization
		Screen = SDL_GetVideoSurface();
		float oldtime = GetMilliSeconds();
        float captime = GetMilliSeconds();
		
		ClearEntities();

		ProcessEvents();
		tLX->iQuitEngine = false;
        fMaxFPS = 1.0f / (float)tLXOptions->nMaxFPS;

        //
        // Main game loop
        //
		while(!tLX->iQuitEngine) {

            tLX->fCurTime = GetMilliSeconds();

            // Cap the FPS
            if(tLX->fCurTime - captime < fMaxFPS)
                continue;
            else
                captime = tLX->fCurTime;

            ProcessEvents();

			// Timing
			tLX->fDeltaTime = tLX->fCurTime - oldtime;
			oldtime = tLX->fCurTime;

			// Main frame
			GameLoop();

			FlipScreen(Screen);
		}
	}


	ShutdownLieroX();

	return 0;
}


///////////////////
// Parse the arguments
void ParseArguments(int argc, char *argv[])
{
    // Parameters passed to liero xtreme overwrite the loaded options
    char *a;
    for(int i=1; i<argc; i++) {
        a = argv[i];

        // -nosound
        // Turns off the sound
        if( stricmp(a, "-nosound") == 0 ) {
            nDisableSound = true;
            tLXOptions->iSoundOn = false;
        }

        // -window
        // Turns fullscreen off
        if( stricmp(a, "-window") == 0 ) {
            tLXOptions->iFullscreen = false;
        }


        // -fullscreen
        // Turns fullscreen on
        if( stricmp(a, "-fullscreen") == 0 ) {
            tLXOptions->iFullscreen = true;
        }
    }
}


///////////////////
// Initialize the game
int InitializeLieroX(void)
{
        printf("Hello there, I am initializing me now...\n");

#ifndef WIN32
        struct stat s;
        char fname[256];
        GetExactFileName("data", fname);
        if(stat(fname, &s) != 0)
        {
                SystemError("ERROR: data-directory not found");
                return false;
        }
        if(!(s.st_mode & S_IFDIR))
        {
                SystemError("ERROR: very strange: data is no directory");
                return false;
        }
#else // WIN32
	// TODO: ...
#endif
	
	// Initialize the aux library
	if(!InitializeAuxLib("Liero Xtreme","config.cfg",16,0)) {
        SystemError("strange problems with the aux library");	
		return false;
	}

	// Initialize the network
    if(!InitNetworkSystem()) {
        SystemError("Error: Failed to initialize the network library");
		return false;
    }

	// Allocate the client & server
	cClient = new CClient;
    if(cClient == NULL) {
        SystemError("Error: InitializeLieroX() Out of memory");
		return false;
    }
	
	cClient->Clear();

	cServer = new CServer;
    if(cServer == NULL) {
        SystemError("Error: InitializeLieroX() Out of memory on creating CServer");
		return false;
    }

	// Initialize the entities
    if(!InitializeEntities()) {
        SystemError("Error: InitializeEntities() Out of memory on initializing the entities");
		return false;
    }


	// Initialize the LieroX structure
	tLX = new lierox_t;
    if(tLX == NULL) {
        SystemError("Error: InitializeLieroX() Out of memory on creating lierox_t");
		return false;
    }
	tLX->iQuitGame = false;
	tLX->debug_string[0] = 0;

	// Load the graphics
    if(!LoadGraphics()) {
        SystemError("Error: Error loading graphics");
		return false;
    }

    // Initialize the console
    if(!Con_Initialize()) {
        SystemError("Error: Could not initialize the console");
        return false;
    }

    // Add some console commands
    Cmd_AddCommand("kick", Cmd_Kick);
	Cmd_AddCommand("ban", Cmd_Ban);
	Cmd_AddCommand("mute", Cmd_Mute);
	Cmd_AddCommand("unmute", Cmd_Unmute);
    Cmd_AddCommand("kickid", Cmd_KickId);
	Cmd_AddCommand("banid", Cmd_BanId);
	Cmd_AddCommand("muteid", Cmd_MuteId);
	Cmd_AddCommand("unmuteid", Cmd_UnmuteId);
	Cmd_AddCommand("crash", Cmd_Crash);
	Cmd_AddCommand("suicide", Cmd_Suicide);
	Cmd_AddCommand("unstuck", Cmd_Unstuck);

	// Load the sounds
	LoadSounds();


	// Load the profiles
	LoadProfiles();

    // Initialize the game info structure
	tGameInfo.iNumPlayers = 0;
    tGameInfo.sMapRandom.psObjects = NULL;

	printf("Initializing ready\n");

	return true;
}


///////////////////
// Start the game
void StartGame(void)
{
    // Clear the screen
    DrawRectFill(SDL_GetVideoSurface(), 0,0, 640,480, 0);

	// Local game
	if(tGameInfo.iGameType == GME_LOCAL) {

		// TODO: uniform message system

		// Start the server
		if(!cServer->StartServer( "local", tLXOptions->iNetworkPort, 8, false )) {
			// ERROR
			//MessageBox(NULL, "Error: Could not start server", "Liero Xtreme Error", MB_OK);
			printf("Error: Could not start server\n");
			return;
		}

		// Setup the client
		if(!cClient->Initialize()) {
			// ERROR
			//MessageBox(NULL, "Error: Could not initialize client", "Liero Xtreme Error", MB_OK);
			printf("Error: Could not initialize client\n");
			return;
		}

		// Tell the client to connect to the server
		cClient->Connect("127.0.0.1");
	}
}


///////////////////
// Game loop
void GameLoop(void)
{
    if(tLX->iQuitEngine)
        return;

	// Local
	switch (tGameInfo.iGameType)  {
	case GME_LOCAL:
		cClient->Frame();
		cServer->Frame();

		// If we are connected, just start the game straight away (bypass lobby in local)
		if(cClient->getStatus() == NET_CONNECTED) {
			if(cServer->getState() == SVS_LOBBY)
				cServer->StartGame();
		}

		cClient->Draw(Screen);
		break;


	// Hosting
	case GME_HOST:
		cClient->Frame();
		cServer->Frame();

		cClient->Draw(Screen);
		break;

	// Joined
	case GME_JOIN:
		cClient->Frame();
		cClient->Draw(Screen);
		break;


	} // SWITCH

	// Switch between window and fullscreen mode
	if( cSwitchMode.isDown() )  {
		// Set to fullscreen
		tLXOptions->iFullscreen = !tLXOptions->iFullscreen;

		// Set the new video mode
		SetVideoMode();

		// Update both menu and game screens
		Screen = SDL_GetVideoSurface();
		tMenu->bmpScreen = SDL_GetVideoSurface();
	}

	// We put it here, so the mouse never displays
    SDL_ShowCursor(SDL_DISABLE);

}


///////////////////
// Quit back to the menu
void QuittoMenu(void)
{
	tLX->iQuitEngine = true;
    Menu_SetSkipStart(false);
	cClient->Disconnect();
}


///////////////////
// Shutdown the game
void ShutdownLieroX(void)
{
	printf("Shutting me down...\n");	
	
	if (tLXOptions->iLogConvos)  {
		FILE *f;

		f = fopen_i("Conversations.log","a");
		if (f)  { 
			fputs("</game>\r\n",f);
			fclose(f);
		}
	}

    Con_Shutdown();

	ShutdownGraphics();

	Menu_Shutdown();
	ShutdownProfiles();

    // Free the game info structure
    if(tGameInfo.sMapRandom.psObjects)
        delete[] tGameInfo.sMapRandom.psObjects;

    tGameInfo.sMapRandom.psObjects = NULL;

	// Free the client & server & gui parser
	if(cClient) {
		cClient->Shutdown();
		delete cClient;
		cClient = NULL;
	}

	if(cServer) {
		cServer->Shutdown();
		delete cServer;
		cServer = NULL;
	}


	ShutdownOptions();

	ShutdownEntities();
	
	if(tLX) {
		delete tLX;
		tLX = NULL;
	}

	QuitNetworkSystem();
	
	ShutdownAuxLib();

	printf("Everything was shut down\n");
	printf("Good Bye and enjoy your day...\n");
	
}
