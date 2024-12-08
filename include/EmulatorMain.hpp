#pragma once
#include <cassert>
#include <iostream>
#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <Input.hpp>
#include <QThread>
#include <QImage>
#include <QPixMap>
#include <QKeyEvent>
#include <Emulator.hpp>
#include <RenderingUtility.hpp>

#include "Video.hpp"
#include "Audio.hpp"
#include "Inputhandling.hpp"
#include "SDL.h"

struct KeyEvent 
{
	int key;
	bool pressed;
};

class EmulatorThread : public QThread 
{
	Q_OBJECT
public:
	EmulatorThread(QObject* parent);
	void loadROM(const std::filesystem::path& path);
	void postEvent(KeyEvent event);

signals:
	void renderedImage(QImage image);

protected:
	void run() override;

private:
	void updateInput();
	void handleEmulatorKeyPress(int key);

	bool m_previousKeyStates[SDL_NUM_SCANCODES] = {};
	std::unique_ptr<ggb::Emulator> m_emulator = nullptr;
	std::unique_ptr<Audio> m_audioHandler = nullptr;
	InputHandler* m_inputHandler = nullptr;
	//std::unique_ptr<QTRenderer> m_tileDataRenderer = nullptr;
	QTRenderer* m_gameRenderer = nullptr;
	std::unordered_map<int, bool> m_keyStates;
	std::vector<KeyEvent> m_pendingEvents;
	std::mutex m_mutex;
};
