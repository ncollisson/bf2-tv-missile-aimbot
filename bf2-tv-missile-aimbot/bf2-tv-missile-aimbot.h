#pragma once
#include "bf2-player.h"
#include <chrono>
#include <list>

class TVMissileAimbot
{
public:
	int Initialize();
	int Run();

private:
	int SetRendDX9Base();
	Player* FindBestTarget();
	D3DXVECTOR3 CalculateXhairCoords(Player* target, bool prediction = false);
	D3DXVECTOR2 GetXhairCoords();
	int AdjustXhairCoords(D3DXVECTOR3);
	Player* GetPlayerByIndex(unsigned int);
	Player* GetMyPlayer();

	Player* best_target = NULL;
	unsigned int renddx9_base = NULL;

	typedef struct location_time
	{
		D3DXVECTOR3 coords;
		std::chrono::system_clock::time_point time;
	} location_time;

	std::list<location_time> previous_locations;
};

int StartHack();

typedef struct player_list_node pl_node;

typedef struct player_list_node
{
	pl_node* next;
	pl_node* prev;
	Player* player;
} pl_node;


