#include "MainWindow.hpp"

MainWindow::MainWindow() : QMainWindow(nullptr), m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);

	m_emulatorThread = new EmulatorThread(this);
	QObject::connect(m_emulatorThread, &EmulatorThread::renderedImage, this, &MainWindow::updateImage);
	m_emulatorThread->start();
}

void MainWindow::updateImage(QImage image)
{
	auto upscaled = image.scaled(image.size() * 5);
	auto buf = QPixmap::fromImage(upscaled);
	m_ui->GameImage->setPixmap(buf);
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