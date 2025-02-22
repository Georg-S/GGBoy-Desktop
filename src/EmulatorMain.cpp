#include "EmulatorMain.hpp"

#include <chrono>
#include <regex>

static std::filesystem::path cartridgePath = "";
static const std::filesystem::path SAVE_STATE_BASE_PATH = "Savestates/";
static const std::filesystem::path RAM_BASE_PATH = "RAM/";
static const std::filesystem::path CARTRIDGE_DATA_BASE_PATH = "CARTRIDGE_DATA/";
static const std::filesystem::path GAMES_BASE_PATH = "Roms/Games/";
static const std::string RAM_FILE_ENDING = ".bin";
static const std::string RAM_FILE_SUFFIX = "_ram";
static const std::string RTC_FILE_SUFFIX = "_RTC";
static const std::string SAVESTATE_FILE_ENDING = ".bin";

namespace
{
	class Timer
	{
	public:
		Timer(long long timeout, std::function<void()> timeoutCallback)
		{
			m_timeout = timeout;
			m_timeoutCallback = std::move(timeoutCallback);
		}

		void update(long long duration)
		{
			m_counter += duration;
			if (m_counter < m_timeout)
				return;

			m_counter -= m_timeout;
			m_timeoutCallback();
		};

	private:
		long long m_counter = 0;
		long long m_timeout = 0;
		std::function<void()> m_timeoutCallback;
	};
}

template <typename TP>
static std::time_t to_time_t(TP tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());

	return system_clock::to_time_t(sctp);
}

static std::string regexEscape(std::string str)
{
	static const std::regex specialChars{ R"([-[\]{}()*+?.,\^$|#\s])" };

	return std::regex_replace(str, specialChars, R"(\$&)");
}

/// Returns the oldest / newest file given the specified path and name (only files with '<name>1', '<name>2' etc. are used)
static std::vector<std::pair<std::filesystem::path, std::time_t>> getFilePaths(const std::filesystem::path& base, const std::string fileName)
{
	if (!std::filesystem::is_directory(base))
		return {};

	std::filesystem::path result = {};

	std::vector<std::pair<std::filesystem::path, std::time_t>> files;
	for (const auto& entry : std::filesystem::directory_iterator(base))
	{
		if (!std::filesystem::is_regular_file(entry))
			continue;

		const auto currentFileName = entry.path().stem().u8string();
		const auto regexString = regexEscape(fileName) + "\\d+";
		if (!std::regex_match(currentFileName, std::regex(regexString)))
			continue;

		const auto lastEditedTime = to_time_t(std::filesystem::last_write_time(entry));
		auto pair = std::make_pair(entry.path(), lastEditedTime);
		files.emplace_back(std::move(pair));
	}

	std::sort(files.begin(), files.end(), [](const std::pair<std::filesystem::path, std::time_t>& lhs, const std::pair<std::filesystem::path, std::time_t>& rhs)
		{
			return lhs.second > rhs.second;
		});

	if (files.empty())
		return {};

	return files;
}

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

	m_emulator->setGameRenderer(std::move(gameRenderer));
	m_emulator->setInput(std::move(inputHandler));
}

void EmulatorThread::setROM(std::filesystem::path path)
{
	std::scoped_lock lock(m_emulatorEventsMutex);
	m_romToBeLoaded = std::move(path);
}

void EmulatorThread::postEvent(KeyEvent event)
{
	std::scoped_lock lock(m_inputMutex);
	m_pendingKeyEvents.emplace_back(std::move(event));
}

void EmulatorThread::quit()
{
	std::scoped_lock lock(m_emulatorEventsMutex);
	m_quit = true;
}

void EmulatorThread::run()
{
	static constexpr long long NANO_SECONDS_PER_SECOND = 1000000000;
	static constexpr int UPDATE_AFTER_STEPS = 20;

	bool running = true;

	auto maxSpeedupTimer = Timer(NANO_SECONDS_PER_SECOND, [this]()
		{
			emit currentMaxSpeedup(m_emulator->getMaxSpeedup());
		});

	auto inputTimer = Timer(NANO_SECONDS_PER_SECOND / 100, [this]()
		{
			updateInput();
		});

	auto emulatorEventsTimer = Timer(NANO_SECONDS_PER_SECOND / 3, [this, &running]()
		{
			std::scoped_lock lock(m_emulatorEventsMutex);
			running = !m_quit;
			if (m_romToBeLoaded.empty())
				return;
			loadROM(m_romToBeLoaded);
			m_romToBeLoaded.clear();
		});

	long long lastTimeStamp = ggb::getCurrentTimeInNanoSeconds();
	int stepCounter = 0;
	while (running)
	{
		if (!m_emulator->isCartridgeLoaded())
		{
			const auto currentTime = ggb::getCurrentTimeInNanoSeconds();
			const auto timePast = currentTime - lastTimeStamp;
			emulatorEventsTimer.update(timePast);
			continue;
		}

		m_emulator->step();
		if (m_gameRenderer->hasNewImage())
			emit renderedImage(m_gameRenderer->getCurrentImage());

		stepCounter++;
		if (stepCounter < UPDATE_AFTER_STEPS)
			continue;

		stepCounter = 0;
		const auto currentTime = ggb::getCurrentTimeInNanoSeconds();
		const auto timePast = currentTime - lastTimeStamp;
		lastTimeStamp = currentTime;

		maxSpeedupTimer.update(timePast);
		inputTimer.update(timePast);
		emulatorEventsTimer.update(timePast);
	}

	saveCartridgeRAM();
	saveCartridgeRTC();
}

std::string EmulatorThread::getCartridgeName()
{
	auto loadedPath = m_emulator->getLoadedCartridgePath();
	if (loadedPath.empty())
		return {};

	const auto gameName = loadedPath.filename().stem().u8string();
	if (gameName.empty())
	{
		emit warning(QString("Unable to get cartridge name, no valid filename loaded"));
		return {};
	}

	return gameName;
}

void EmulatorThread::loadRAM()
{
	auto gameName = getCartridgeName();
	if (gameName.empty())
		return;

	std::filesystem::path path = CARTRIDGE_DATA_BASE_PATH;
	auto paths = getFilePaths(CARTRIDGE_DATA_BASE_PATH, gameName + RAM_FILE_SUFFIX);
	if (paths.empty())
		return;

	const auto& pathToLoad = paths.front().first;
	try
	{
		m_emulator->loadRAM(pathToLoad);
	}
	catch (const std::exception& e)
	{
		auto pathString = QString::fromStdString(pathToLoad.u8string());
		auto errorStr = QString::fromUtf8(e.what());
		emit warning(QString("Unable to load ram '%1' \n %2").arg(pathString, errorStr));
	}
}

void EmulatorThread::loadRTC()
{
	auto gameName = getCartridgeName();
	if (gameName.empty())
		return;

	std::filesystem::path path = CARTRIDGE_DATA_BASE_PATH;
	auto paths = getFilePaths(CARTRIDGE_DATA_BASE_PATH, gameName + RTC_FILE_SUFFIX);
	if (paths.empty())
		return;
	const auto& pathToLoad = paths.front().first;

	try
	{
		m_emulator->loadRTC(pathToLoad);
	}
	catch (const std::exception& e)
	{
		auto pathString = QString::fromStdString(pathToLoad.u8string());
		auto errorStr = QString::fromUtf8(e.what());
		emit warning(QString("Unable to load real time clock '%1' \n %2").arg(pathString, errorStr));
	}
}

void EmulatorThread::loadROM(const std::filesystem::path& path)
{
	saveCartridgeRAM();
	saveCartridgeRTC();

	try
	{
		m_emulator->loadCartridge(path);
	}
	catch (const std::exception& e)
	{
		auto pathString = QString::fromStdString(path.u8string());
		auto errorStr = QString::fromUtf8(e.what());
		emit warning(QString("Unable to load '%1' \n %2").arg(pathString, errorStr));
	}

	loadRAM();
	loadRTC();
}

void EmulatorThread::saveCartridgeRAM()
{
	auto pathToWrite = getFileSavePath(RAM_FILE_SUFFIX, RAM_FILE_ENDING);
	if (pathToWrite.empty())
		return;

	try
	{
		m_emulator->saveRAM(pathToWrite);
	}
	catch (const std::exception& e)
	{
		auto pathString = QString::fromStdString(pathToWrite.u8string());
		auto errorStr = QString::fromUtf8(e.what());
		emit warning(QString("Unable to save '%1' \n %2").arg(pathString, errorStr));
	}
}

void EmulatorThread::saveCartridgeRTC()
{
	auto pathToWrite = getFileSavePath(RTC_FILE_SUFFIX, RAM_FILE_ENDING);
	if (pathToWrite.empty())
		return;

	try
	{
		m_emulator->saveRTC(pathToWrite);
	}
	catch (const std::exception& e)
	{
		auto pathString = QString::fromStdString(pathToWrite.u8string());
		auto errorStr = QString::fromUtf8(e.what());
		emit warning(QString("Unable to save '%1' \n %2").arg(pathString, errorStr));
	}
}

std::filesystem::path EmulatorThread::getFileSavePath(const std::string& fileName, const std::string& fileExtension)
{
	auto gameName = getCartridgeName();
	if (gameName.empty())
		return {};

	std::filesystem::path path = CARTRIDGE_DATA_BASE_PATH;
	try 
	{
		if (!std::filesystem::exists(CARTRIDGE_DATA_BASE_PATH))
			std::filesystem::create_directories(CARTRIDGE_DATA_BASE_PATH);

		auto paths = getFilePaths(CARTRIDGE_DATA_BASE_PATH, gameName + fileName);
		if (paths.size() <= 1)
		{
			auto toTestPath = CARTRIDGE_DATA_BASE_PATH / (gameName + (fileName + "0") + RAM_FILE_ENDING);
			if (!paths.empty() && (paths.front().first == toTestPath))
				path /= gameName + (fileName + "1") + RAM_FILE_ENDING;
			else
				path /= gameName + (fileName + "0") + RAM_FILE_ENDING;
		}
		else
		{
			// More than one file, use the oldest
			path = paths.back().first;
		}
		return path;
	}
	catch (const std::exception& e)
	{
		auto pathString = QString::fromStdString(path.u8string());
		auto errorStr = QString::fromUtf8(e.what());
		emit warning(QString("Unable to get file name '%1' \n %2").arg(pathString, errorStr));
	}

	return {};
}

void EmulatorThread::updateInput()
{
	std::vector<KeyEvent> events;
	{
		std::scoped_lock lock(m_inputMutex);
		events = std::move(m_pendingKeyEvents);
		m_pendingKeyEvents.clear();
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
	auto saveSavestate = [this](int number)
	{
		try
		{
			if (!std::filesystem::exists(SAVE_STATE_BASE_PATH))
				std::filesystem::create_directory(SAVE_STATE_BASE_PATH);
		}
		catch (const std::exception& e)
		{
			emit warning(QString("Unable to save savestate: %1").arg(e.what()));
		}

		if (!m_emulator->saveEmulatorState(SAVE_STATE_BASE_PATH / ("Savestate" + std::to_string(number) + SAVESTATE_FILE_ENDING)))
			emit warning(QString("Unable to save savestate%1").arg(number));
	};

	auto loadSavestate = [this](int number)
	{
		auto saveStatePath = SAVE_STATE_BASE_PATH / ("Savestate" + std::to_string(number) + SAVESTATE_FILE_ENDING);
		if (!std::filesystem::exists(saveStatePath)) 
		{
			emit warning(QString("Savestate %1 does not exist").arg(number));
			return;
		}

		if (!m_emulator->loadEmulatorState(saveStatePath))
			emit warning(QString("Unable to load savestate%1").arg(number));
	};

	if (key == Qt::Key::Key_R)
		m_emulator->reset();
	if (key == Qt::Key::Key_F1)
		saveSavestate(1);
	if (key == Qt::Key::Key_F2)
		saveSavestate(2);
	if (key == Qt::Key::Key_F3)
		saveSavestate(3);
	if (key == Qt::Key::Key_F4)
		saveSavestate(4);
	if (key == Qt::Key::Key_F5)
		loadSavestate(1);
	if (key == Qt::Key::Key_F6)
		loadSavestate(2);
	if (key == Qt::Key::Key_F7)
		loadSavestate(3);
	if (key == Qt::Key::Key_F8)
		loadSavestate(4);
	if (key == Qt::Key::Key_F9)
		m_emulator->muteChannel(0, !m_emulator->isChannelMuted(0));
	if (key == Qt::Key::Key_F10)
		m_emulator->muteChannel(1, !m_emulator->isChannelMuted(1));
	if (key == Qt::Key::Key_F11)
		m_emulator->muteChannel(2, !m_emulator->isChannelMuted(2));
	if (key == Qt::Key::Key_F12)
		m_emulator->muteChannel(3, !m_emulator->isChannelMuted(3));
	if (key == Qt::Key::Key_T)
	{
		if (m_emulator->emulationSpeed() == 1.0)
		{
			m_emulator->setEmulationSpeed(5.0);
			m_audioHandler->setAudioPlaying(false);
		}
		else
		{
			m_emulator->setEmulationSpeed(1.0);
			m_audioHandler->setAudioPlaying(true);
		}
	}
}
