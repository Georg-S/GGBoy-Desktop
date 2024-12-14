#include "EmulatorMain.hpp"

static std::filesystem::path cartridgePath = "";
static const std::filesystem::path RAM_BASE_PATH = "RAM/";
static const std::filesystem::path GAMES_BASE_PATH = "Roms/Games/";

EmulatorThread::EmulatorThread(QObject* parent) : QThread(parent)
{
	m_emulator = std::make_unique<ggb::Emulator>();
	m_audioHandler = std::make_unique<Audio>(m_emulator->getSampleBuffer());
	auto inputHandler = std::make_unique<InputHandler>();
	m_inputHandler = inputHandler.get();

	auto tileDataDimensions = m_emulator->getTileDataDimensions();
	auto gameWindowDimensions = m_emulator->getGameWindowDimensions();
	//m_tileDataRenderer = std::make_unique<SDLRenderer>(tileDataDimensions.width, tileDataDimensions.height, 4);
	auto gameRenderer = std::make_unique<QTRenderer>(gameWindowDimensions.width, gameWindowDimensions.height);
	m_gameRenderer = gameRenderer.get();
	//loadROM(GAMES_BASE_PATH / "Teenage_Mutant_Hero_Turtles_3.gb");
	loadROM(GAMES_BASE_PATH / "Pokemon_Kristall.gbc");
	//loadROM(GAMES_BASE_PATH / "DragonBallZ.gbc");

	m_emulator->setGameRenderer(std::move(gameRenderer));
	m_emulator->setInput(std::move(inputHandler));
}

void EmulatorThread::loadROM(const std::filesystem::path& path)
{
	m_emulator->loadCartridge(path);
}

void EmulatorThread::postEvent(KeyEvent event)
{
	std::scoped_lock lock(m_mutex);
	m_pendingEvents.emplace_back(std::move(event));
}

void EmulatorThread::run()
{
	long long lastTimeStamp = ggb::getCurrentTimeInNanoSeconds();
	static constexpr long long NANO_SECONDS_PER_SECOND = 1000000000;
	static constexpr long long INPUT_UPDATE_AFTER_NANOSECONDS = 10000000;
	constexpr static int UPDATE_AFTER_STEPS = 20;
	int stepCounter = 0;
	long long inputUpdateCounter = 0;
	long long maxSpeedupCounter = 0;
	while (true)
	{
		m_emulator->step();
		if (m_gameRenderer->hasNewImage())
			emit renderedImage(m_gameRenderer->getCurrentImage());

		stepCounter++;
		if (stepCounter < UPDATE_AFTER_STEPS)
			continue;

		stepCounter = 0;
		auto currentTime = ggb::getCurrentTimeInNanoSeconds();
		auto timePast = currentTime - lastTimeStamp;
		lastTimeStamp = currentTime;
		inputUpdateCounter += timePast;
		maxSpeedupCounter += timePast;
		if (inputUpdateCounter > INPUT_UPDATE_AFTER_NANOSECONDS)
		{
			inputUpdateCounter -= INPUT_UPDATE_AFTER_NANOSECONDS;
			updateInput();
		}

		if (maxSpeedupCounter > NANO_SECONDS_PER_SECOND) 
		{
			maxSpeedupCounter -= NANO_SECONDS_PER_SECOND;
			emit currentMaxSpeedup(m_emulator->getMaxSpeedup());
		}
	}
}

void EmulatorThread::updateInput()
{
	std::vector<KeyEvent> events;
	{
		std::scoped_lock lock(m_mutex);
		events = std::move(m_pendingEvents);
		m_pendingEvents.clear();
	}

	for (const auto& event : events) 
	{
		if (event.pressed)
			handleEmulatorKeyPress(event.key);
		m_keyStates[event.key] = event.pressed;
	}

	m_inputHandler->update(m_keyStates);
}

void EmulatorThread::handleEmulatorKeyPress(int key)
{
	if (key == Qt::Key::Key_R)
		m_emulator->reset();
	if (key == Qt::Key::Key_F1)
		m_emulator->saveEmulatorState("Savestate1.bin");
	if (key == Qt::Key::Key_F2)
		m_emulator->saveEmulatorState("Savestate2.bin");
	if (key == Qt::Key::Key_F3)
		m_emulator->saveEmulatorState("Savestate3.bin");
	if (key == Qt::Key::Key_F4)
		m_emulator->saveEmulatorState("Savestate4.bin");
	if (key == Qt::Key::Key_F5)
		m_emulator->loadEmulatorState("Savestate1.bin");
	if (key == Qt::Key::Key_F6)
		m_emulator->loadEmulatorState("Savestate2.bin");
	if (key == Qt::Key::Key_F7)
		m_emulator->loadEmulatorState("Savestate3.bin");
	if (key == Qt::Key::Key_F8)
		m_emulator->loadEmulatorState("Savestate4.bin");
	if (key == Qt::Key::Key_F9)
		m_emulator->muteChannel(0, !m_emulator->isChannelMuted(0));
	if (key == Qt::Key::Key_F10)
		m_emulator->muteChannel(1, !m_emulator->isChannelMuted(1));
	if (key == Qt::Key::Key_F11)
		m_emulator->muteChannel(2, !m_emulator->isChannelMuted(2));
	if (key == Qt::Key::Key_F12)
		m_emulator->muteChannel(3, !m_emulator->isChannelMuted(3));
	if (key == Qt::Key::Key_Escape)
		m_emulator->saveRAM(RAM_BASE_PATH / cartridgePath.filename());
	if (key == Qt::Key::Key_T)
	{
		if (m_emulator->emulationSpeed() == 1.0)
		{
			m_emulator->setEmulationSpeed(30.0);
			m_audioHandler->setAudioPlaying(false);
		}
		else
		{
			m_emulator->setEmulationSpeed(1.0);
			m_audioHandler->setAudioPlaying(true);
		}
	}
}
