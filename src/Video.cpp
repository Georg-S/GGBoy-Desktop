#include "Video.hpp"

QTRenderer::QTRenderer(int width, int height)
	: m_width(width), m_height(height)
{
}

void QTRenderer::renderNewFrame(const ggb::FrameBuffer& framebuffer)
{
	m_hasNewImage = true;
	m_image = QImage(QSize(m_width, m_height), QImage::Format_RGB32);

	for (int y = 0; y < framebuffer.m_buffer[0].size(); y++)
	{
		QRgb* scanLine = reinterpret_cast<QRgb*>(m_image.scanLine(y));
		for (int x = 0; x < framebuffer.m_buffer.size(); x++)
		{
			const auto& ggbColor = framebuffer.m_buffer[x][y];
			scanLine[x] = qRgb(ggbColor.r, ggbColor.g, ggbColor.b);
		}
	}
}

bool QTRenderer::hasNewImage() const
{
	return m_hasNewImage;
}

QImage QTRenderer::getCurrentImage()
{
	m_hasNewImage = false;
	return m_image;
}
