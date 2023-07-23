#include <donut/events/Error.hpp>
#include <donut/events/MessageBox.hpp>

#include <SDL.h>  // SDL..., Uint32
#include <format> // std::format

namespace donut::events {

void MessageBox::show(Type type, const char* title, const char* message) {
	Uint32 flags = 0;
	switch (type) {
		case Type::ERROR_MESSAGE: flags = SDL_MESSAGEBOX_ERROR; break;
		case Type::WARNING_MESSAGE: flags = SDL_MESSAGEBOX_WARNING; break;
		case Type::INFO_MESSAGE: flags = SDL_MESSAGEBOX_INFORMATION; break;
	}
	if (SDL_ShowSimpleMessageBox(flags, title, message, nullptr) != 0) {
		throw Error{std::format("Failed to show simple message box: {}", SDL_GetError())};
	}
}

} // namespace donut::events
