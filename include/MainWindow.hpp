#pragma once
#include <QMainWindow>
#include <QWindow>
#include <QMessageBox>
#include <QFileDialog>

#include <memory>

#include "InformationWindow.hpp"
#include "EmulatorMain.hpp"

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	 ~MainWindow();
public slots:
	void currentMaxSpeedup(double speedUp);
	void updateImage(QImage image);
	void warning(QString errorString);

private:
	void openROM();
	void toggleInformationWindow();
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	QWidget* m_windowContainer = nullptr;
	QWindow* m_window = nullptr;
	Ui::MainWindow* m_ui = nullptr;
	EmulatorThread* m_emulatorThread = nullptr;
	std::unique_ptr<InformationWindow> m_informationWindow = nullptr;
};

