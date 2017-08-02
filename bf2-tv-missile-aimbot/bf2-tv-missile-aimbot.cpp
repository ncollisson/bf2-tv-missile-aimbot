#include "stdafx.h"
#include <windows.h>
#include <Psapi.h>
#include <d3dx9.h>
#include <cmath>
#include <list>
#include <chrono>
#include "bf2-tv-missile-aimbot.h"
#include "bf2-player.h"

int StartHack()
{
	TVMissileAimbot* aimbot = new TVMissileAimbot();

	aimbot->Initialize();
	aimbot->Run();

	return 0;
}

int TVMissileAimbot::SetRendDX9Base()
{
	HANDLE h_process = GetCurrentProcess();
	HMODULE h_renddx9 = GetModuleHandle(L"RendDX9.dll");
	MODULEINFO modinfo;
	
	GetModuleInformation(h_process, h_renddx9, &modinfo, sizeof(modinfo));

	renddx9_base = (unsigned int)modinfo.lpBaseOfDll;

	return 1;
}

int TVMissileAimbot::Initialize()
{
	SetRendDX9Base();

	return 1;
}

int TVMissileAimbot::Run()
{
	D3DXVECTOR3 xhair_coords;

	while (true)
	{
		if (GetAsyncKeyState(VK_SPACE))
		{
 			if (best_target == NULL) best_target = FindBestTarget();
			if (best_target == NULL) continue;
			if (best_target->isInfantry() == true) continue;

			xhair_coords = CalculateXhairCoords(best_target, true);
			AdjustXhairCoords(xhair_coords);
		}
		else
		{
			best_target = NULL;
			if (previous_locations.empty() == false) previous_locations.clear();
		}

		Sleep(10);
	}

	return 0;
}

// returns pointer to player object for best target
// returns NULL on error/no good target found
Player* TVMissileAimbot::FindBestTarget()
{
	Player *potential_target = NULL,
		*best_target = NULL,
		*my_player = GetMyPlayer();
	D3DXVECTOR3 *my_coords = NULL, *p_target_coords = NULL,
		p_target_vector, xhair_coords;
	D3DXVECTOR2 current_xhair_coords;
	D3DXMATRIX* my_camera = (D3DXMATRIX *)(renddx9_base + 0x25e64c);
	float pi = 3.1415926535f, p_target_dist, xhair_dist,
		best_xhair_dist = 10000, delta_x, delta_y;

	if (my_player == NULL) return NULL;

	for (unsigned int i = 1; i <= 64; i++)
	{
		if (previous_locations.empty() == false) previous_locations.clear();

		potential_target = GetPlayerByIndex(i);
		if (potential_target == NULL) break;

		if (my_player->GetTeam() == potential_target->GetTeam()) continue;

		if (potential_target->isManDown() || !potential_target->isAlive()) continue;

		if (potential_target->isInfantry()) continue; 

		if (my_camera == NULL) return NULL;
		my_coords = (D3DXVECTOR3 *)&(my_camera->_41);
		if (my_coords == NULL) continue;

		p_target_coords = potential_target->GetCoords();
		if (p_target_coords == NULL) continue;

		p_target_vector = *p_target_coords - *my_coords;
		p_target_dist = D3DXVec3Length(&p_target_vector);

		if (p_target_dist > 450.0) continue;

		xhair_coords = CalculateXhairCoords(potential_target);
		if (xhair_coords.z < 0) continue;
		if (std::abs(xhair_coords.x) > 300) continue;
		if (std::abs(xhair_coords.y) > 250) continue;

		current_xhair_coords = GetXhairCoords();

		delta_x = xhair_coords.x - current_xhair_coords.x;
		delta_y = xhair_coords.y - current_xhair_coords.y;
		xhair_dist = std::sqrt(delta_x * delta_x + delta_y * delta_y);

		if (xhair_dist < best_xhair_dist)
		{
			best_xhair_dist = xhair_dist;
			best_target = potential_target;
		}
	}

	return best_target;
}

D3DXVECTOR3 TVMissileAimbot::CalculateXhairCoords(Player* target, bool prediction)
{
	D3DXVECTOR3 xhair_coords, relative_velocity;
	D3DXVECTOR3 *my_coords, *target_coords, predicted_coords,
		missile_velocity, *target_velocity,
		*camera_right, *camera_up, *camera_ahead,
		target_movement, target_vector, n_target_vector, lag_compensation;
	D3DXMATRIX* my_camera;
	const float missile_speed = 125;
	float target_distance, relative_speed, time_to_impact, amount_to_lead, target_speed;
	
	my_camera = (D3DXMATRIX *)(renddx9_base + 0x25e64c);
	camera_right = (D3DXVECTOR3 *)&(my_camera->_11);
	camera_up = (D3DXVECTOR3 *)&(my_camera->_21);
	camera_ahead = (D3DXVECTOR3 *)&(my_camera->_31);
	my_coords = (D3DXVECTOR3 *)&(my_camera->_41);

	target_coords = target->GetCoords();
	if (target_coords == NULL) return { 0.0f, 0.0f, 0.0f };

	// save coords with current time
	location_time current_location;
	current_location.time = std::chrono::system_clock::now();
	current_location.coords = *target_coords;

	previous_locations.push_front(current_location);

	// get ping
	int ping_val = 200;
	if (target->GetVehicleClass() == 3)
	{
		ping_val = 100;
	}
	else if (target->GetVehicleClass() == 2)
	{
		ping_val = 100;
	}

	std::chrono::milliseconds ping(ping_val);

	// go back through prev coords until current time - ping is closest to saved time
	std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point target_time = current_time - ping;

	std::chrono::duration<std::chrono::system_clock::rep, std::chrono::system_clock::period> delta_t, last_delta_t;
	last_delta_t = std::chrono::milliseconds(10000);

	std::chrono::system_clock::time_point previous_location_time;
	D3DXVECTOR3 lag_target_coords = *target_coords;

	for (auto previous_location : previous_locations)
	{
		previous_location_time = previous_location.time;
		delta_t = target_time - previous_location_time;

		if (std::abs(delta_t.count()) > std::abs(last_delta_t.count())) break;

		lag_target_coords = previous_location.coords;
		last_delta_t = delta_t;
	}

	target_coords = &lag_target_coords;

	if (prediction && (target_velocity = target->GetVelocity()) != NULL)
	{
		D3DXVec3Scale(&missile_velocity, camera_ahead, missile_speed);

		relative_velocity = missile_velocity - *target_velocity;
		relative_speed = D3DXVec3Length(&relative_velocity);

		target_vector = *target_coords - *my_coords;
		target_distance = D3DXVec3Length(&target_vector);

		if (std::abs(relative_speed) < 0.1) relative_speed = 0.1;
		time_to_impact = target_distance / relative_speed;

		D3DXVECTOR3 n_target_velocity, n_missile_velocity;

		target_speed = D3DXVec3Length(target_velocity);

		D3DXVec3Normalize(&n_target_velocity, target_velocity);
		D3DXVec3Normalize(&n_missile_velocity, &missile_velocity);

		/*float relative_direction = D3DXVec3Dot(&n_target_velocity, &n_missile_velocity);

		relative_direction *= -1;
		relative_direction += 1;
		relative_direction /= 2;

		amount_to_lead = target_speed * relative_direction / 40;*/
		
		// simple, works fairly well
		/*amount_to_lead = target_speed / 100;

   		if (amount_to_lead > 0.8) amount_to_lead = 0.8;
		if (amount_to_lead < 0.4) amount_to_lead = 0.4;*/

		amount_to_lead = 0.2;

		//if (target->GetVehicleClass() == 3)
		//{
			//D3DXVec3Scale(&lag_compensation, target_velocity, -0.0075);
			//D3DXVec3Add(&predicted_coords, &lag_compensation, &predicted_coords);

			if (time_to_impact < 1.0) amount_to_lead += (1.0 - amount_to_lead) * (1.0 - time_to_impact);
			if (amount_to_lead > 0.9) amount_to_lead = 0.9;
		//}

		D3DXVec3Scale(&target_movement, target_velocity, time_to_impact * amount_to_lead);
		D3DXVec3Add(&predicted_coords, &target_movement, target_coords);

		target_coords = &predicted_coords;
	}

	target_vector = *target_coords - *my_coords;
	D3DXVec3Normalize(&n_target_vector, &target_vector);

	xhair_coords.x = D3DXVec3Dot(camera_right, &n_target_vector) * 1000;
	xhair_coords.y = D3DXVec3Dot(camera_up, &n_target_vector) * -1000;
	xhair_coords.z = D3DXVec3Dot(camera_ahead, &n_target_vector);

	return xhair_coords;
}

D3DXVECTOR2 TVMissileAimbot::GetXhairCoords()
{
	D3DXVECTOR2 current_xhair_coords;

	unsigned int anchor_2 = 0xA10890;

	unsigned int object_2 = *(unsigned int *)anchor_2;
	if (object_2 == NULL) return { 0.0f, 0.0f };

	unsigned int object_3 = *(unsigned int *)(object_2 + 0xf4);
	if (object_3 == NULL) return { 0.0f, 0.0f };

	current_xhair_coords = *(D3DXVECTOR2 *)(object_3 + 0x38);

	return current_xhair_coords;
}

int TVMissileAimbot::AdjustXhairCoords(D3DXVECTOR3 coords)
{
	D3DXVECTOR2* xhair_coords;

	unsigned int anchor_2 = 0xA10890;

	unsigned int object_2 = *(unsigned int *)anchor_2;
	if (object_2 == NULL) return 0;

	unsigned int object_3 = *(unsigned int *)(object_2 + 0xf4);
	if (object_3 == NULL) return 0;

	xhair_coords = (D3DXVECTOR2 *)(object_3 + 0x38); 

	if (coords.x > 226) coords.x = 226;
	if (coords.x < -226) coords.x = -226;
	if (coords.y > 166) coords.y = 166;
	if (coords.y < -166) coords.y = -166;

	xhair_coords->x = coords.x;
	xhair_coords->y = coords.y;

	return 1;
}

Player* TVMissileAimbot::GetPlayerByIndex(unsigned int index)
{
	unsigned int anchor_1 = 0xA08F60;

	unsigned int object_1 = *(unsigned int *)anchor_1;
	if (object_1 == NULL) return NULL;

	unsigned int dbl_linked_player_list = *(unsigned int *)(object_1 + 0x10);
	if (dbl_linked_player_list == NULL) return NULL;

	Player* player = NULL;
	pl_node* current_node = (pl_node *)dbl_linked_player_list;
	// do i need to deref here?

	for (unsigned int i = 0; i != index; i++)
	{
		if (i != 0 && current_node->player == NULL) return NULL;
		current_node = current_node->next;
	}

	return current_node->player;
}

Player* TVMissileAimbot::GetMyPlayer()
{
	return GetPlayerByIndex(1);
}