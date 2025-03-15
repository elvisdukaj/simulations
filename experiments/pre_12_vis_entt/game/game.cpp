
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

export module Game;

import std;
import vis;

export {
	namespace Game {

	struct Mesh : vis::mesh::Mesh {};

	struct Position : vis::vec2 {
		using vis::vec2::vec2;
	};

	struct Transformation {
		vis::vec2 position = vis::vec2{0.0f, 0.0f};
		vis::vec2 scale = vis::vec2{1.0f, 1.0f};
		float angle = 0.0f;
	};

	vis::mat4 to_mat4(const Transformation& t) {
		auto model = vis::ext::identity<vis::mat4>();
		model = vis::ext::translate(model, vis::vec3(t.position, 0.0f));
		model = vis::ext::rotate(model, t.angle, vis::vec3(0.0f, 0.0f, 1.0f));
		model = vis::ext::scale(model, vis::vec3(t.scale, 1.0f));
		return model;
	}

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
				screen_proj = vis::orthogonal_matrix(screen_width, screen_height, 20.0f, 20.0f);
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		static auto now() -> float {
			return static_cast<float>(SDL_GetTicks()) / 1000.0f;
		}

		[[nodiscard]] SDL_AppResult update() noexcept {
			engine.clear();

			const auto t = SDL_GetTicks() / 1000.0f;
			update_physic_system(t);
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
			screen_proj = vis::orthogonal_matrix(screen_width, screen_height, 20.0f, 20.0f);

			// TODO: load this from file or even better, build using SPRIV during compilation time
			program = vis::opengl::ProgramBuilder{}
										.add_shader(vis::opengl::Shader::create(vis::opengl::ShaderType::vertex, R"(
#version 410 core
layout (location = 0) in vec2 pos;

uniform mat4 model_view_projection;

void main()
{
    gl_Position = model_view_projection * vec4(pos.xy, 0.0f, 1.0f);
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

			initialize_scene();
		}

		void render_system() {
			const auto view = entity_registry.view<vis::mesh::Mesh, Transformation>();
			program->use();

			view.each([&](const auto& shape, const auto& transformation) {
				const vis::mat4 model_view = to_mat4(transformation);
				const auto model_view_projection = screen_proj.projection * model_view;
				program->set_uniform("model_view_projection", model_view_projection);
				shape.draw(*program);
				shape.unbind();
			});
		}

		void update_physic_system(float t) {
			// const auto view = entity_registry.view<Transformation>();
			// view.each([&](auto& tr) {
			// tr.position = pos;
			// tr.scale = scale;
			// tr.angle = angle;
			// });
		}

		void initialize_scene() {
			constexpr auto wall_thickness = 0.6f;
			constexpr auto half_wall_thickness = wall_thickness / 2.0f;
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			const auto half_screen_extent = screen_proj.half_world_extent;
			constexpr auto x_offset = vis::vec2{half_wall_thickness, 0.0f};
			constexpr auto y_offset = vis::vec2{0.0f, half_wall_thickness};
			const auto left_pos = vis::vec2{-half_screen_extent.x, 0.0f} + x_offset;
			const auto right_pos = vis::vec2{+half_screen_extent.x, 0.0f} - x_offset;
			const auto top_pos = vis::vec2{0.0f, +half_screen_extent.y} - y_offset;
			const auto bottom_pos = vis::vec2{0.0f, -half_screen_extent.y} + y_offset;
			const auto vertical_half_extent = vis::vec2{half_screen_extent.x, half_wall_thickness};
			const auto horizontal_half_extent = vis::vec2{half_wall_thickness, half_screen_extent.y};

			const auto left_wall = entity_registry.create();
			const auto right_wall = entity_registry.create();
			const auto top_wall = entity_registry.create();
			const auto bottom_wall = entity_registry.create();

			constexpr auto red = vis::vec4{1.0f, 0.0f, 0.0f, 1.0f};
			constexpr auto wall_color = red;

			entity_registry.emplace<vis::mesh::Mesh>(left_wall,
																							 vis::mesh::create_rectangle_shape(origin, horizontal_half_extent));
			entity_registry.emplace<Transformation>(left_wall, Transformation{
																														 .position = left_pos,
																												 });

			entity_registry.emplace<vis::mesh::Mesh>(right_wall,
																							 vis::mesh::create_rectangle_shape(origin, horizontal_half_extent));
			entity_registry.emplace<Transformation>(right_wall, Transformation{
																															.position = right_pos,
																													});

			entity_registry.emplace<vis::mesh::Mesh>(top_wall,
																							 vis::mesh::create_rectangle_shape(origin, vertical_half_extent));
			entity_registry.emplace<Transformation>(top_wall, Transformation{
																														.position = top_pos,
																												});

			entity_registry.emplace<vis::mesh::Mesh>(bottom_wall,
																							 vis::mesh::create_rectangle_shape(origin, vertical_half_extent));
			entity_registry.emplace<Transformation>(bottom_wall, Transformation{
																															 .position = bottom_pos,
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
		vis::ScreenProjection screen_proj;
	};

	} // namespace Game
}
