#include "pch.h"
#pragma once
class Ants
{
public:
	Ants(DirectX::SimpleMath::Vector4 cubePos, int cubeDimensions, int cubeID);
	~Ants();
	void GenerateTraits();
	float GetDigSpeed();
	DirectX::SimpleMath::Vector3 GetDigVector();
	DirectX::SimpleMath::Vector3 GetPosition();
	DirectX::SimpleMath::Vector3 GetCubePosition();
	void SetPosition(DirectX::SimpleMath::Vector3);
	void UpdateLifeSpan(float input);
	bool isDead();
	void generateNewVector();
	void stuck(float input);
	void resetStuck();
	bool isStuck();
	void updateErraticTimer(float input);
	int getCubeID();
	int getCubeType();


private:
	bool isErratic = false;
	DirectX::SimpleMath::Vector3 spawn;
	DirectX::SimpleMath::Vector3 currentPos;
	DirectX::SimpleMath::Vector3 DiggingVector;
	DirectX::SimpleMath::Vector3 CubePos;
	float digSpeed;
	float lifeSpan;
	int CubeSize;
	float stuckTimer = 0.0f;
	float erraticTimer = 0.0f;
	int cubeID;
	int cubeType;
};

