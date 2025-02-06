
module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include <cmath>
#include <numbers>
#include <print>

export module Game;

export {
	namespace Game {

	template <typename T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template <arithmetic T> struct Rect {
		T x;
		T y;
		T width;
		T height;
	};

	template <arithmetic T, std::floating_point Color = float> struct FilledRectangle {
		Rect<T> shape;
		Color color[4];
	};

	enum class PongState { CONTINUE, FAILURE, TERMINATE_WITH_SUCCESS };

	struct color4 {
		float r, g, b, a;
	};

	struct RectangleShape {
		SDL_FRect shape;
		color4 color;
	};

	constexpr color4 red = {.r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f};
	constexpr color4 blue = {.r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f};

	class App {
	public:
		static App* create() noexcept {
			if (SDL_InitSubSystem(SDL_INIT_VIDEO) == false) {
				std::println("Unable to initialize SDL subsystems: {}", SDL_GetError());
				return nullptr;
			}

			SDL_Window* window = nullptr;
			SDL_Renderer* renderer = nullptr;

			if (SDL_CreateWindowAndRenderer("Hello SDL callbacks", App::screen_width, App::screen_height, App::screen_flags, &window, &renderer) == false) {
				std::println("Unable to create the window or the renderer: {}", SDL_GetError());
				return nullptr;
			}

			return new App(window, renderer);
		}

		[[nodiscard]] SDL_AppResult processEvent(const SDL_Event* event) noexcept {
			if (event->type == SDL_EVENT_QUIT) {
				return SDL_AppResult::SDL_APP_SUCCESS;
			}

			if (event->type == SDL_EVENT_KEY_DOWN) {
				switch (event->key.key) {
				case SDLK_ESCAPE:
					return SDL_AppResult::SDL_APP_SUCCESS;

				case SDLK_P:
					paused = !paused;
					break;

				default:
					break;
				}
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		[[nodiscard]] SDL_AppResult update() const noexcept {
			if (not paused) {
			}

			SDL_SetRenderDrawColorFloat(renderer, 1.0f, 1.0f, 1.0f, 1.0f);
			SDL_RenderClear(renderer);

			SDL_SetRenderDrawColorFloat(renderer, border_left.color.r, border_left.color.g, border_left.color.b, border_left.color.a);
			SDL_RenderFillRect(renderer, &border_left.shape);
			SDL_SetRenderDrawColorFloat(renderer, border_right.color.r, border_right.color.g, border_right.color.b, border_right.color.a);
			SDL_RenderFillRect(renderer, &border_right.shape);
			SDL_SetRenderDrawColorFloat(renderer, border_bottom.color.r, border_bottom.color.g, border_bottom.color.b, border_bottom.color.a);
			SDL_RenderFillRect(renderer, &border_bottom.shape);
			SDL_SetRenderDrawColorFloat(renderer, border_top.color.r, border_top.color.g, border_top.color.b, border_top.color.a);
			SDL_RenderFillRect(renderer, &border_top.shape);

			SDL_SetRenderDrawColorFloat(renderer, pad.color.r, pad.color.g, pad.color.b, pad.color.a);
			SDL_RenderFillRect(renderer, &pad.shape);

			SDL_RenderPresent(renderer);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, SDL_Renderer* renderer)
				: window{window}, renderer(renderer), border_top{.shape = {.x = 0, .y = 0, .w = screen_width, .h = border_thickness}, .color = red},
					border_bottom{.shape = {.x = 0, .y = screen_height - border_thickness, .w = screen_width, .h = border_thickness}, .color = red},
					border_left{.shape = {.x = 0, .y = 0, .w = border_thickness, .h = screen_height}, .color = red},
					border_right{.shape = {.x = screen_width - border_thickness, .y = 0, .w = border_thickness, .h = screen_height}, .color = red} {
			pad = {.shape = {.x = (screen_width - 2 * border_thickness - border_thickness) / 2.0f + border_thickness,
											 .y = (screen_height - 2 * border_thickness - border_thickness) / 2.0f + border_thickness,
											 .w = border_thickness,
											 .h = border_thickness},
						 .color = blue};
		}
		friend App* create();

	private:
		static constexpr int screen_width = 800;
		static constexpr int screen_height = 600;
		static constexpr SDL_WindowFlags screen_flags = 0;

		const float border_thickness = 10.0f;
		const RectangleShape border_left;
		const RectangleShape border_right;
		const RectangleShape border_top;
		const RectangleShape border_bottom;

		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;
		bool paused = false;

		RectangleShape pad;
	};

	} // namespace Game
}
