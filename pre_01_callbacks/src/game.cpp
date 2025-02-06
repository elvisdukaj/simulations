
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

	enum class PongState { CONTINUE, FAILURE, TERMINATE_WITH_SUCCESS };

	class App {
	public:
		static App* create() noexcept {
			if (SDL_InitSubSystem(SDL_INIT_VIDEO) == false) {
				std::println("Unable to initialize SDL subsystems: {}", SDL_GetError());
				return nullptr;
			}

			SDL_Window* window = nullptr;
			SDL_Renderer* renderer = nullptr;

			if (SDL_CreateWindowAndRenderer("Hello SDL callbacks", 800, 600, 0, &window, &renderer) == false) {
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
			const double now = static_cast<double>(SDL_GetTicks()) / 1000.0; /* convert from milliseconds to seconds. */

			constexpr auto pi = std::numbers::pi_v<double>;

			if (not paused) {
				/* choose the color for the frame we will draw. The sine wave trick makes it
				 * fade between colors smoothly. */
				const auto red = static_cast<float>(0.5 + 0.5 * std::sin(now));
				const auto green = static_cast<float>(0.5 + 0.5 * std::sin(now + pi * 2 / 3));
				const auto blue = static_cast<float>(0.5 + 0.5 * std::sin(now + pi * 4 / 3));
				SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */
			}

			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, SDL_Renderer* renderer) : window{window}, renderer(renderer) {}
		friend App* create();

	private:
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;
		bool paused = false;
	};

	} // namespace Game
}
