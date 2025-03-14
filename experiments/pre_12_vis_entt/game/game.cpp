
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

export module Game;

import std;
import vis;

export {
	namespace Game {

	struct Mesh : vis::mesh::Mesh {};

	struct Position : public vis::vec2 {
		using vis::vec2::vec2;
	};

	constexpr int SCREEN_HEIGHT = 600;
	constexpr std::ratio<4, 3> ASPECT_RATIO;
	constexpr int SCREEN_WIDTH = 800; // SCREEN_HEIGHT * ASPECT_RATIO.num / ASPECT_RATIO.den;

	class App {
	public:
		static App* create() {
			static SDL_Window* window = SDL_CreateWindow("Hello OpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, screen_flags);

			if (not window) {
				throw std::runtime_error(std::format("Unable to create the window: {}", SDL_GetError()));
			}

			static auto engine = vis::engine::create(window);
			return new App{window, engine};
		}

		~App() {
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
				engine.set_viewport(0, 0, screen_width, screen_height);
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		static auto now() -> float {
			return static_cast<float>(SDL_GetTicks()) / 1000.0f;
		}

		[[nodiscard]] SDL_AppResult update() noexcept {
			engine.clear();

			render_system();

			engine.render(window);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, vis::engine::Engine& engine)
				: window{window},
					engine(engine),
					program{std::nullopt},
					circle{vis::mesh::create_regular_shape(vis::vec2{}, 0.5f, vis::vec4{}, 50)} {

			initialize_video();
		}

		void initialize_video() {
			engine.print_info();
			engine.set_clear_color(vis::vec4(1.0f, 0.5f, 0.5f, 1.0f));
			engine.set_viewport(0, 0, screen_width, screen_height);

			program = vis::opengl::ProgramBuilder{}
										.add_shader(vis::opengl::Shader::create(vis::opengl::ShaderType::vertex, R"(
#version 410 core
layout (location = 0) in vec2 pos;

void main()
{
    gl_Position = vec4(pos.x, pos.y, 0.0f, 1.0);
}
)"))
										.add_shader(vis::opengl::Shader::create(vis::opengl::ShaderType::fragment, R"(
#version 410 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0f, 0.5f, 0.2f, 1.0f);
}
)"))
										.build();

			if (not program) {
				throw std::runtime_error{"Unable to initialize the video"};
			}

			const auto left_wall = entity_registry.create();
			const auto right_wall = entity_registry.create();
			const auto top_wall = entity_registry.create();
			const auto bottom_wall = entity_registry.create();

			constexpr auto red = vis::vec4{1.0f, 0.0f, 0.0f, 1.0f};
			constexpr auto wall_color = red;

			const float screen_width = 1.0f;
			const float screen_height = 1.0f;

			entity_registry.emplace<vis::mesh::Mesh>(
					left_wall, vis::mesh::create_rectangle_shape(vis::vec2{0.0f, 0.0f}, vis::vec2{0.4f, 0.5f}));
			entity_registry.emplace<Position>(left_wall, Position{0.0f, 0.0f});
		}

		void render_system() {
			const auto view = entity_registry.view<vis::mesh::Mesh, Position>();
			view.each([&](auto& shape, auto& position) {
				program->use();
				shape.draw(*program);
				shape.unbind();
			});
		}

	private:
		const float border_thickness = 0.1f;

		SDL_Window* window = nullptr;
		vis::engine::Engine& engine;

		int screen_width = SCREEN_WIDTH;
		int screen_height = SCREEN_HEIGHT;
		static constexpr SDL_WindowFlags screen_flags = SDL_WINDOW_OPENGL;

		std::optional<vis::opengl::Program> program{};
		vis::registry entity_registry;
		vis::mesh::Mesh circle;
	};

	} // namespace Game
}
