#pragma once
#include <RenderingUtility.hpp>
#include <QImage>

class QTRenderer : public ggb::Renderer
{
public:
	QTRenderer(int width, int height);
	void renderNewFrame(const ggb::FrameBuffer& framebuffer) override;
	bool hasNewImage() const;
	QImage getCurrentImage();
	void setFrameSkip(int skipFrames);

private:
	int m_frameSkipCount = 0;
	int m_skipImageCounter = 0;
	bool m_hasNewImage = false;
	QImage m_image;
	int m_width;
	int m_height;
};
