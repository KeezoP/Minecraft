#include "pch.h"
#include "Input.h"


Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialise(HWND window)
{
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(window);
	m_quitApp = false;

	m_GameInput.forward = false;
	m_GameInput.back = false;
	m_GameInput.right = false;
	m_GameInput.left = false;
	m_GameInput.strUp = false;
	m_GameInput.strDown = false;
	m_GameInput.toggleGlass = false;
	m_GameInput.toggleSand = false;
	m_GameInput.addAnt = false;
	m_GameInput.toggleEaten = false;
	m_GameInput.bloom = false;
	m_GameInput.monochrome = false;


}

void Input::Update()
{
	auto kb = m_keyboard->GetState();	//updates the basic keyboard state
	m_KeyboardTracker.Update(kb);		//updates the more feature filled state. Press / release etc. 
	auto mouse = m_mouse->GetState();   //updates the basic mouse state
	m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 

	if (kb.Escape)// check has escape been pressed.  if so, quit out. 
	{
		m_quitApp = true;
	}

	//A key
	if (kb.A || kb.Left)	m_GameInput.strLeft = true;
	else		m_GameInput.strLeft = false;

	//D key
	if (kb.D || kb.Right)	m_GameInput.strRight = true;
	else		m_GameInput.strRight = false;

	//W key
	if (kb.W || kb.Up)	m_GameInput.forward = true;
	else		m_GameInput.forward = false;

	//S key
	if (kb.S || kb.Down)	m_GameInput.back = true;
	else		m_GameInput.back = false;

	//Q key
	if (kb.Q)	m_GameInput.left = true;
	else		m_GameInput.left = false;

	//E key
	if (kb.E)	m_GameInput.right = true;
	else		m_GameInput.right = false;

	//U / space key
	if (kb.U || kb.Space)	m_GameInput.strUp = true;
	else		m_GameInput.strUp = false;

	//J / left shift key
	if (kb.J || kb.LeftShift)	m_GameInput.strDown = true;
	else		m_GameInput.strDown = false;

	//P key
	if (m_KeyboardTracker.pressed.P)	m_GameInput.pause = true;
	else		m_GameInput.pause = false;

	//R key
	if (m_KeyboardTracker.pressed.R)	m_GameInput.reset = true;
	else		m_GameInput.reset = false;


	// 1 || nm 1
	if (m_KeyboardTracker.pressed.NumPad1 || m_KeyboardTracker.pressed.D1)	m_GameInput.toggleGlass = true;
	else		m_GameInput.toggleGlass = false;

	// 2 || nm 2
	if (m_KeyboardTracker.pressed.NumPad2 || m_KeyboardTracker.pressed.D2)	m_GameInput.toggleSand = true;
	else		m_GameInput.toggleSand = false;

	// 3 || nm 3
	if (m_KeyboardTracker.pressed.NumPad3 || m_KeyboardTracker.pressed.D3)	m_GameInput.addAnt = true;
	else		m_GameInput.addAnt = false;

	// 4 || nm 4
	if (m_KeyboardTracker.pressed.NumPad4 || m_KeyboardTracker.pressed.D4)	m_GameInput.toggleEaten = true;
	else		m_GameInput.toggleEaten = false;

	// 5 || nm 5
	if (m_KeyboardTracker.pressed.NumPad5 || m_KeyboardTracker.pressed.D5)	m_GameInput.toggleHand = true;
	else		m_GameInput.toggleHand = false;

	// Enter 
	if (m_KeyboardTracker.pressed.Enter)	m_GameInput.placeBlock = true;
	else		m_GameInput.placeBlock = false;


	// Backspace
	if (m_KeyboardTracker.pressed.Back)	m_GameInput.deleteBlock = true;
	else		m_GameInput.deleteBlock = false;

	// blurred, n
	if (m_KeyboardTracker.pressed.N)	m_GameInput.monochrome = true;
	else		m_GameInput.monochrome = false;

	// bloom, m
	if (m_KeyboardTracker.pressed.M)	m_GameInput.bloom = true;
	else		m_GameInput.bloom = false;

}

bool Input::Quit()
{
	return m_quitApp;
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}
