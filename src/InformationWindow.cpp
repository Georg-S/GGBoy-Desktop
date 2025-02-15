#include "InformationWindow.hpp"

InformationWindow::InformationWindow(QWidget* parent) 
	: QWidget(parent)
	, m_ui(new Ui::InformationWindow)
{
	m_ui->setupUi(this);
}

void InformationWindow::addSpeedup(double speedUp)
{
	if (m_wrappedAround)
	{
		m_maxSpeedups[m_currentIndex] = speedUp;
	}
	else
	{
		m_maxSpeedups.emplace_back(speedUp);
	}

	if (m_currentIndex == m_maxSizeForSpeedups) 
	{
		m_wrappedAround = true;
		m_currentIndex -= m_maxSizeForSpeedups;
	}

	updateInformations();
	m_currentIndex++;
}

void InformationWindow::updateInformations()
{
	double average = 0.0;
	for (auto val : m_maxSpeedups) 
		average += val;
	average /= m_maxSpeedups.size();

	auto current = m_maxSpeedups[m_currentIndex];

	m_ui->averageSpeedupLineEdit->setText(QString::number(average, 'f', 2));
	m_ui->currentSpeedupLineEdit->setText(QString::number(current, 'f', 2));
}
