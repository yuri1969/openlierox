/////////////////////////////////////////
//
//         LieroX Game Script Compiler
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Main compiler
// Created 7/2/02
// Jason Boettcher


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CVec.h"
#include "CGameScript.h"
#include "ConfigHandler.h"
#include "SmartPointer.h"
#include "CrashHandler.h"
#include "Sounds.h"
#include "WeaponDesc.h"
#include "ProjectileDesc.h"

#include <cassert>
#include <setjmp.h>
#include <sstream> // for print_binary_string
#include <set>
#include <string>

#include "LieroX.h"
#include "IpToCountryDB.h"
#include "AuxLib.h"
#include "CClient.h"
#include "CServer.h"
#include "ConfigHandler.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "Entity.h"
#include "Error.h"
#include "DedicatedControl.h"
#include "Physics.h"
#include "Version.h"
#include "OLXG15.h"
#include "CrashHandler.h"
#include "Cursor.h"
#include "CssParser.h"
#include "FontHandling.h"
#include "Timer.h"
#include "CChannel.h"
#include "Cache.h"
#include "ProfileSystem.h"
#include "IRC.h"
#include "Music.h"
#include "Debug.h"
#include "TaskManager.h"
#include "CGameMode.h"
#include "ConversationLogger.h"
#include "StaticAssert.h"
#include "Command.h"

#include "DeprecatedGUI/CBar.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "DeprecatedGUI/CChatWidget.h"

#include "SkinnedGUI/CGuiSkin.h"

CGameScript	*Game;



int		Decompile();
int		DecompileWeapon(int id);
void	DecompileBeam(FILE * fp, const weapon_t *Weap);
void	DecompileProjectile(const proj_t * proj, const char * weaponName);
int		DecompileExtra(FILE * fp);

int		DecompileJetpack(FILE * fp, const weapon_t *Weap);

int ProjCount = 0;

const char * DisclaimerHeader = 
"# This file was generated by LieroX game script decompiler\n"
"# Please don't steal other people work, if you want to make modification\n"
"# of this game script, ASK AUTHOR PERMISSION FIRST! And don't claim you're the author.\n\n"
;


///////////////////
// Main entry point
int main(int argc, char *argv[])
{
	printf("Liero Xtreme Game Script Decompiler v0.3\nCopyright OLX team 2009\n");
	printf("GameScript Version: %d\n\n\n",GS_VERSION);


	Game = new CGameScript;
	if(Game == NULL) {
		printf("Error: Out of memory\n");
		return 1;
	}

	// Add some keywords
	AddKeyword("WPN_PROJECTILE",WPN_PROJECTILE);
	AddKeyword("WPN_SPECIAL",WPN_SPECIAL);
	AddKeyword("WPN_BEAM",WPN_BEAM);
	AddKeyword("WCL_AUTOMATIC",WCL_AUTOMATIC);
	AddKeyword("WCL_POWERGUN",WCL_POWERGUN);
	AddKeyword("WCL_GRENADE",WCL_GRENADE);
	AddKeyword("WCL_CLOSERANGE",WCL_CLOSERANGE);
	AddKeyword("PRJ_PIXEL",PRJ_PIXEL);
	AddKeyword("PRJ_IMAGE",PRJ_IMAGE);
	AddKeyword("Bounce",PJ_BOUNCE);
	AddKeyword("Explode",PJ_EXPLODE);
	AddKeyword("Injure",PJ_INJURE);
	AddKeyword("Carve",PJ_CARVE);
	AddKeyword("Dirt",PJ_DIRT);
    AddKeyword("GreenDirt",PJ_GREENDIRT);
    AddKeyword("Disappear",PJ_DISAPPEAR);
	AddKeyword("Nothing",PJ_NOTHING);
	AddKeyword("TRL_NONE",TRL_NONE);
	AddKeyword("TRL_SMOKE",TRL_SMOKE);
	AddKeyword("TRL_CHEMSMOKE",TRL_CHEMSMOKE);
	AddKeyword("TRL_PROJECTILE",TRL_PROJECTILE);
	AddKeyword("TRL_DOOMSDAY",TRL_DOOMSDAY);
	AddKeyword("TRL_EXPLOSIVE",TRL_EXPLOSIVE);
	AddKeyword("SPC_JETPACK",SPC_JETPACK);
	AddKeyword("ANI_ONCE",ANI_ONCE);
	AddKeyword("ANI_LOOP",ANI_LOOP);
	AddKeyword("ANI_PINGPONG",ANI_PINGPONG);
	AddKeyword("true",true);
	AddKeyword("false",false);


	const char * dir = ".";
	if( argc > 1 )
		dir = argv[1];

	InitBaseSearchPaths();
	
	if( Game->Load(dir) != GSE_OK )
	{
		printf("\nError loading mod!\n");
		return 1;
	}
	printf("\nInfo:\nWeapons: %d\n",Game->GetNumWeapons());

	Decompile();

	printf("\nDone!\nInfo:\nWeapons: %d Projectiles: %d\n",Game->GetNumWeapons(), ProjCount );
	

	//Game->Shutdown();
	if(Game)
		delete Game;

	return 0;
}




int Decompile()
{
	char buf[64],wpn[64],weap[32];
	int num,n;


	// Check the file
	FILE *fp = fopen("Main.txt", "w");
	if(!fp)
		return false;
	fprintf(fp, "%s", DisclaimerHeader);


	fprintf(fp, "[General]\n\nModName = %s\n",Game->GetHeader()->ModName);

	fprintf(fp, "\n[Weapons]\n\nNumWeapons = %i\n",Game->GetNumWeapons());


	weapon_t *weapons;

	// Compile the weapons
	for(n=0;n<Game->GetNumWeapons();n++) {
		fprintf(fp,"Weapon%d = w_%s.txt\n", n+1, Game->GetWeapons()[n].Name.c_str() );
	}

	// Compile the extra stuff
	DecompileExtra(fp);

	fclose(fp);

	for(n=0;n<Game->GetNumWeapons();n++) {
		DecompileWeapon(n);
	}

	return true;
}


///////////////////
// Compile a weapon
int DecompileWeapon(int id)
{
	const weapon_t *Weap = Game->GetWeapons()+id;
	
	char fname[128];
	sprintf(fname, "w_%s.txt", Weap->Name.c_str());
	
	FILE * fp = fopen(fname, "w");
	if(!fp)
		return false;
	fprintf(fp, "%s", DisclaimerHeader);

	printf("Compiling Weapon '%s'\n",Weap->Name.c_str());
	fprintf(fp,"[General]\n\nName = %s\n",Weap->Name.c_str());
	fprintf(fp,"Type = %s\n", (
				Weap->Type == WPN_PROJECTILE ? "WPN_PROJECTILE" : (
				Weap->Type == WPN_SPECIAL ? "WPN_SPECIAL" : (
				Weap->Type == WPN_BEAM ? "WPN_BEAM" : "Unknown" ) ) ) );


	// Special Weapons
	if(Weap->Type == WPN_SPECIAL) {
		
		fprintf(fp,"Special = %s\n", (
					Weap->Special == SPC_JETPACK ? "SPC_JETPACK" : "Unknown" ) );

		// If it is a special weapon, read the values depending on the special weapon
		// We don't bother with the 'normal' values
		switch(Weap->Special) {
			// Jetpack
			case SPC_JETPACK:
				DecompileJetpack(fp, Weap);
				break;

			default:
				printf("   Error: Unknown special type\n");
		}
		return true;
	}


	// Beam Weapons
	if(Weap->Type == WPN_BEAM) {

		DecompileBeam(fp,Weap);
		return true;
	}


	// Projectile Weapons
	fprintf( fp, "Recoil = %i\n",Weap->Recoil);
	fprintf( fp, "Drain = %f\n", Weap->Drain);
	fprintf( fp, "Recharge = %f\n", Weap->Recharge * 10.0f);
	fprintf( fp, "ROF = %f\n",Weap->ROF * 1000.0f);
	if( Weap->UseSound )
		fprintf( fp, "Sound = %s\n",Weap->SndFilename.c_str());
	fprintf( fp, "LaserSight = %s\n", (Weap->LaserSight ? "true":"false"));
	fprintf( fp, "Class = %s\n", (
				Weap->Class == WCL_AUTOMATIC ? "WCL_AUTOMATIC" : (
				Weap->Class == WCL_POWERGUN ? "WCL_POWERGUN" : (
				Weap->Class == WCL_GRENADE ? "WCL_GRENADE" : (
				Weap->Class == WCL_CLOSERANGE ? "WCL_CLOSERANGE" : "Unknown" )))));

	fprintf( fp,"\n[Projectile]\n\nSpeed = %i\n",Weap->Proj.Speed);
	fprintf( fp, "SpeedVar = %f\n", Weap->Proj.SpeedVar);
	fprintf( fp, "Spread = %f\n", Weap->Proj.Spread);
	fprintf( fp, "Amount = %i\n", Weap->Proj.Amount);
	fprintf( fp, "Projectile = p_%s_%08x.txt\n", Weap->Name.c_str(), (int)Weap->Proj.Proj);
	
	fclose(fp);
	
	DecompileProjectile( Weap->Proj.Proj, Weap->Name.c_str() );

	return true;
}


///////////////////
// Compile a beam weapon
void DecompileBeam(FILE * fp, const weapon_t *Weap)
{
	fprintf( fp,"\n[General]\n\nRecoil = %i\n",Weap->Recoil);
	fprintf( fp, "Drain = %f\n", Weap->Drain);
	fprintf( fp, "Recharge = %f\n", Weap->Recharge * 10.0f);
	fprintf( fp, "ROF = %f\n",Weap->ROF * 1000.0f);
	if( Weap->UseSound )
		fprintf( fp, "Sound = %s\n",Weap->SndFilename.c_str());

	fprintf( fp, "\n[Beam]\n\nDamage = %i\n", Weap->Bm.Damage);
	fprintf( fp, "Length = %i\n", Weap->Bm.Length);
	fprintf( fp, "PlayerDamage = %i\n", Weap->Bm.PlyDamage);
	fprintf( fp, "Colour = %u,%u,%u\n", Weap->Bm.Colour.r, Weap->Bm.Colour.g, Weap->Bm.Colour.b);

}


///////////////////
// Compile a projectile
void DecompileProjectile(const proj_t * proj, const char * weaponName)
{
	char fname[128];

	sprintf(fname, "p_%s_%08x.txt", weaponName, (int)proj);
	
	FILE * fp = fopen(fname, "w");
	if(!fp)
		return;
	fprintf(fp, "%s", DisclaimerHeader);

	ProjCount++;

	printf("    Decompiling Projectile '%s'\n",fname);
	
	fprintf(fp,"[General]\n\nType = %s\n", (
				proj->Type == PRJ_PIXEL ? "PRJ_PIXEL" : (
				proj->Type == PRJ_IMAGE ? "PRJ_IMAGE" : "Unknown" )));

	fprintf( fp, "Timer = %f\n", proj->Timer.Time);
	fprintf( fp, "TimerVar = %f\n", proj->Timer.TimeVar);
	fprintf( fp, "Trail = %s\n", (
				proj->Trail.Type == TRL_NONE ? "TRL_NONE" : ( 
				proj->Trail.Type == TRL_SMOKE ? "TRL_SMOKE" : ( 
				proj->Trail.Type == TRL_CHEMSMOKE ? "TRL_CHEMSMOKE" : ( 
				proj->Trail.Type == TRL_PROJECTILE ? "TRL_PROJECTILE" : ( 
				proj->Trail.Type == TRL_DOOMSDAY ? "TRL_DOOMSDAY" : ( 
				proj->Trail.Type == TRL_EXPLOSIVE ? "TRL_EXPLOSIVE" : " Unknown" 
				)))))));
	

	if( proj->UseCustomGravity )
		fprintf( fp, "Gravity = %i\n", proj->Gravity);
	
	fprintf( fp, "Dampening = %f\n", proj->Dampening);

	if(proj->Type == PRJ_PIXEL) {

		fprintf( fp, "Colour1 = %u,%u,%u\n", proj->Colour[0].r, proj->Colour[0].g, proj->Colour[0].b);

		if( proj->Colour.size() >= 2 )
			fprintf( fp, "Colour2 = %u,%u,%u\n", proj->Colour[1].r, proj->Colour[1].g, proj->Colour[1].b);

	} else if(proj->Type == PRJ_IMAGE) {
		fprintf( fp, "Image = %s\n", proj->ImgFilename.c_str());
		fprintf( fp, "Rotating = %s\n", ( proj->Rotating ? "true":"false" ));
		fprintf( fp, "RotIncrement = %i\n", proj->RotIncrement);
		fprintf( fp, "RotSpeed = %i\n", proj->RotSpeed);
		fprintf( fp, "UseAngle = %s\n", ( proj->UseAngle ? "true":"false" ));
		fprintf( fp, "UseSpecAngle = %s\n", ( proj->UseSpecAngle ? "true":"false" ));
		if(proj->UseAngle || proj->UseSpecAngle)
			fprintf( fp, "AngleImages = %i\n", proj->AngleImages);
	
		fprintf( fp, "Animating = %s\n", ( proj->Animating ? "true":"false" ));

		if(proj->Animating) {
			fprintf( fp, "AnimRate = %f\n", proj->AnimRate);
			fprintf( fp, "AnimType = %s\n", (
						proj->AnimType == ANI_ONCE ? "ANI_ONCE" : (
						proj->AnimType == ANI_LOOP ? "ANI_LOOP" : (
						proj->AnimType == ANI_PINGPONG ? "ANI_PINGPONG" : "Unknown" ))));
		}
	}
	

	fprintf( fp, "\n[Hit]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->Hit.Type == PJ_BOUNCE ? "Bounce" : (
				proj->Hit.Type == PJ_EXPLODE ? "Explode" : (
				proj->Hit.Type == PJ_INJURE ? "Injure" : (
				proj->Hit.Type == PJ_CARVE ? "Carve" : (
				proj->Hit.Type == PJ_DIRT ? "Dirt" : (
				proj->Hit.Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Hit.Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Hit.Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));


	// Hit::Explode
	if(proj->Hit.Type == PJ_EXPLODE) {

		fprintf( fp, "Damage = %i\n", proj->Hit.Damage);
		fprintf( fp, "Projectiles = %s\n", ( proj->Hit.Projectiles ? "true":"false" ));
		fprintf( fp, "Shake = %i\n", proj->Hit.Shake);
		
		if( proj->Hit.UseSound )
			fprintf( fp, "Sound = %s\n", proj->Hit.SndFilename.c_str());
	}

	// Hit::Carve
	if(proj->Hit.Type == PJ_CARVE) {
		fprintf( fp, "Damage = %i\n", proj->Hit.Damage);
	}

	// Hit::Bounce
	if(proj->Hit.Type == PJ_BOUNCE) {
		fprintf( fp, "BounceCoeff = %f\n", proj->Hit.BounceCoeff);
		fprintf( fp, "BounceExplode = %i\n", proj->Hit.BounceExplode);
	}

	// Timer
	if(proj->Timer.Time > 0) {
	
		fprintf( fp, "\n[Time]\n" );
		fprintf( fp, "Type = %s\n", (
				proj->Timer.Type == PJ_BOUNCE ? "Bounce" : (
				proj->Timer.Type == PJ_EXPLODE ? "Explode" : (
				proj->Timer.Type == PJ_INJURE ? "Injure" : (
				proj->Timer.Type == PJ_CARVE ? "Carve" : (
				proj->Timer.Type == PJ_DIRT ? "Dirt" : (
				proj->Timer.Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Timer.Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Timer.Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));
		
		//if(proj->Timer_Type == PJ_EXPLODE) {

			fprintf( fp, "Damage = %i\n", proj->Timer.Damage);
			fprintf( fp, "Projectiles = %s\n", ( proj->Timer.Projectiles ? "true":"false" ));
			fprintf( fp, "Shake = %i\n", proj->Timer.Shake);
		//}
	}

	// Player hit
	fprintf( fp, "\n[PlayerHit]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->PlyHit.Type == PJ_BOUNCE ? "Bounce" : (
				proj->PlyHit.Type == PJ_EXPLODE ? "Explode" : (
				proj->PlyHit.Type == PJ_INJURE ? "Injure" : (
				proj->PlyHit.Type == PJ_CARVE ? "Carve" : (
				proj->PlyHit.Type == PJ_DIRT ? "Dirt" : (
				proj->PlyHit.Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->PlyHit.Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->PlyHit.Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));


	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit.Type == PJ_EXPLODE || proj->PlyHit.Type == PJ_INJURE) {
		fprintf( fp, "Damage = %i\n", proj->PlyHit.Damage);
		fprintf( fp, "Projectiles = %s\n", ( proj->PlyHit.Projectiles ? "true":"false" ));
	}

	// PlyHit::Bounce
	if(proj->PlyHit.Type == PJ_BOUNCE) {
		fprintf( fp, "BounceCoeff = %f\n", proj->PlyHit.BounceCoeff);
	}


    // OnExplode
	fprintf( fp, "\n[Explode]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->Exp.Type == PJ_BOUNCE ? "Bounce" : (
				proj->Exp.Type == PJ_EXPLODE ? "Explode" : (
				proj->Exp.Type == PJ_INJURE ? "Injure" : (
				proj->Exp.Type == PJ_CARVE ? "Carve" : (
				proj->Exp.Type == PJ_DIRT ? "Dirt" : (
				proj->Exp.Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Exp.Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Exp.Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));

	fprintf( fp, "Damage = %i\n", proj->Exp.Damage);
	fprintf( fp, "Projectiles = %s\n", ( proj->Exp.Projectiles ? "true":"false" ));
	fprintf( fp, "Shake = %i\n", proj->Exp.Shake);
	if( proj->Exp.UseSound )
		fprintf( fp, "Sound = %s\n", proj->Exp.SndFilename.c_str());


    // Touch
	fprintf( fp, "\n[Touch]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->Tch.Type == PJ_BOUNCE ? "Bounce" : (
				proj->Tch.Type == PJ_EXPLODE ? "Explode" : (
				proj->Tch.Type == PJ_INJURE ? "Injure" : (
				proj->Tch.Type == PJ_CARVE ? "Carve" : (
				proj->Tch.Type == PJ_DIRT ? "Dirt" : (
				proj->Tch.Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Tch.Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Tch.Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));

	fprintf( fp, "Damage = %i\n", proj->Tch.Damage);
	fprintf( fp, "Projectiles = %s\n", ( proj->Tch.Projectiles ? "true":"false" ));
	fprintf( fp, "Shake = %i\n", proj->Tch.Shake);
	if( proj->Tch.UseSound )
		fprintf( fp, "Sound = %s\n", proj->Tch.SndFilename.c_str());


	// Projectiles - in LX56 there's only one projectile type for all actions
	const Proj_SpawnInfo * childProj = &proj->GeneralSpawnInfo;
	/*
	// Not used in LX56
	if(proj->Timer.Projectiles)
		childProj = &proj->Timer.Proj;
	if(proj->Hit.Projectiles)
		childProj = &proj->Hit.Proj;
	if(proj->PlyHit.Projectiles)
		childProj = &proj->PlyHit.Proj;
	if(proj->Exp.Projectiles)
		childProj = &proj->Exp.Proj;
	if(proj->Tch.Projectiles)
		childProj = &proj->Tch.Proj;
	*/
	
	if(proj->Timer.Projectiles || proj->Hit.Projectiles || proj->PlyHit.Projectiles || proj->Exp.Projectiles || proj->Tch.Projectiles)
	{
	
		fprintf( fp, "\n[Projectile]\n" );
		fprintf( fp, "Projectile = p_%s_%08x.txt\n", weaponName, (int)childProj->Proj);
		fprintf( fp, "Amount = %i\n", childProj->Amount);
		fprintf( fp, "Speed = %i\n", childProj->Speed);
		fprintf( fp, "SpeedVar = %f\n", childProj->SpeedVar);
		fprintf( fp, "Spread = %f\n", childProj->Spread);
		fprintf( fp, "Useangle = %s\n", ( childProj->Useangle ? "true":"false" ));
		fprintf( fp, "Angle = %i\n", childProj->Angle);

		DecompileProjectile(childProj->Proj, weaponName);
	}


	// Projectile trail
	if(proj->Trail.Type == TRL_PROJECTILE) {

		fprintf( fp, "\n[ProjectileTrail]\n" );
		fprintf( fp, "Projectile = p_%s_%08x.txt\n", weaponName, (int)proj->Trail.Proj.Proj);
		fprintf( fp, "Delay = %f\n", proj->Trail.Delay * 1000.0f);
		fprintf( fp, "UseProjVelocity = %s\n", ( proj->Trail.Proj.UseParentVelocityForSpread ? "true":"false" ));
		fprintf( fp, "Amount = %i\n", proj->Trail.Proj.Amount);
		fprintf( fp, "Speed = %i\n", proj->Trail.Proj.Speed);
		fprintf( fp, "SpeedVar = %f\n", proj->Trail.Proj.SpeedVar);
		fprintf( fp, "Spread = %f\n", proj->Trail.Proj.Spread);

		DecompileProjectile(proj->Trail.Proj.Proj, weaponName);
	}
}


///////////////////
// Compile the extra stuff
int DecompileExtra(FILE * fp)
{
	char file[64];

	int ropel, restl;
	float strength;

	fprintf(fp,"\n[NinjaRope]\n\nRopeLength = %i\n", Game->getRopeLength());
	fprintf(fp, "RestLength = %i\n", Game->getRestLength() );
	fprintf(fp, "Strength = %f\n",Game->getStrength());

	const gs_worm_t *wrm = Game->getWorm();
	
	fprintf(fp,"\n[Worm]\n\nAngleSpeed = %f\n", wrm->AngleSpeed );
	fprintf(fp, "GroundSpeed = %f\n", wrm->GroundSpeed );
	fprintf(fp, "AirSpeed = %f\n", wrm->AirSpeed );
	fprintf(fp, "Gravity = %f\n", wrm->Gravity );
	fprintf(fp, "JumpForce = %f\n", wrm->JumpForce );
	fprintf(fp, "AirFriction = %f\n", wrm->AirFriction );
	fprintf(fp, "GroundFriction = %f\n", wrm->GroundFriction );

	return true;
}


/*
===============================

		Special items

===============================
*/


///////////////////
// Compile the jetpack
int DecompileJetpack(FILE * fp, const weapon_t *Weap)
{

	fprintf( fp, "[JetPack]\n\nThrust = %i\n", Weap->tSpecial.Thrust);
	fprintf( fp, "Drain = %f\n", Weap->Drain);
	fprintf( fp, "Recharge = %f\n", Weap->Recharge * 10.0f);

	return true;
}


// some dummies/stubs are following to be able to compile with OLX sources
ConversationLogger *convoLogger = NULL;
GameState currentGameState() { return S_INACTIVE; }
lierox_t	*tLX = NULL;
IpToCountryDB *tIpToCountryDB = NULL;
bool        bDisableSound = true;
bool		bDedicated = true;
bool		bJoystickSupport = false;
bool		bRestartGameAfterQuit = false;
keyboard_t	*kb = NULL;
void GotoLocalMenu(){};
void GotoNetMenu(){};
void QuittoMenu(){};
void doActionInMainThread(Action* act) {};
void doVideoFrameInMainThread() {};
void doSetVideoModeInMainThread() {};
void doVppOperation(Action* act) {};
FileListCacheIntf* modList = NULL;
FileListCacheIntf* skinList = NULL;
FileListCacheIntf* mapList = NULL;
TStartFunction startFunction = NULL;
void* startFunctionData = NULL;



void ResetQuitEngineFlag() {};
void SetQuitEngineFlag(const std::string& reason) { };
bool Warning_QuitEngineFlagSet(const std::string& preText) { };
#ifndef WIN32
sigjmp_buf longJumpBuffer;
#endif
void ShutdownLieroX() {} ;
void updateFileListCaches() {};
void SetCrashHandlerReturnPoint(const char* name) { };



#if 0

FILE* OpenGameFile(const std::string& file, const char* mod) {
	// stub
	return fopen(file.c_str(), mod);
}

bool GetExactFileName(const std::string& fn, std::string& exactfn) {
	// sub
	exactfn = fn;
	return true;
}

bool IsFileAvailable(const std::string& f, bool absolute) {
	// stub
	FILE * ff = fopen(f.c_str(), "r");
	if( !ff )
		return false;
	fclose(ff);
	return true;
}


//struct SoundSample;
template <> void SmartPointer_ObjectDeinit<SoundSample> ( SoundSample * obj )
{
	//errors << "SmartPointer_ObjectDeinit SoundSample: stub" << endl;
}

template <> void SmartPointer_ObjectDeinit<SDL_Surface> ( SDL_Surface * obj )
{
	//errors << "SmartPointer_ObjectDeinit SDL_Surface: stub" << endl;
}

SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying) {
	// stub
	return new SoundSample; // It will never be played, we just have to return non-NULL here
}

SmartPointer<SDL_Surface> LoadGameImage(const std::string& _filename, bool withalpha) {
	// stub
	return NULL;
}

void SetColorKey(SDL_Surface * dst) {} // stub

bool bDedicated = true;

void SetError(const std::string& text) { errors << "SetError: " << text << endl; }


bool Con_IsInited() { return false; };

CrashHandler* CrashHandler::get() {	return NULL; };

void Con_AddText(int colour, const std::string& text, bool alsoToLogger) {}

SDL_PixelFormat defaultFallbackFormat =
{
 NULL, //SDL_Palette *palette;
 32, //Uint8  BitsPerPixel;
 4, //Uint8  BytesPerPixel;
 0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
 24, 16, 8, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
 0xff000000, 0xff0000, 0xff00, 0xff, //Uint32 Rmask, Gmask, Bmask, Amask;
 0, //Uint32 colorkey;
 255 //Uint8  alpha;
};

SDL_PixelFormat* mainPixelFormat = &defaultFallbackFormat;

#endif