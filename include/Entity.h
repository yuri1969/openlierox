/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Entity structure
// Created 23/7/02
// Jason Boettcher


#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "Event.h"

class CMap;
class CViewport;


#define		MAX_ENTITIES	1024


// Entity types
enum {
	ENT_PARTICLE=0,
	ENT_GIB,
	ENT_SPAWN,
	ENT_BLOOD,
	ENT_BLOODDROPPER,
	ENT_EXPLOSION,
	ENT_SMOKE,
	ENT_CHEMSMOKE,
	ENT_SPARKLE,
	ENT_DOOMSDAY,
	ENT_JETPACKSPRAY,
	ENT_BEAM,
	ENT_LASERSIGHT
};



class entity_t { public:
entity_t() : bUsed(false) {}
private:
	bool	bUsed;

public:
	int		iType;
	int		iType2;
	float	fLife;
	CVec	vPos;
	CVec	vVel;
	int		iAngle;
	Uint32	iColour;
	float	fAnglVel;
	int		iRotation;
	
	float	fFrame;
	TimeDiff	fExtra;
	SmartPointer<SDL_Surface> bmpSurf;

	bool isUsed() const	{ return bUsed; }
	void Spawn()		{ bUsed = true; }
	void setUnused()  { 
		bUsed = false;
		onInvalidation.occurred(EventData(this)); 
	}
	Event<> onInvalidation;

};

enum EntityEffectType {
	ENTE_NONE,
	ENTE_SPARKLE_DOT,
	ENTE_SPARKLE_RANDOM,
	ENTE_SPARKLE_SPREAD,
	ENTE_SPARKLE_CIRCLE,
	ENTE_SPARKLE_CIRCLE_ROTATING,
};

class EntityEffect
{
	public:
	EntityEffect()
	{
		Set();

		_lastTime = 0.0f;
		_lastAngle = 0.0f;
	}
	
	void Process( CVec pos, CVec vel );
	
	void Set( EntityEffectType type = ENTE_NONE, int amount = 3, float delay = 0.5f, float speed = 5.0f, float radius = 25.0f, int fade = 5 )
	{
		_type = type;
		_amount = amount;
		_delay = delay;
		_speed = speed;
		_radius = radius;
		_fade = fade;
	};

	EntityEffectType _type;
	int _amount;
	float _speed;
	float _delay;
	float _radius;
	int _fade;
	
	private:
	float _lastTime;
	float _lastAngle;
};

// Entity routines
int		InitializeEntities(void);
void	ShutdownEntities(void);
void	ClearEntities(void);

void	SpawnEntity(int type, int type2, CVec pos, CVec vel, Uint32 colour, SmartPointer<SDL_Surface> img);
void	DrawEntities(SDL_Surface * bmpDest, CViewport *v);
void	SimulateEntities(TimeDiff dt);
void	EntityBounce(entity_t *ent);

void	NewNet_SaveEntities();
void	NewNet_LoadEntities();

#endif  //  __ENTITY_H__
