#pragma once
#include <QMainWindow>
#include <QWindow>
//#include <QtGamepad>

#include "EmulatorMain.hpp"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
public slots:
	void updateImage(QImage image);

private:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	QWidget* m_windowContainer = nullptr;
	QWindow* m_window = nullptr;
	Ui::MainWindow* m_ui = nullptr;
	EmulatorThread* m_emulatorThread = nullptr;
};

