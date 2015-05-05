#pragma once 

#include "../Utilities/ClPlatform.h"

#include "../GLRenderer/GLRenderer.h"

#if PLATFORM  == PLATFORM_WIN32
	#include "../DXRenderer/DXRenderer.h"
#endif

#include "IRenderer.h"

class Clockwork
{
public:

	//The renderer the the engine will use
	enum Renderer
	{
		OpenGL4,
		DirectX11
	};

	Clockwork(){/* Not used */}
	~Clockwork(){/* Not used */ }

	// Init the game engine with the specified renderer
	void Init(Renderer renderer);

	// Run the main loop of the game engine, implements scripts.
	void Run();

	// Shutdown the game engine and end the game. Should deallocate all systems correctly
	void Shutdown();
private:
	IRenderer* m_renderer;
	
};