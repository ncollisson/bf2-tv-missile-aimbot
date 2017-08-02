#include "stdafx.h"
#include <d3dx9.h>
#include "bf2-player.h"

bool Player::isInfantry()
{
	int a = *(int*)(u1 + 0x80);
	if (a == NULL) return false;;

	int b = *(int*)((char*)a + 0x4);
	if (b == NULL) return false;

	return *(int*)b == 0x8E80F8;
}

int Player::GetVehicleClass()
{
	Object_1* a = object_1;
	if (a == nullptr) return 10;

	Physical_Object* b = a->physical_object;
	if (b == nullptr) return 10;

	Object_2* c = b->object_2;
	if (c == nullptr) return 10;

	return c->vehicleClass;
}

D3DXVECTOR3* Player::GetCoords()
{
	bool vehicle_position = true;
	
	if (vehicle_position)
	{
		int a = *(int*)(u1 + 0x80);
		if (a == NULL) return NULL;

		int b = *(int*)((char*)a + 0x4);
		if (b == NULL) return NULL;

		int c = *(int*)((char*)b + 0xC);
		if (c == NULL) return NULL;

		D3DXVECTOR3* d = (D3DXVECTOR3*)((char*)c + 0xB8);
		return d;
	}

	Object_1* a = object_1;
	if (a == NULL) return NULL;

	Physical_Object* b = a->physical_object;
	if (b == NULL) return NULL;

	D3DXVECTOR3* coords;
	coords = (D3DXVECTOR3*)&(b->coords);

	return coords;
}

D3DXVECTOR3* Player::GetVelocity()
{
	Object_1* a = object_1;
	if (a == NULL) return NULL;

	Physical_Object* b = a->physical_object;
	if (b == NULL) return NULL;

	Velocity_Object* c = b->velocity_object;
	if (c == NULL)
	{
		int d = *(int*)(u1 + 0xCC);
		if (d == NULL) return NULL;

		int e = *(int*)((char*)d + 0x4);
		if (e == NULL) return NULL;

		int f = *(int*)((char*)e + 0x44);
		if (f == NULL) return NULL;

		D3DXVECTOR3* passenger_velocity = (D3DXVECTOR3*)((char *)f + 0x78);
		return passenger_velocity;
	}

	D3DXVECTOR3* velocity;
	velocity = (D3DXVECTOR3*)&(c->velocity);

	return velocity;
}

int Player::GetTeam()
{
	return team;
}

unsigned char Player::isManDown()
{
	unsigned char isManDown = *(u1 + 0xE8);

	return isManDown;
}

unsigned char Player::isAlive()
{
	unsigned char isAlive = *(u1 + 0xD5);

	return isAlive;
}