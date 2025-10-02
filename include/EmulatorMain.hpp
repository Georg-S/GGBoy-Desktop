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
	void setROM(std::filesystem::path path);
	void postEvent(KeyEvent event);
	void quit();

signals:
	void renderedImage(QImage image);
	void currentMaxSpeedup(double speedUp);
	void warning(QString errorString);

protected:
	void run() override;

private:
	void performanceProfiling();
	std::string getCartridgeName();
	void loadRAM();
	void loadRTC();
	void loadROM(const std::filesystem::path& path);
	void saveCartridgeRAM();
	void saveCartridgeRTC();
	// Returns the path which file should be written / overwritten
	std::filesystem::path getFileSavePath(const std::string& fileName, const std::string& fileExtension);
	void updateInput();
	void handleEmulatorKeyPress(int key);

	std::unique_ptr<ggb::Emulator> m_emulator = nullptr;
	std::unique_ptr<Audio> m_audioHandler = nullptr;
	std::unique_ptr<InputHandler> m_inputHandler = nullptr;
	//std::unique_ptr<QTRenderer> m_tileDataRenderer = nullptr;
	QTRenderer* m_gameRenderer = nullptr;
	bool m_quit = false;
	std::unordered_map<int, bool> m_keyStates;
	std::vector<KeyEvent> m_pendingKeyEvents;
	std::filesystem::path m_romToBeLoaded;
	std::mutex m_inputMutex;
	std::mutex m_emulatorEventsMutex;
	static constexpr bool runPerformanceProfiling = false;
};
