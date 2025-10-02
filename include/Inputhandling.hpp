#pragma once
#include <SDL.h>
#include <Input.hpp>
#include <Emulator.hpp>
#include <unordered_map>
#include <QKeyEvent>


class InputHandler
{
public:
	InputHandler();
	~InputHandler();
	void update(const std::unordered_map<int, bool>& keyStates);
	ggb::GameboyInput getCurrentState() const;

private:
	void handleControllerConnectAndDisconnect();
	void updateKeyboardInput(const std::unordered_map<int, bool>& keyStates);
	void updateControllerInput();
	bool controllerButtonPressed(SDL_GameControllerButton button);

	ggb::GameboyInput m_inputState = {};
	const Uint8* m_keyStates = SDL_GetKeyboardState(nullptr);
	SDL_GameController* m_controller = nullptr;
	int m_controllerID = 0;
};