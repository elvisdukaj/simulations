
module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <cmath>
#include <numbers>
#include <print>

export module Game;

export {
	namespace Game {

	constexpr int SCREEN_WIDTH = 800;
	constexpr int SCREEN_HEIGHT = 600;

	// Function to convert from your coordinate system to Box2D's system
	b2Vec2 to_box2d(const glm::vec2& v) {
		return b2Vec2{v.x / 100.0f, (SCREEN_HEIGHT - v.y) / 100.0f};
		;
	}
	glm::vec2 from_box2d(const b2Vec2& v) { return glm::vec2{v.x * 100.0f, (SCREEN_HEIGHT - v.y) * 100.0f}; }

	struct GeometryShape {
		explicit GeometryShape(glm::vec2 pos = glm::vec2{0.0f, 0.0f}) : pos{pos} {}
		glm::vec2 pos{};
		std::vector<SDL_Vertex> vertices{};
		std::vector<int> indices{};
	};

	SDL_FPoint to_sdl_fpoint(glm::vec2 v) { return SDL_FPoint{v.x, v.y}; }

	GeometryShape create_regular_shape(glm::vec2 center, float radius, const SDL_FColor& color, int num_vertices = 6) {
		GeometryShape shape{center};
		const float theta_step = 2.0f * std::numbers::pi_v<float> / (float)num_vertices;

		shape.vertices.reserve(num_vertices + 1); // plus one for the center
		shape.vertices.emplace_back(SDL_Vertex{.position = to_sdl_fpoint(center), .color = color});
		for (int i = 0; i != num_vertices; i++) {
			shape.vertices.emplace_back(
					SDL_Vertex{.position = SDL_FPoint{.x = std::cos(theta_step * (float)i) * radius + center.x,
																						.y = std::sin(theta_step * (float)i) * radius + center.y},
										 .color = color});
		}

		for (int i = 0; i != num_vertices; ++i) {
			const auto mid_vertex = (i + 2);
			const auto mid_vertex_round = mid_vertex % num_vertices;

			shape.indices.push_back(i + 1);
			shape.indices.push_back(mid_vertex_round == 0 ? mid_vertex : mid_vertex_round);
			shape.indices.push_back(0);
		}
		return shape;
	}

	GeometryShape create_rectangle_shape(const glm::vec2& center, const glm::vec2& half_extent, const SDL_FColor& color) {
		GeometryShape shape{center};

		glm::vec2 up = glm::vec2(0.0f, -half_extent.y);
		glm::vec2 down = glm::vec2(0.0f, half_extent.y);
		glm::vec2 left = glm::vec2(-half_extent.x, 0.0f);
		glm::vec2 right = glm::vec2(half_extent.x, 0.0f);

		shape.vertices.reserve(4);
		shape.vertices.emplace_back(SDL_Vertex{.position = to_sdl_fpoint(center + up + left), .color = color});
		shape.vertices.emplace_back(SDL_Vertex{.position = to_sdl_fpoint(center + up + right), .color = color});
		shape.vertices.emplace_back(SDL_Vertex{.position = to_sdl_fpoint(center + down + right), .color = color});
		shape.vertices.emplace_back(SDL_Vertex{.position = to_sdl_fpoint(center + down + left), .color = color});

		shape.indices = std::vector<int>{0, 1, 2, 2, 3, 0};
		return shape;
	}

	struct RigidBody {
		b2BodyId body{};
	};

	class App {
	public:
		static App* create() noexcept {
			if (not SDL_InitSubSystem(SDL_INIT_VIDEO)) {
				std::println("Unable to initialize SDL subsystems: {}", SDL_GetError());
				return nullptr;
			}

			SDL_Window* window = nullptr;
			SDL_Renderer* renderer = nullptr;

			if (not SDL_CreateWindowAndRenderer("Hello Geometry", App::screen_width, App::screen_height, App::screen_flags,
																					&window, &renderer)) {
				std::println("Unable to create the window or the renderer: {}", SDL_GetError());
				return nullptr;
			}

			if (not SDL_SetRenderVSync(renderer, 1)) {
				std::println("It's not possible to set the vsync");
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				return nullptr;
			}

			int vsync;
			if (not SDL_GetRenderVSync(renderer, &vsync)) {
				std::println("Unable to get the vsync!");
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
			}

			if (vsync != 1) {
				std::println("Unable to get the vsync!");
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
			}

			std::println("Setting vsync with value of: {}", vsync);

			return new App(window, renderer);
		}

		~App() {
			b2DestroyWorld(world);
			SDL_DestroyRenderer(renderer);
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
			default:
				break;
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		static auto now() -> float { return static_cast<float>(SDL_GetTicks()) / 1000.0f; }

		[[nodiscard]] SDL_AppResult update() noexcept {
			const auto current_time = now();
			const auto dt = current_time - time_start;
			time_start = current_time;

			SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 1.0f);
			SDL_RenderClear(renderer);

			physic_system(dt);
			render_system();

			SDL_RenderPresent(renderer);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, SDL_Renderer* renderer)
				: window{window}, renderer(renderer), time_start{App::now()}, world{} {
			auto worldDef = b2DefaultWorldDef();
			worldDef.gravity = b2Vec2(0.0f, -10.0f);
			world = b2CreateWorld(&worldDef);

			create_ball_entity();
			create_ground_entity();
		}

		void physic_system(float dt) {
			static float accumulated_time = 0.0f;
			static const float fixed_time_step = 1 / 30.0f;

			accumulated_time += dt;

			while (accumulated_time >= fixed_time_step) {
				b2World_Step(world, fixed_time_step, 4);
				std::println("dt: {}, accumulated_time: {}", dt, accumulated_time);
				accumulated_time -= fixed_time_step;
			}

			const auto view = entity_registry.view<RigidBody, GeometryShape>();
			view.each([&](const RigidBody& rb, GeometryShape& shape) {
				auto rb_pos = from_box2d(b2Body_GetPosition(rb.body));
				auto ds = rb_pos - shape.pos;
				shape.pos = rb_pos;
				for (auto& v : shape.vertices) {
					v.position.x += ds.x;
					v.position.y += ds.y;
				}
			});
		}

		void render_system() {
			const auto view = entity_registry.view<GeometryShape>();
			view.each([&](const GeometryShape& shape) {
				const auto& vertices = shape.vertices;
				const auto& indices = shape.indices;
				SDL_RenderGeometry(renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), indices.data(),
													 static_cast<int>(indices.size()));
			});
		}

		void create_ball_entity() {
			constexpr auto yellow = SDL_FColor{.r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f};

			const auto ball = entity_registry.create();
			float ball_radius = 30.0f;
			glm::vec2 center = glm::vec2{screen_width / 2.0f, ball_radius};

			auto ball_rb_def = b2DefaultBodyDef();
			ball_rb_def.position = to_box2d(center);
			ball_rb_def.type = b2_dynamicBody;

			entity_registry.emplace<GeometryShape>(ball, create_regular_shape(center, ball_radius, yellow, 50));

			auto& ball_rigid_body =
					entity_registry.emplace<RigidBody>(ball, RigidBody{.body = b2CreateBody(world, &ball_rb_def)});

			auto ball_shape_polygon = b2MakeBox(ball_radius / 100.0f, ball_radius / 100.0f);
			auto ball_shape_def = b2DefaultShapeDef();
			ball_shape_def.density = 1.0f;
			ball_shape_def.friction = 0.3f;
			ball_shape_def.restitution = .7f;
			b2CreatePolygonShape(ball_rigid_body.body, &ball_shape_def, &ball_shape_polygon);
		}

		void create_ground_entity() {
			constexpr auto red = SDL_FColor{.r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f};

			const auto wall_center = glm::vec2{screen_width / 2.0f, screen_height - border_thickness};
			const auto wall_half_extent = glm::vec2{screen_width / 2.0f, border_thickness / 2.0f};

			auto ground = entity_registry.create();
			entity_registry.emplace<GeometryShape>(ground, create_rectangle_shape(wall_center, wall_half_extent, red));
			b2BodyDef wall_rb_def = b2DefaultBodyDef();
			wall_rb_def.position = to_box2d(wall_center);
			auto& wall_rigid_body =
					entity_registry.emplace<RigidBody>(ground, RigidBody{.body = b2CreateBody(world, &wall_rb_def)});

			auto wall_box = b2MakeBox(wall_half_extent.x / 100.0f, wall_half_extent.y / 100.0f);
			auto wall_shape = b2DefaultShapeDef();
			b2CreatePolygonShape(wall_rigid_body.body, &wall_shape, &wall_box);
		}

		friend App* create() noexcept;

	private:
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;

		static constexpr int screen_width = SCREEN_WIDTH;
		static constexpr int screen_height = SCREEN_HEIGHT;
		static constexpr SDL_WindowFlags screen_flags = 0;

		const float border_thickness = 10.0f;
		const float pad_length = 100.0f;

		float time_start;

		b2WorldId world;
		entt::registry entity_registry;
	};

	} // namespace Game
}
