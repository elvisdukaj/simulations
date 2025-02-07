
module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <ranges>
#include <cmath>
#include <numbers>
#include <print>

export module Game;

export
{
	namespace Game {

	struct GeometryShape {
		std::vector<SDL_Vertex> vertices;
		std::vector<int> indices;
	};

	struct RegularShape : public GeometryShape {
		RegularShape(SDL_FPoint center, float radius, const SDL_FColor& color, int num_vertices = 6) {
			const float theta_step = 2 * std::numbers::pi / num_vertices;

			vertices.reserve(num_vertices + 1); // plus one for the center
			vertices.emplace_back(
					SDL_Vertex{
							.position = center,
							.color = color}
					);
			for (int i = 0; i != num_vertices; i++) {
				vertices.emplace_back(
						SDL_Vertex{
								.position = SDL_FPoint{
										.x = std::cos(theta_step * i) * radius + center.x,
										.y = std::sin(theta_step * i) * radius + center.y
								},
								.color = color
						});
			}

			for (int i = 0; i != num_vertices; ++i) {
				const auto mid_vertex = (i + 2);
				const auto mid_vertex_round = mid_vertex % num_vertices;

				indices.push_back(i + 1);
				indices.push_back(mid_vertex_round == 0 ? mid_vertex : mid_vertex_round);
				indices.push_back(0);
			}
		}
	};

	struct RectangleShape : public GeometryShape {
		RectangleShape(const SDL_FPoint& top_left, const SDL_FPoint& bottom_right, const SDL_FColor& color) {
			vertices.reserve(4);
			// left-top corner
			vertices.emplace_back(
					SDL_Vertex{
							.position = top_left,
							.color = color
					}
					);
			// right-top corner
			vertices.emplace_back(
					SDL_Vertex{
							.position = {.x = bottom_right.x, .y = top_left.y},
							.color = color
					});
			// bottom-right corner
			vertices.emplace_back(
					SDL_Vertex{
							.position = bottom_right,
							.color = color
					});
			// bottom-left corner
			vertices.emplace_back(
					SDL_Vertex{
							.position = {.x = top_left.x, .y = bottom_right.y},
							.color = color
					});

			indices = std::vector<int>{0, 1, 2, 2, 3, 0};
		}
	};

	class App {
	public:
		static App* create() noexcept {
			if (SDL_InitSubSystem(SDL_INIT_VIDEO) == false) {
				std::println("Unable to initialize SDL subsystems: {}", SDL_GetError());
				return nullptr;
			}

			SDL_Window* window = nullptr;
			SDL_Renderer* renderer = nullptr;

			if (SDL_CreateWindowAndRenderer(
					    "Hello Geometry", App::screen_width, App::screen_height, App::screen_flags, &window,
					    &renderer) == false) {
				std::println("Unable to create the window or the renderer: {}", SDL_GetError());
				return nullptr;
			}

			return new App(window, renderer);
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
			}
			break;
			case SDL_EVENT_MOUSE_MOTION: {

			}
			break;
			default:
				break;
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		[[nodiscard]] SDL_AppResult update() noexcept {
			SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 1.0f);
			SDL_RenderClear(renderer);

			render_system();

			SDL_RenderPresent(renderer);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, SDL_Renderer* renderer)
			: window{window},
			  renderer(renderer) {
			const auto left_wall = entity_registry.create();
			const auto right_wall = entity_registry.create();
			const auto top_wall = entity_registry.create();
			const auto bottom_wall = entity_registry.create();

			const auto pentagon = entity_registry.create();
			const auto exaagon = entity_registry.create();

			constexpr auto red = SDL_FColor{.r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f};
			constexpr auto yellow = SDL_FColor{.r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f};

			constexpr auto wall_color = red;

			entity_registry.emplace<GeometryShape>(
					left_wall,
					RectangleShape{
							SDL_FPoint{.x = 0.0f, .y = 0.0f},
							SDL_FPoint{.x = border_thickness, .y = screen_height},
							wall_color
					});
			entity_registry.emplace<GeometryShape>(
					right_wall,
					RectangleShape{
							SDL_FPoint{.x = screen_width - border_thickness, .y = 0.0f},
							SDL_FPoint{.x = screen_width, .y = screen_height},
							wall_color
					});
			entity_registry.emplace<GeometryShape>(
					top_wall,
					RectangleShape{
							SDL_FPoint{.x = 0.0f, .y = 0.0f},
							SDL_FPoint{.x = screen_width, .y = border_thickness},
							wall_color
					});
			entity_registry.emplace<GeometryShape>(
					bottom_wall,
					RectangleShape{
							SDL_FPoint{.x = 0.0f, .y = screen_height - border_thickness},
							SDL_FPoint{.x = screen_width, .y = screen_height},
							wall_color
					});

			entity_registry.emplace<GeometryShape>(
					pentagon,
					RegularShape{
							SDL_FPoint{
									.x = screen_width / 4.0, .y = screen_height / 4.0},
							100.0f,
							yellow, 50
					});
		}

		void render_system() {
			const auto view = entity_registry.view<GeometryShape>();
			view.each([&](const GeometryShape& shape) {
				const auto& vertices = shape.vertices;
				const auto& indices = shape.indices;
				SDL_RenderGeometry(
						renderer, nullptr,
						vertices.data(), static_cast<int>(vertices.size()),
						indices.data(), static_cast<int>(indices.size()));
			});
		}

		friend App* create() noexcept;

	private:
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;

		static constexpr int screen_width = 800;
		static constexpr int screen_height = 600;
		static constexpr SDL_WindowFlags screen_flags = 0;

		const float border_thickness = 10.0f;
		const float pad_length = 100.0f;

		entt::registry entity_registry;
	};

	} // namespace Game
}
