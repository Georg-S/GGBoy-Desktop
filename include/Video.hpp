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

private:
	bool m_hasNewImage = false;
	QImage m_image;
	int m_width;
	int m_height;
};
