#include "Video.hpp"

QTRenderer::QTRenderer(int width, int height)
	: m_width(width), m_height(height)
{
}

void QTRenderer::renderNewFrame(const ggb::FrameBuffer& framebuffer)
{
	m_skipImageCounter++;
	if (m_skipImageCounter < m_frameSkipCount)
		return;

	m_skipImageCounter = 0;
	m_hasNewImage = true;
	m_image = QImage(QSize(m_width, m_height), QImage::Format_RGB32);

	for (size_t y = 0; y < framebuffer.height(); y++)
	{
		QRgb* scanLine = reinterpret_cast<QRgb*>(m_image.scanLine(static_cast<int>(y)));
		for (size_t x = 0; x < framebuffer.width(); x++)
		{
			const auto& ggbColor = framebuffer.getPixel(x, y);
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

void QTRenderer::setFrameSkip(int skipFrames)
{
	m_frameSkipCount = skipFrames;
}
