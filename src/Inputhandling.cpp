#include "Inputhandling.hpp"

#include <iostream>

InputHandler::InputHandler()
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
	{
		fprintf(stderr, "Error initializing controller SDL_Error: %s\n", SDL_GetError());
		return;
	}
}

InputHandler::~InputHandler()
{
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

bool InputHandler::isAPressed()
{
	return m_inputState.aButtonDown;
}

bool InputHandler::isBPressed()
{
	return m_inputState.bButtonDown;
}

bool InputHandler::isStartPressed()
{
	return m_inputState.startButtonDown;
}

bool InputHandler::isSelectPressed()
{
	return m_inputState.selectButtonDown;
}

bool InputHandler::isUpPressed()
{
	return m_inputState.upButtonDown;
}

bool InputHandler::isDownPressed()
{
	return m_inputState.downButtonDown;
}

bool InputHandler::isLeftPressed()
{
	return m_inputState.leftButtonDown;
}

bool InputHandler::isRightPressed()
{
	return m_inputState.rightButtonDown;
}

void InputHandler::update(const std::unordered_map<int, bool>& keyStates)
{
	m_inputState = {};
	updateKeyboardInput(keyStates);
	updateControllerInput();
}

void InputHandler::handleControllerConnectAndDisconnect()
{
	auto connectToFirstController = [this]()
	{
		if (m_controller) 
		{
			SDL_GameControllerClose(m_controller);
			m_controller = nullptr;
			m_controllerID = INT_MAX;
		}

		for (int i = 0; i < SDL_NumJoysticks(); i++)
		{
			if (SDL_IsGameController(i))
			{
				m_controller = SDL_GameControllerOpen(i);
				m_controllerID = i;
			}
		}
	};

	SDL_Event sdlEvent = {};
	// SDL_PollEvent also pumps the QT message queue which is ... unfortunate
	// but shouldn't be a problem, since we are on a new thread,
	// even though SDL_PollEvent should be called on the main thread only
	while (SDL_PollEvent(&sdlEvent))
	{
		if (sdlEvent.type == SDL_CONTROLLERDEVICEADDED)
		{
			if (!m_controller)
				connectToFirstController();
		}
		else if (sdlEvent.type == SDL_CONTROLLERDEVICEREMOVED)
		{
			if (m_controllerID == sdlEvent.cdevice.which)
				connectToFirstController();
		}
	}
}

void InputHandler::updateKeyboardInput(const std::unordered_map<int, bool>& keyStates)
{
	auto buttonPressed = [&keyStates](int key) -> bool
	{
		if (keyStates.count(key))
			return keyStates.at(key);
		return false;
	};

	m_inputState.aButtonDown |= buttonPressed(Qt::Key::Key_O);
	m_inputState.bButtonDown |= buttonPressed(Qt::Key::Key_P);
	m_inputState.startButtonDown |= buttonPressed(Qt::Key::Key_Space);
	m_inputState.selectButtonDown |= buttonPressed(Qt::Key::Key_Return);
	m_inputState.upButtonDown |= buttonPressed(Qt::Key::Key_W);
	m_inputState.downButtonDown |= buttonPressed(Qt::Key::Key_S);
	m_inputState.leftButtonDown |= buttonPressed(Qt::Key::Key_A);
	m_inputState.rightButtonDown |= buttonPressed(Qt::Key::Key_D);
}

void InputHandler::updateControllerInput()
{
	handleControllerConnectAndDisconnect();

	constexpr int joyStickThreshold = 20000;
	auto xValue = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_LEFTX);
	auto yValue = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_LEFTY);
	const bool joyStickLeft = xValue < -joyStickThreshold;
	const bool joyStickRight = xValue > joyStickThreshold;
	const bool joyStickDown = yValue > joyStickThreshold;
	const bool joyStickUp = yValue < -joyStickThreshold;

	m_inputState.aButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
	m_inputState.bButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
	m_inputState.startButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);
	m_inputState.selectButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
	m_inputState.selectButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
	m_inputState.upButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP) || joyStickUp;
	m_inputState.downButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN) || joyStickDown;
	m_inputState.leftButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT) || joyStickLeft;
	m_inputState.rightButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || joyStickRight;
}

bool InputHandler::controllerButtonPressed(SDL_GameControllerButton button)
{
	if (!m_controller)
		return false;
	return SDL_GameControllerGetButton(m_controller, button);
}
