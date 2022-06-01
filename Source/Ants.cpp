#include "pch.h"
#include "Ants.h"
#include <algorithm>
Ants::Ants(DirectX::SimpleMath::Vector4 input, int cubeDimensions, int CubeID)
{
	CubePos = DirectX::SimpleMath::Vector3(input.x, input.y, input.z);
	CubeSize = cubeDimensions;
	cubeID = CubeID;
	cubeType = input.w;
	GenerateTraits();

}

Ants::~Ants()
{
}

void Ants::GenerateTraits()
{
	// generates all traits for ants

	// generate dig speed
	digSpeed = float((rand() % 5 + 5));
	if (cubeType != 5)
		digSpeed /= 10;
	// coinflip, is ant erratic?
	if (rand() % 2 == 1) {
		isErratic = true;
	}

	// generate lifespan of ant (seconds) 30-> 180
	lifeSpan = rand() % 151 + 30;

	// generates dig angle, XYZ rotation angles, range 0 ->9, * 36
	generateNewVector();

	// generate which face to spawn on
	int faceID = rand() % 6 + 1;
	switch (faceID) {
	case 1:
		// TOP 
		spawn = DirectX::SimpleMath::Vector3(rand() % (CubeSize - 2) + 1, CubeSize - 2, rand() % (CubeSize - 2) + 1);
		break;
	case 2:
		// BOTTOM
		spawn = DirectX::SimpleMath::Vector3(rand() % (CubeSize - 2) + 1, 1, rand() % (CubeSize - 2) + 1);
		break;
	case 3:
		// LEFT 
		spawn = DirectX::SimpleMath::Vector3(1, rand() % (CubeSize - 2) + 1, rand() % (CubeSize - 2) + 1);
		break;
	case 4:
		// RIGHT 
		spawn = DirectX::SimpleMath::Vector3(CubeSize - 2, rand() % (CubeSize - 2) + 1, rand() % (CubeSize - 2) + 1);
		break;
	case 5:
		// FRONT 
		spawn = DirectX::SimpleMath::Vector3(rand() % (CubeSize - 2) + 1, rand() % (CubeSize - 2) + 1, 1);
		break;
	case 6:
		// BACK
		spawn = DirectX::SimpleMath::Vector3(rand() % (CubeSize - 2) + 1, rand() % (CubeSize - 2) + 1, CubeSize - 2);
		break;
	}
	spawn += CubePos;
	currentPos = spawn;
}

float Ants::GetDigSpeed()
{
	return digSpeed;
}

DirectX::SimpleMath::Vector3 Ants::GetDigVector()
{
	return DiggingVector;
}


DirectX::SimpleMath::Vector3 Ants::GetPosition()
{
	return currentPos;
}

DirectX::SimpleMath::Vector3 Ants::GetCubePosition()
{
	return CubePos;
}

void Ants::SetPosition(DirectX::SimpleMath::Vector3 input)
{

	// c++ 17 -> float x = std::clamp(input.x, 1, 9);
	currentPos = input;
	bool collision = false;
	// keep in bounds of cube
	if (input.x < 1.0f)
	{
		input.x = 1.0f;
		collision = true;
	}
	else if (input.x > float(CubeSize - 2))
	{
		input.x = float(CubeSize - 2);
		collision = true;
	}

	if (input.y < 2.0f)
	{
		input.y = 2.0f;
		collision = true;
	}

	else if (input.y > float(CubeSize - 1))
	{
		input.y = float(CubeSize - 1);
		collision = true;
	}

	if (input.z < 1.0f)
	{
		input.z = 1.0f;
		collision = true;
	}

	else if (input.z > float(CubeSize - 2))
	{
		input.z = float(CubeSize - 2);
		collision = true;
	}

	// if ant has collided with edge
	if (collision == true) {
		currentPos = input;

		// calc new angle
		generateNewVector();
	}

}

void Ants::UpdateLifeSpan(float input)
{
	if (cubeType != 5)
		lifeSpan -= input;

	if (isErratic)
		updateErraticTimer(input);
}

bool Ants::isDead()
{
	if (lifeSpan > 0.0f)
		return false;
	else
		return true;
}

void Ants::generateNewVector() {

	float x = float(rand() % 4 - 2);
	float y = float(rand() % 4 - 2);
	float z = float(rand() % 4 - 2);



	DiggingVector = DirectX::SimpleMath::Vector3(x, y, z);
}

void Ants::stuck(float input)
{
	stuckTimer += input;

	if (stuckTimer > 2.0f)
		lifeSpan = -1.0f;
	else if (stuckTimer > 0.1f)
		generateNewVector();
}

void Ants::resetStuck() {
	stuckTimer = 0.0f;
}

bool Ants::isStuck()
{
	if (stuckTimer > 0.0f)
		return true;
	else
		return false;

}

void Ants::updateErraticTimer(float input)
{
	if (cubeType != 5) {
		erraticTimer += input;
		if (erraticTimer > 5.0f) {
			generateNewVector();
			erraticTimer = 0.0f;
		}
	}
	else {
		erraticTimer += input;
		if (erraticTimer > 1.0f) {
			generateNewVector();
			erraticTimer = 0.0f;
		}
	}
}

int Ants::getCubeID()
{
	return cubeID;
}

int Ants::getCubeType()
{
	return cubeType;
}

