#include "MainWindow.hpp"

MainWindow::MainWindow() : QMainWindow(nullptr), m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);
	m_emulatorThread = new EmulatorThread(this);
	m_informationWindow = std::make_unique<InformationWindow>();
	m_informationWindow->hide();

	connect(m_ui->actionOpenROM, &QAction::triggered, this, &MainWindow::openROM);
	connect(m_ui->actionInformations, &QAction::triggered, this, &MainWindow::toggleInformationWindow);
	connect(m_emulatorThread, &EmulatorThread::renderedImage, this, &MainWindow::updateImage);
	connect(m_emulatorThread, &EmulatorThread::currentMaxSpeedup, this, &MainWindow::currentMaxSpeedup);
	connect(m_emulatorThread, &EmulatorThread::warning, this, &MainWindow::warning);
	m_emulatorThread->start();
}

MainWindow::~MainWindow()
{
	m_emulatorThread->quit();
	m_emulatorThread->wait();
}

void MainWindow::currentMaxSpeedup(double speedUp)
{
	m_informationWindow->addSpeedup(speedUp);
}

void MainWindow::updateImage(QImage image)
{
	auto upscaled = image.scaled(image.size() * 5);
	auto buf = QPixmap::fromImage(upscaled);
	m_ui->GameImage->setPixmap(buf);
}

void MainWindow::warning(QString errorString)
{
	QMessageBox messageBox;
	messageBox.warning(this, "Warning", errorString);
}

void MainWindow::openROM()
{
	auto fileName = QFileDialog::getOpenFileName(this, "Open ROM", "ROMs", "ROM Files (*.gb *.gbc);; All (*.*)");
	if (fileName.isEmpty())
		return;
	std::filesystem::path path(fileName.toStdU16String());
	m_emulatorThread->setROM(std::move(path));
}

void MainWindow::toggleInformationWindow()
{
	if (m_informationWindow->isHidden())
		m_informationWindow->show();
	else
		m_informationWindow->hide();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	KeyEvent newEvent = { event->key(), true };
	m_emulatorThread->postEvent(std::move(newEvent));
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
	KeyEvent newEvent = { event->key(), false };
	m_emulatorThread->postEvent(std::move(newEvent));
}