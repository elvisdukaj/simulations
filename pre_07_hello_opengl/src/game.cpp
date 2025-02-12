
module;

#include <GL/glew.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

#include <exception>
#include <numbers>
#include <print>
#include <ratio>

export module Game;

import vis;

export {
	namespace Game {

	constexpr int SCREEN_HEIGHT = 600;
	constexpr std::ratio<4, 3> ASPECT_RATIO;
	constexpr int SCREEN_WIDTH = 800; // SCREEN_HEIGHT * ASPECT_RATIO.num / ASPECT_RATIO.den;

	class App {
	public:
		static App* create() noexcept {
			if (not SDL_InitSubSystem(SDL_INIT_VIDEO)) {
				std::println("Unable to initialize SDL subsystems: {}", SDL_GetError());
				return nullptr;
			}

			SDL_Window* window = SDL_CreateWindow("Hello OpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, screen_flags);

			if (not window) {
				std::println("Unable to create the window or the renderer: {}", SDL_GetError());
				return nullptr;
			}

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifndef NDEBUG
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

			SDL_GLContext opengl_context = SDL_GL_CreateContext(window);

			if (not opengl_context) {
				std::println("Unable to initialize OpenGL: {}", SDL_GetError());
				return nullptr;
			}

			if (not SDL_GL_MakeCurrent(window, opengl_context)) {
				std::println("It's not possible to init the graphic");
				SDL_GL_DestroyContext(opengl_context);
				SDL_DestroyWindow(window);
				return nullptr;
			}

			auto glewStatus = glewInit();
			if (glewStatus != GLEW_OK) {
				std::println("It's not possible to init the graphic");
				SDL_GL_DestroyContext(opengl_context);
				SDL_DestroyWindow(window);
				return nullptr;
			}

			// Get graphics info
			auto renderer = (const char*)glGetString(GL_RENDERER);
			auto version = (const char*)glGetString(GL_VERSION);
			std::println("Renderer {}", renderer);
			std::println("OpenGL version supported {}", version);

			if (not SDL_GL_SetSwapInterval(1)) {
				std::println("It's not possible to set the vsync");
				SDL_GL_DestroyContext(opengl_context);
				SDL_DestroyWindow(window);
				return nullptr;
			}

			return new App(window, opengl_context);
		}

		~App() {
			SDL_GL_DestroyContext(opengl_context);
			SDL_DestroyWindow(window);
		}

		[[nodiscard]] SDL_AppResult processEvent(const SDL_Event* event) noexcept {
			if (event->type == SDL_EVENT_QUIT) {
				return SDL_AppResult::SDL_APP_SUCCESS;
			}

			switch (event->type) {
			case SDL_EVENT_KEY_DOWN: {
				switch (event->key.key) {
				case SDLK_ESCAPE:
					return SDL_AppResult::SDL_APP_SUCCESS;
				default:
					break;
				}
			} break;
			case SDL_EVENT_MOUSE_MOTION: {

			} break;
			case SDL_EVENT_WINDOW_RESIZED:
				screen_width = event->window.data1;
				screen_height = event->window.data2;
				glViewport(0, 0, screen_width, screen_height);
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		static auto now() -> float { return static_cast<float>(SDL_GetTicks()) / 1000.0f; }

		[[nodiscard]] SDL_AppResult update() noexcept {
			const auto current_time = now();
			const auto dt = current_time - time_start;
			time_start = current_time;

			glClear(GL_COLOR_BUFFER_BIT);

			shape->draw(*program);

			SDL_GL_SwapWindow(window);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, SDL_GLContext opengl_context)
				: window{window}, opengl_context(opengl_context), time_start{App::now()}, program{std::nullopt}, shape{} {
			initialize_video();

			//			shape = vis::create_rectangle_shape(glm::vec2{}, glm::vec2{0.5f, 0.5f});
			shape = vis::create_regular_shape(glm::vec2{}, 0.5f, glm::vec4{}, 50);
		}

		void initialize_video() {
			glViewport(0, 0, screen_width, screen_height);
			glClearColor(0.0f, 1.0f, 0.0f, 0.0f);

			program = vis::ProgramBuilder{}
										.add_shader(vis::Shader::create(GL_VERTEX_SHADER, R"(
#version 330 core
layout (location = 0) in vec2 pos;

void main()
{
    gl_Position = vec4(pos.x, pos.y, 0.0f, 1.0);
}
)"))
										.add_shader(vis::Shader::create(GL_FRAGMENT_SHADER, R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)"))
										.build();

			if (not program) {
				throw std::runtime_error{"Unable to initialize the video"};
			}
		}

		friend App* create() noexcept;

	private:
		SDL_Window* window = nullptr;
		SDL_GLContext opengl_context;

		int screen_width = SCREEN_WIDTH;
		int screen_height = SCREEN_HEIGHT;
		static constexpr SDL_WindowFlags screen_flags = SDL_WINDOW_OPENGL;

		const float border_thickness = 10.0f;
		const float pad_length = 100.0f;

		float time_start;
		std::optional<vis::Program> program{};
		std::optional<vis::GeometryShape> shape;
	};

	} // namespace Game
}
