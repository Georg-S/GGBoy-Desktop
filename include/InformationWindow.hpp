#pragma once
#include <QWidget>
#include <vector>

#include "ui_informationwindow.h"

class InformationWindow : public QWidget
{
	Q_OBJECT
public:
	InformationWindow(QWidget* parent = nullptr);
	void addSpeedup(double speedUp);

private:
	void updateInformations();

	Ui::InformationWindow* m_ui;
	std::vector<double> m_maxSpeedups;
	const size_t m_maxSizeForSpeedups = 10;
	size_t m_currentIndex = 0;
	bool m_wrappedAround = false;
};