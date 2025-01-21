#include "OpenGLContext.hpp"
#ifdef TAKO_GLFW
#include "GLFW/glfw3.h"
#endif
#include "OpenGLPixelArtDrawer.hpp"
#include "OpenGL.hpp"
#include "Utility.hpp"

#ifdef TAKO_WIN32
#include <Windows.h>
#pragma comment(lib, "opengl32.lib")

auto GetGLProcAddress(const char* name)
{
	auto procAddress = wglGetProcAddress(name);
	if (procAddress == nullptr)
	{
		static auto module = LoadLibrary("OPENGL32.DLL");
		procAddress = GetProcAddress(module, name);
	}

	return procAddress;
}
#endif

import Tako.Bitmap;
import Tako.Math;

namespace tako
{
	OpenGLContext::OpenGLContext(Window* window)
	{
		m_handle = window->GetHandle();
#ifdef TAKO_GLFW
		glfwMakeContextCurrent(m_handle);
#ifdef TAKO_GLAD
		gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);
#endif
#endif
#ifdef TAKO_WIN32
		auto m_hdc = GetDC(m_handle);
		PIXELFORMATDESCRIPTOR pfd;

		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;

		int pf = ChoosePixelFormat(m_hdc, &pfd);
		SetPixelFormat(m_hdc, pf, &pfd);
		DescribePixelFormat(m_hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		ReleaseDC(m_handle, m_hdc);


		auto m_hrc = wglCreateContext(m_hdc);
		wglMakeCurrent(m_hdc, m_hrc);
#ifdef TAKO_GLAD
		gladLoadGLES2Loader((GLADloadproc)GetGLProcAddress);
#endif
#endif
		Resize(window->GetWidth(), window->GetHeight());
	}

	GraphicsAPI OpenGLContext::GetAPI()
	{
		return GraphicsAPI::OpenGL;
	}

	void OpenGLContext::Resize(int w, int h)
	{
		m_width = w;
		m_height = h;
	}

	void OpenGLContext::HandleEvent(Event& evt)
	{
		switch (evt.GetType())
		{
		case tako::EventType::WindowResize:
		{
			tako::WindowResize& res = static_cast<tako::WindowResize&>(evt);
			LOG("Window Resize: {} {} {}", res.GetName(), res.width, res.height);
			Resize(res.width, res.height);
		} break;
		}
	}

	U32 OpenGLContext::GetWidth()
	{
		return m_width;
	}

	U32 OpenGLContext::GetHeight()
	{
		return m_height;
	}

	Texture OpenGLContext::CreateTexture(const ImageView image)
	{
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());

		Texture t;
		t.width = image.GetWidth();
		t.height = image.GetHeight();
		t.handle.value = tex;
		return t;
	}

	Texture OpenGLContext::CreateTexture(const std::span<const ImageView> images)
	{
		ASSERT(false);
		return {};
	}

	void OpenGLContext::Begin()
	{
#ifdef TAKO_GLFW
		glfwMakeContextCurrent(m_handle);
#endif
	}

	void OpenGLContext::End()
	{

	}

	void OpenGLContext::Present()
	{
		//m_impl->Draw();
		glFlush();
#ifdef TAKO_GLFW
		glfwSwapBuffers(m_handle);
#endif
#ifdef TAKO_WIN32
		SwapBuffers(GetDC(m_handle));
#endif
	}

	//TODO: implement
	void OpenGLContext::BindPipeline(const Pipeline *pipeline)
	{

	}

	void OpenGLContext::BindVertexBuffer(const Buffer *buffer)
	{

	}

	void OpenGLContext::BindIndexBuffer(const Buffer *buffer)
	{

	}

	void OpenGLContext::BindMaterial(const Material *material)
	{

	}

	void OpenGLContext::UpdateCamera(const CameraUniformData &cameraData)
	{

	}

	void OpenGLContext::UpdateUniform(const void *uniformData, size_t uniformSize, size_t offset)
	{

	}

	void OpenGLContext::Draw(U32 vertexCount)
	{

	}

	void OpenGLContext::DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix)
	{

	}

	void OpenGLContext::DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix)
	{
	}

	Pipeline OpenGLContext::CreatePipeline(const PipelineDescriptor &pipelineDescriptor)
	{
		return Pipeline();
	}

	Material OpenGLContext::CreateMaterial(const Texture texture, const MaterialDescriptor& materialDescriptor)
	{
		return Material();
	}

	Buffer OpenGLContext::CreateBuffer(BufferType bufferType, const void *bufferData, size_t bufferSize)
	{
		return Buffer();
	}

	void OpenGLContext::ReleaseBuffer(Buffer buffer)
	{

	}
}
