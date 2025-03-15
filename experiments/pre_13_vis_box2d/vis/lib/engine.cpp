module;

#include <SDL3/SDL.h>

export module vis:engine;

import std;

import :math;
import :opengl;

export namespace vis::engine {
class Engine {
public:
	friend Engine create(SDL_Window* window);

	static void set_clear_color(const vis::vec4& color) {
		vis::opengl::renderer_set_clear_color(color);
	}

	static void clear() {
		vis::opengl::renderer_clear();
	}

	static void render(SDL_Window* window) {
		vis::opengl::renderer_render(window);
	}

	static void set_viewport(int x, int y, int width, int height) {
		vis::opengl::renderer_set_viewport(x, y, width, height);
	}

	static void print_info() {
		std::println("{}: ", vis::opengl::renderer_print_info());
	}

private:
	explicit Engine(SDL_Window* window, SDL_GLContext context) : window{window}, opengl_context{context} {}

private:
	SDL_Window* window = nullptr;
	SDL_GLContext opengl_context = nullptr;
};

Engine create(SDL_Window* window) {
	if (not SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		throw std::runtime_error{std::format("Unable to initialize SDL subsystems: {}", SDL_GetError())};
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifndef NDEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	SDL_GLContext opengl_context = SDL_GL_CreateContext(window);

	if (not opengl_context) {
		throw std::runtime_error{std::format("Unable to initialize OpenGL: {}", SDL_GetError())};
	}

	if (not SDL_GL_MakeCurrent(window, opengl_context)) {
		SDL_GL_DestroyContext(opengl_context);
		throw std::runtime_error{std::format("It's not possible to init the graphic")};
	}

	vis::opengl::renderer_init();

	if (not SDL_GL_SetSwapInterval(1)) {
		std::println("It's not possible to set the vsync");
		SDL_GL_DestroyContext(opengl_context);
		SDL_DestroyWindow(window);
	}

	return Engine(window, opengl_context);
}

} // namespace vis::engine