
module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <cmath>
#include <numbers>
#include <print>

export module Game;

export
{
	namespace Game {

	struct RectangleShape {
		float w, h;
		glm::vec4 color;
	};

	struct Position : public glm::vec2 {
		using glm::vec2::vec2;
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

			if (SDL_CreateWindowAndRenderer("Hello EnTT", App::screen_width, App::screen_height, App::screen_flags, &window, &renderer) == false) {
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
			SDL_SetRenderDrawColorFloat(renderer, 1.0f, 1.0f, 1.0f, 1.0f);
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

			constexpr auto red = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
			constexpr auto wall_color = red;

			entity_registry.emplace<RectangleShape>(left_wall, RectangleShape{.w = border_thickness, .h = screen_height, .color = wall_color});
			entity_registry.emplace<Position>(left_wall, Position{0.0f, 0.0f});

			entity_registry.emplace<RectangleShape>(right_wall, RectangleShape{.w = border_thickness, .h = screen_height, .color = wall_color});
			entity_registry.emplace<Position>(right_wall, Position{screen_width - border_thickness, 0.0f});

			entity_registry.emplace<RectangleShape>(top_wall, RectangleShape{.w = screen_width, .h = border_thickness, .color = wall_color});
			entity_registry.emplace<Position>(top_wall, Position{0.0f, 0.0f});

			entity_registry.emplace<RectangleShape>(bottom_wall, RectangleShape{.w = screen_width, .h = border_thickness, .color = wall_color});
			entity_registry.emplace<Position>(bottom_wall, Position{0.0f, screen_height - border_thickness});
		}

		void render_system() {
			const auto view = entity_registry.view<RectangleShape, Position>();
			view.each([&](auto& shape, auto& position) {
				SDL_SetRenderDrawColorFloat(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);
				SDL_FRect rect = {.x = position.x, .y = position.y, .w = shape.w, .h = shape.h};
				SDL_RenderFillRect(renderer, &rect);
			});
		}

		friend App* create();

	private
	:
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
