
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <print>

export module Game;

import std;
import vis;

export {
	namespace Game {

	struct Mesh : vis::mesh::Mesh {};

	// struct Transformation {
	// 	vis::mat4 model;
	// };

	// struct RigidBody : vis::physics::b2BodyId {};
	// struct RigidBody : vis::physics::RigidBodyDef {};

	// vis::mat4 to_mat4(const Transformation& t) {
	// 	auto model = vis::ext::identity<vis::mat4>();
	// 	model = vis::ext::translate(model, vis::vec3(t.position, 0.0f));
	// 	model = vis::ext::rotate(model, t.angle, vis::vec3(0.0f, 0.0f, 1.0f));
	// 	model = vis::ext::scale(model, vis::vec3(t.scale, 1.0f));
	// 	return model;
	// }

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

			static float previous_time = SDL_GetTicks() / 1000.0f;
			engine.clear();

			const auto t = SDL_GetTicks() / 1000.0f;
			const auto dt = t - previous_time;

			update_physic_system(t, dt);
			render_system();

			engine.render(window);

			previous_time = t;

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, vis::engine::Engine& engine)
				: window{window},
					engine(engine),
					program{std::nullopt},
					circle{vis::mesh::create_regular_shape(vis::vec2{}, 0.5f, vis::vec4{}, 50)} {
			initialize_video();
			initialize_physics();
			initialize_scene();
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
layout (location = 1) in vec4 col;

uniform mat4 model_view_projection;

out vec4 vertex_color;

void main()
{
    gl_Position = model_view_projection * vec4(pos.xy, 0.0f, 1.0f);
		vertex_color = col;
}
)"))
										.add_shader(vis::opengl::Shader::create(vis::opengl::ShaderType::fragment, R"(
#version 410 core

in vec4 vertex_color;
out vec4 fragment_color;

void main()
{
//    FragColor = vec4(0.0f, 0.5f, 0.2f, 1.0f);
		fragment_color = vertex_color;
}
)"))
										.build();

			if (not program) {
				throw std::runtime_error{"Unable to initialize the video"};
			}
		}

		void render_system() {
			const auto view = entity_registry.view<vis::mesh::Mesh, vis::physics::Transformation>();
			program->use();

			view.each([&](const auto& shape, const auto& transformation) {
				const vis::mat4 model_view = transformation.get_model();
				const auto model_view_projection = screen_proj.projection * model_view;
				program->set_uniform("model_view_projection", model_view_projection);
				shape.draw(*program);
				shape.unbind();
			});
		}

		void update_physic_system(float t, float dt) {
			static float accumulated_time = 0.0f;
			static constexpr float fixed_time_step = 1 / 30.0f;

			accumulated_time += dt;

			while (accumulated_time >= fixed_time_step) {
				world->step(fixed_time_step, 4);
				accumulated_time -= fixed_time_step;
			}

			const auto view = entity_registry.view<vis::physics::Transformation, vis::physics::RigidBody>();
			view.each([&](auto& tr, const auto& body) { tr = body.get_transform(); });
		}

		void initialize_physics() {
			auto world_def = vis::physics::WorldDef();
			world_def.set_gravity(vis::vec2{0.0f, -9.81f});
			world = vis::physics::create_world(world_def);
		}

		void initialize_scene() {
			constexpr auto wall_thickness = 0.6f;
			constexpr auto half_wall_thickness = wall_thickness / 2.0f;
			const auto half_screen_extent = screen_proj.half_world_extent;
			constexpr auto x_offset = vis::vec2{half_wall_thickness, 0.0f};
			constexpr auto y_offset = vis::vec2{0.0f, half_wall_thickness};
			const auto left_pos = vis::vec2{-half_screen_extent.x, 0.0f} + x_offset;
			const auto right_pos = vis::vec2{+half_screen_extent.x, 0.0f} - x_offset;
			const auto top_pos = vis::vec2{0.0f, +half_screen_extent.y} - y_offset;
			const auto bottom_pos = vis::vec2{0.0f, -half_screen_extent.y} + y_offset;
			const auto vertical_half_extent = vis::vec2{half_screen_extent.x, half_wall_thickness};
			const auto horizontal_half_extent = vis::vec2{half_wall_thickness, half_screen_extent.y};

			constexpr auto ball_color = vis::vec4{1.0f, 0.0f, 0.0f, 1.0f};
			constexpr auto wall_color = vis::vec4{0.0f, 0.5f, 0.2f, 1.0f};

			add_wall(horizontal_half_extent, left_pos, wall_color);
			add_wall(horizontal_half_extent, right_pos, wall_color);
			add_wall(vertical_half_extent, top_pos, wall_color);
			add_wall(vertical_half_extent, bottom_pos, wall_color);

			add_ball(0.6, vis::vec2{10.0f, 0.0f}, ball_color);
		}

		void add_wall(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto wall = entity_registry.create();
			entity_registry.emplace<vis::mesh::Mesh>(wall, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto& transform = entity_registry.emplace<vis::physics::Transformation>(wall, vis::physics::Transformation{
																																												.position = pos,
																																										});
			vis::physics::RigidBodyDef body_def;
			body_def.set_position(transform.position).set_body_type(vis::physics::BodyType::fixed);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(wall, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			vis::physics::ShapeDef wall_shape;
			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_ball(float radius, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto ball = entity_registry.create();
			entity_registry.emplace<vis::mesh::Mesh>(ball, vis::mesh::create_regular_shape(origin, radius, color, 20));

			auto& transform = entity_registry.emplace<vis::physics::Transformation>(ball, vis::physics::Transformation{
																																												.position = origin,
																																										});
			auto circle = vis::physics::Circle{
					.center = origin,
					.radius = radius,
			};

			vis::physics::RigidBodyDef body_def;
			body_def.set_position(transform.position);
			body_def.set_body_type(vis::physics::BodyType::dynamic);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(ball, vis::physics::RigidBody{
																																										world->create_body(body_def),
																																								});

			vis::physics::ShapeDef shape_def;
			shape_def.set_restitution(0.7);
			rigid_body.create_shape(shape_def, circle);
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

		std::optional<vis::physics::World> world;
	};

	} // namespace Game
}
