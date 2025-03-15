
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

	struct Transformation {
		vis::vec2 position = vis::vec2{0.0f, 0.0f};
		vis::vec2 scale = vis::vec2{1.0f, 1.0f};
		float angle = 0.0f;
	};

	struct RigidBody : vis::physics::b2BodyId {};

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

		void update_physic_system(float t, float dt) {
			std::println("t = {}, dt = {}", t, dt);
			static float accumulated_time = 0.0f;
			static constexpr float fixed_time_step = 1 / 30.0f;

			accumulated_time += dt;

			while (accumulated_time >= fixed_time_step) {
				vis::physics::b2World_Step(world, fixed_time_step, 4);
				accumulated_time -= fixed_time_step;
			}

			const auto view = entity_registry.view<Transformation, RigidBody>();
			view.each([&](auto& tr, const auto& body) {
				auto b2tr = vis::physics::b2Body_GetTransform(body);
				tr.position = vis::vec2{b2tr.p.x, b2tr.p.y};
				std::println("body {}, pos: {{{}.{}}}", body.index1, tr.position.x, tr.position.y);
			});
		}

		void initialize_physics() {
			auto world_def = vis::physics::b2DefaultWorldDef();
			world_def.gravity = vis::physics::b2Vec2{0.0f, -9.81f};
			world = vis::physics::b2CreateWorld(&world_def);
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
			const auto ball = entity_registry.create();

			constexpr auto red = vis::vec4{1.0f, 0.0f, 0.0f, 1.0f};
			constexpr auto wall_color = vis::vec4{0.0f, 0.5f, 0.2f, 1.0f};
			{
				entity_registry.emplace<vis::mesh::Mesh>(
						left_wall, vis::mesh::create_rectangle_shape(origin, horizontal_half_extent, wall_color));
				auto& transform = entity_registry.emplace<Transformation>(left_wall, Transformation{
																																								 .position = left_pos,
																																						 });
				auto body_def = vis::physics::b2DefaultBodyDef();
				body_def.position = vis::physics::b2Vec2{transform.position.x, transform.position.y};
				body_def.type = vis::physics::b2BodyType::b2_staticBody;
				auto& rigid_body =
						entity_registry.emplace<RigidBody>(left_wall, RigidBody{
																															vis::physics::b2CreateBody(world, &body_def),
																													});

				auto wall_box = vis::physics::b2MakeBox(horizontal_half_extent.x, horizontal_half_extent.y);
				auto wall_shape = vis::physics::b2DefaultShapeDef();
				vis::physics::b2CreatePolygonShape(rigid_body, &wall_shape, &wall_box);
			}

			{
				entity_registry.emplace<vis::mesh::Mesh>(
						right_wall, vis::mesh::create_rectangle_shape(origin, horizontal_half_extent, wall_color));
				auto& transform = entity_registry.emplace<Transformation>(right_wall, Transformation{
																																									.position = right_pos,
																																							});
				auto body_def = vis::physics::b2DefaultBodyDef();
				body_def.position = vis::physics::b2Vec2{transform.position.x, transform.position.y};
				body_def.type = vis::physics::b2BodyType::b2_staticBody;
				auto& rigid_body =
						entity_registry.emplace<RigidBody>(right_wall, RigidBody{
																															 vis::physics::b2CreateBody(world, &body_def),
																													 });

				auto wall_box = vis::physics::b2MakeBox(horizontal_half_extent.x, horizontal_half_extent.y);
				auto wall_shape = vis::physics::b2DefaultShapeDef();
				vis::physics::b2CreatePolygonShape(rigid_body, &wall_shape, &wall_box);
			}

			{
				entity_registry.emplace<vis::mesh::Mesh>(
						top_wall, vis::mesh::create_rectangle_shape(origin, vertical_half_extent, wall_color));
				auto& transform = entity_registry.emplace<Transformation>(top_wall, Transformation{
																																								.position = top_pos,
																																						});
				auto body_def = vis::physics::b2DefaultBodyDef();
				body_def.position = vis::physics::b2Vec2{transform.position.x, transform.position.y};
				body_def.type = vis::physics::b2BodyType::b2_staticBody;
				auto& rigid_body =
						entity_registry.emplace<RigidBody>(top_wall, RigidBody{
																														 vis::physics::b2CreateBody(world, &body_def),
																												 });

				auto wall_box = vis::physics::b2MakeBox(vertical_half_extent.x, vertical_half_extent.y);
				auto wall_shape = vis::physics::b2DefaultShapeDef();
				vis::physics::b2CreatePolygonShape(rigid_body, &wall_shape, &wall_box);
			}

			{
				entity_registry.emplace<vis::mesh::Mesh>(
						bottom_wall, vis::mesh::create_rectangle_shape(origin, vertical_half_extent, wall_color));
				auto& transform = entity_registry.emplace<Transformation>(bottom_wall, Transformation{
																																									 .position = bottom_pos,
																																							 });
				auto body_def = vis::physics::b2DefaultBodyDef();
				body_def.position = vis::physics::b2Vec2{transform.position.x, transform.position.y};
				body_def.type = vis::physics::b2BodyType::b2_staticBody;
				auto& rigid_body =
						entity_registry.emplace<RigidBody>(bottom_wall, RigidBody{
																																vis::physics::b2CreateBody(world, &body_def),
																														});

				auto wall_box = vis::physics::b2MakeBox(vertical_half_extent.x, vertical_half_extent.y);
				auto wall_shape = vis::physics::b2DefaultShapeDef();
				vis::physics::b2CreatePolygonShape(rigid_body, &wall_shape, &wall_box);
			}

			{
				constexpr float radius = 1.0f;
				entity_registry.emplace<vis::mesh::Mesh>(ball, vis::mesh::create_regular_shape(origin, radius, red, 20));
				auto& transform = entity_registry.emplace<Transformation>(ball, Transformation{
																																						.position = origin,
																																				});
				auto circle = vis::physics::b2Circle{
						.center = {origin.x, origin.y},
						.radius = radius,
				};
				auto body_def = vis::physics::b2DefaultBodyDef();
				body_def.isBullet = true;
				body_def.position = vis::physics::b2Vec2{transform.position.x, transform.position.y};
				body_def.type = vis::physics::b2BodyType::b2_dynamicBody;
				auto& rigid_body = entity_registry.emplace<RigidBody>(ball, RigidBody{
																																				vis::physics::b2CreateBody(world, &body_def),
																																		});

				auto shape_def = vis::physics::b2DefaultShapeDef();
				shape_def.restitution = .5f;
				auto shape = vis::physics::b2CreateCircleShape(rigid_body, &shape_def, &circle);

				vis::physics::b2Body_ApplyForce(rigid_body, {.x = 450.0f, .y = 450.0f},
																				vis::physics::b2Body_GetLocalCenterOfMass(rigid_body), true);
			}
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

		vis::physics::b2WorldId world;
	};

	} // namespace Game
}
