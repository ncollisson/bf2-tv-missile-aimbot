#pragma once
#include <d3dx9.h>

class Velocity_Object
{
public:
	char u1[0x20];
	D3DXVECTOR3 velocity;
};

class Object_2
{
public:
	char u1[0x350];
	int vehicleClass;
};

class Physical_Object
{
public:
	int u3[5];
	Object_2* object_2;
	int u1[11];
	Velocity_Object* velocity_object;
	char u2[0xb8 - 0x44 - sizeof(Velocity_Object *)];
	D3DXVECTOR3 coords;
};

class Object_1
{
public:
	char u1[0x4];
	Physical_Object* physical_object;
};

class Player
{
public:
	D3DXVECTOR3* GetCoords();
	D3DXVECTOR3* GetVelocity();
	int GetTeam();
	unsigned char isManDown();
	unsigned char isAlive();
	bool isInfantry();
	int GetVehicleClass();

private:

	char u1[0x80];
	Object_1* object_1;
	char u2[0x54];
	int team;
};