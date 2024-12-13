#pragma once
#include <SDL.h>
#include <Input.hpp>
#include <Emulator.hpp>
#include <unordered_map>
#include <QKeyEvent>


class InputHandler : public ggb::Input
{
public:
	InputHandler();
	~InputHandler();
	bool isAPressed() override;
	bool isBPressed() override;
	bool isStartPressed() override;
	bool isSelectPressed() override;
	bool isUpPressed() override;
	bool isDownPressed() override;
	bool isLeftPressed() override;
	bool isRightPressed() override;
	void update(const std::unordered_map<int, bool>& keyStates);

private:
	void handleControllerConnectAndDisconnect();
	void updateKeyboardInput(const std::unordered_map<int, bool>& keyStates);
	void updateControllerInput();
	bool controllerButtonPressed(SDL_GameControllerButton button);
	struct InputState
	{
		bool aButtonDown = false;
		bool bButtonDown = false;
		bool startButtonDown = false;
		bool selectButtonDown = false;
		bool upButtonDown = false;
		bool downButtonDown = false;
		bool leftButtonDown = false;
		bool rightButtonDown = false;
	};

	InputState m_inputState = {};
	const Uint8* m_keyStates = SDL_GetKeyboardState(nullptr);
	SDL_GameController* m_controller = nullptr;
	int m_controllerID = 0;
};