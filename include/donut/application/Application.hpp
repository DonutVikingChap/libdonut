#ifndef DONUT_APPLICATION_APPLICATION_HPP
#define DONUT_APPLICATION_APPLICATION_HPP

#include <donut/Time.hpp>

#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint64_t
#include <glm/glm.hpp> // glm::...
#include <string>      // std::string

namespace donut {
namespace application {

struct Event; // Forward declaration, to avoid including Event.hpp.

/**
 * Transient information about the current tick of an Application.
 */
struct TickInfo {
	/**
	 * Number of ticks that have been fully processed since the start of the
	 * application.
	 */
	std::size_t processedTickCount;

	/**
	 * The accumulated time, in seconds, of all ticks that had been processed
	 * since the start of the application at the beginning of the current tick.
	 */
	Time<float> processedTickTime;

	/**
	 * The average time, in seconds, that should elapse between each tick.
	 *
	 * This is calculated as the reciprocal of the desired application tick
	 * rate, i.e. tickInterval = 1 / ApplicationOptions::tickRate.
	 *
	 * The tick interval should be used as the time delta when updating
	 * physics, timers, etc. within a tick. This will ensure a fixed update
	 * interval, which generally results in more stable, predictable and
	 * consistent behavior compared to using a variable update interval,
	 * especially with regard to floating-point errors and numerical integration
	 * methods which may produce different results depending on the step size.
	 *
	 * To achieve a higher perceived update rate for the user, some form of
	 * interpolation and/or extrapolation should be used in the variable-rate
	 * Application::display() callback in order to smooth out the result of the
	 * fixed-rate ticks when applicable.
	 */
	Time<float> tickInterval;
};

/**
 * Transient information about the current frame of an Application.
 */
struct FrameInfo {
	/**
	 * Information about the latest processed tick.
	 */
	TickInfo tickInfo;

	/**
	 * The ratio of the latest processed tick's importance compared to the tick
	 * processed before it, for use when interpolating data between the two.
	 */
	float tickInterpolationAlpha;

	/**
	 * The time, in seconds, that had elapsed since the start of the application
	 * at the beginning of the current frame.
	 */
	Time<float> elapsedTime;

	/**
	 * The time, in seconds, elapsed between the beginning of the previous frame
	 * and the beginning of the current frame.
	 */
	Time<float> deltaTime;
};

/**
 * Configuration options for an Application.
 */
struct ApplicationOptions {
	/**
	 * Non-owning pointer to a null-terminated UTF-8 string that commonly
	 * identifies the publisher of the application, such as an organization
	 * name, alias or internet domain.
	 *
	 * This will be used as the name of the organization folder in the
	 * user/platform-specific preferences directory where the application folder
	 * will be created if it doesn't already exist, into which files such as
	 * user settings or saved images can then be read and written by the
	 * application.
	 *
	 * If set to nullptr, no application folder will be created nor mounted, and
	 * the application will be unable to write such files.
	 */
	const char* organizationName = nullptr;

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string that uniquely
	 * identifies the application among all other applications released by the
	 * same organization.
	 *
	 * This will be used as name of the application folder, that will be created
	 * if it doesn't exist, under the organization folder in the
	 * user/platform-specific preferences directory, into which files such as
	 * user settings or saved images can then be read and written by the 
	 * application.
	 *
	 * If set to nullptr, no application folder will be created nor mounted, and
	 * the application will be unable to write such files.
	 */
	const char* applicationName = nullptr;

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string of the native
	 * filepath to the main data directory which will be mounted for reading
	 * application resources.
	 *
	 * This may be either an absolute path or a path relative to the program's
	 * working directory.
	 *
	 * If set to nullptr, no main data directory will be mounted, and the
	 * application will be unable to read any resource files, except for those
	 * in the application folder under the organization folder in the
	 * user/platform-specific preferences directory, if one was specified.
	 */
	const char* dataDirectoryFilepath = ".";

	/**
	 * Non-owning pointer to a null-terminated UTF-8 string of the filename
	 * extension for mod archives.
	 *
	 * All mounted directories will be automatically searched for archives with
	 * this extension at application startup. Any found archives will be mounted
	 * for resource file reading, with a higher priority than the main data
	 * directory. This means that any file in an archive that has the same
	 * relative filepath as a file in the main data directory will be preferred
	 * over the original file. This is useful for allowing users to easily
	 * create and share modifications or plugins that add or override
	 * application resources, without having to hack the application.
	 *
	 * If set to nullptr, no archives will be automatically mounted.
	 */
	const char* archiveFilenameExtension = nullptr;

	/**
	 * The tick rate of the application, in hertz (ticks per second).
	 *
	 * This is the rate at which the application will try to trigger the
	 * processing of a tick, which is the main mechanism for providing
	 * application subsystems with updates at a fixed interval, independent from
	 * the main frame rate of the application.
	 *
	 * Tick polling is performed on each frame of the application, which may
	 * result in anywhere from 0 to tickRate/minFps ticks being processed,
	 * depending on the time since the previous frame. When not enough time has
	 * passed to process any ticks within a frame, the time is accumulated for
	 * the next frame, and so on, until enough time has passed to process more
	 * ticks. If several ticks' worth of time passed since the previous frame,
	 * multiple ticks will be processed, and any remaining time will carry over
	 * to the next frame. This results in a fixed average interval between
	 * ticks even in the event of high framerates or small frame rate drops.
	 */
	float tickRate = 60.0f;

	/**
	 * Minimum frame rate of the application, in hertz (frames per second),
	 * before tick slowdown occurs.
	 *
	 * If the frame rate drops below this limit, the application will start to
	 * skip the processing of some ticks in order to avoid a spiral of death
	 * where the amount of ticks to process continues to increase faster than
	 * they can be processed, which would lead to the application becoming
	 * completely unresponsive.
	 *
	 * If set to 0 or lower, or to a value higher than the tick rate, the
	 * maximum number of ticks per frame will be set to 1, causing slowdown to
	 * always occur whenever the frame rate goes below the tick rate. This is
	 * generally not recommended.
	 */
	float minFps = 10.0f;

	/**
	 * Maximum frame rate of the application, in hertz (frames per second),
	 * before frames are delayed.
	 *
	 * If the frame rate goes above this limit, the application will wait until
	 * enough time has passed for the next frame to begin.
	 */
	float maxFps = 60.0f;
};

/**
 * Main application base class.
 *
 * The application controls the main loop, including Event pumping, frame pacing
 * and fixed-interval frame rate-independent tick updates.
 *
 * Concrete applications should derive from this class and implement the
 * associated virtual functions in order to receive all of the relevant
 * callbacks. Deriving from this class also ensures that it is constructed
 * before any code in the concrete application is executed, which means that any
 * global systems will be properly initialized before any code that may depend
 * on them is able to run, assuming all user code is constrained to being called
 * from an instance of the concrete application class.
 */
class Application {
public:
	/**
	 * Type of message contained in a message box.
	 */
	enum class MessageBoxType {
		ERROR_MESSAGE,   ///< Indicates that an error occured.
		WARNING_MESSAGE, ///< Warns the user about a potential error.
		INFO_MESSAGE,    ///< Provides general information.
	};

	/**
	 * Display a simple message box that blocks execution on the current thread
	 * until the user presses OK.
	 *
	 * This does not require the application to be running.
	 *
	 * \param type type of message shown in the box. This may be reflected in
	 *        the styling of the box.
	 * \param title non-owning read-only pointer to a UTF-8 string containing
	 *        the window title of the message box. Must not be nullptr.
	 * \param message non-owning read-only pointer to a UTF-8 string containing
	 *        the main message to show in the box. Must not be nullptr.
	 *
	 * \throws application::Error on failure to show the message box.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void showSimpleMessageBox(MessageBoxType type, const char* title, const char* message);

	/**
	 * Construct the base of the main application and initialize global systems.
	 *
	 * \param programFilepath the first string in the argument vector passed to
	 *        the main function of the program, i.e. argv[0].
	 *
	 * \param options initial configuration of the application, see
	 *        ApplicationOptions.
	 *
	 * \throws application::Error if system initialization failed.
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \warning The behavior of passing programFilepath a value other than the
	 *          argv[0] string received from main is undefined.
	 */
	explicit Application(const char* programFilepath, const ApplicationOptions& options);

	/**
	 * Virtual destructor which must be overridden by the concrete application
	 * class in order to perform any application-specific cleanup before
	 * shutdown.
	 *
	 * May also be overridden implicitly by the compiler-generated destructor of
	 * the derived class.
	 */
	virtual ~Application() = default;

	/**
	 * Start the main loop of the application and keep running until the
	 * application quits or an unhandled exception is thrown.
	 *
	 * \note Under emscripten-based WebAssembly builds, this function will never
	 *       return. Instead, it explicitly calls the virtual application
	 *       destructor in order to perform any necessary cleanup when the main
	 *       loop has ended. It is therefore expected that any application-
	 *       specific cleanup or shutdown code be called from the overriden
	 *       destructor, rather than being called from main after run() has
	 *       finished.
	 *
	 * \throws any unhandled exception which was thrown during the execution of
	 *         the main loop, unless running under emscripten, in which case
	 *         exceptions are simply printed before shutting down.
	 *
	 * \remark The intended usage of this function is to call it once at the end
	 *         of main function of the program as the last code to be executed,
	 *         save for any catch blocks that are specific to non-emscripten
	 *         builds.
	 *
	 * \warning The result of calling this function while the application is
	 *          already running is undefined.
	 */
	void run();

	/**
	 * Initiate the shutdown process, meaning that the current frame will be the
	 * last to be processed and displayed before the main loop ends.
	 */
	void quit() noexcept;

	/**
	 * Check if the application is currently running, meaning that it is fully
	 * initialized, that run() has been called and has started the main loop,
	 * and that it is not in the process of shutting down.
	 *
	 * \return true if the application is currently running, false otherwise.
	 */
	[[nodiscard]] bool isRunning() const noexcept;

	/**
	 * Get the latest measurement of the average frame rate.
	 *
	 * The average frame rate is automatically measured for every second that
	 * passes while the application is running by keeping a frame counter that
	 * is incremented by 1 on each frame and reset to 0 when a full second has
	 * passed.
	 *
	 * \return the average frame rate over the last second that was measured, or
	 *         0 if less than one full second has passed since the start of the
	 *         application.
	 *
	 * \note This approximation of the frame rate does not update frequently
	 *       enough to be used as an accurate time delta between frames. Use the
	 *       values that are supplied in the FrameInfo struct to each
	 *       relevant callback for this purpose instead.
	 */
	[[nodiscard]] unsigned getLatestMeasuredFps() const noexcept;

	/**
	 * Get the current text contained in the clipboard.
	 *
	 * \return The text in the clipboard.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::string getClipboardText() const;

	/**
	 * Check if the application supports a screen keyboard.
	 *
	 * \return true if a screen keyboard is supported, false otherwise.
	 *
	 * \sa graphics::Window::isScreenKeyboardShown()
	 */
	[[nodiscard]] bool hasScreenKeyboardSupport() const noexcept;

	/**
	 * Set the frame rate parameters of the application.
	 *
	 * \param tickRate desired tick rate of the application, in hertz (ticks per
	 *        second).
	 * \param minFps minimum frame rate of the application, in hertz (frames per
	 *        second), before tick slowdown occurs.
	 * \param maxFps maximum frame rate of the application, in hertz (frames per
	 *        second), before frames are delayed.
	 *
	 * \sa ApplicationOptions::tickRate
	 * \sa ApplicationOptions::minFps
	 * \sa ApplicationOptions::maxFps
	 */
	void setFrameRateParameters(float tickRate, float minFps, float maxFps);

	/**
	 * Set the input rectangle for text input.
	 *
	 * \param offset offset of the input rectangle, in screen coordinates.
	 * \param size size of the input rectangle, in screen coordinates.
	 *
	 * \sa startTextInput()
	 * \sa stopTextInput()
	 */
	void setTextInputRectangle(glm::ivec2 offset, glm::ivec2 size);

	/**
	 * Start accepting text input events in the current text input rectangle.
	 *
	 * \sa setTextInputRectangle()
	 * \sa stopTextInput()
	 */
	void startTextInput();

	/**
	 * Stop accepting text input events.
	 *
	 * \sa setTextInputRectangle()
	 * \sa startTextInput()
	 */
	void stopTextInput();

protected:
	/**
	 * Initial frame callback, called in the main loop once at the beginning of
	 * each frame, before event processing.
	 *
	 * \param frameInfo information about the current frame, see FrameInfo.
	 *
	 * \note Any exception that is thrown out of this function will percolate up
	 *       to run() and cause the main loop to stop.
	 *
	 * \warning The behavior of calling this function manually is undefined.
	 *
	 * \sa handleEvent()
	 * \sa update()
	 * \sa tick()
	 * \sa display()
	 */
	virtual void prepareForEvents(FrameInfo frameInfo) = 0;

	/**
	 * Event pumping callback, called in the main loop 0 or more times during
	 * event processing, which happens on each frame after calling
	 * prepareForEvents() and before calling update(), in order to forward any
	 * events that occured since the last frame to the application for it to
	 * handle.
	 *
	 * \param frameInfo information about the current frame, see FrameInfo.
	 * \param event the event that occured, see Event.
	 *
	 * \note Any exception that is thrown out of this function will percolate up
	 *       to run() and cause the main loop to stop.
	 *
	 * \warning The behavior of calling this function manually is undefined.
	 *
	 * \sa prepareForEvents()
	 * \sa update()
	 * \sa tick()
	 */
	virtual void handleEvent(FrameInfo frameInfo, const Event& event) = 0;

	/**
	 * Post-event frame callback, called in the main loop once on each frame
	 * after processing events and before processing ticks.
	 *
	 * This is the best time to apply changes to any interactive application
	 * state that depends on user input and is used by tick(), since it
	 * minimizes the average latency between processing an input event and it
	 * affecting the result of a subsequent tick.
	 *
	 * \param frameInfo information about the current frame, see FrameInfo.
	 *
	 * \note Any exception that is thrown out of this function will percolate up
	 *       to run() and cause the main loop to stop.
	 *
	 * \warning The behavior of calling this function manually is undefined.
	 *
	 * \sa prepareForEvents()
	 * \sa tick()
	 */
	virtual void update(FrameInfo frameInfo) = 0;

	/**
	 * Fixed-interval tick callback, called in the main loop 0 or more times
	 * during tick processing, which happens on each frame after calling
	 * update() and before calling display().
	 *
	 * See the documentation of TickInfo::tickInterval for an explanation of
	 * what this function may be useful for.
	 *
	 * \param tickInfo information about the current tick, see TickInfo.
	 *
	 * \note Any exception that is thrown out of this function will percolate up
	 *       to run() and cause the main loop to stop.
	 *
	 * \warning The behavior of calling this function manually is undefined.
	 *
	 * \sa prepareForEvents()
	 * \sa update()
	 * \sa display()
	 */
	virtual void tick(TickInfo tickInfo) = 0;

	/**
	 * Frame rendering callback, called in the main loop once at the end of each
	 * frame after processing ticks, in order to render the latest state of the
	 * application.
	 *
	 * Before rendering, this is also the best time to apply any final cosmetic
	 * changes to the state of the application that is about to be presented,
	 * such as interpolation of data that is updated in tick().
	 *
	 * \param frameInfo information about the current frame, see FrameInfo.
	 *
	 * \note Any exception that is thrown out of this function will percolate up
	 *       to run() and cause the main loop to stop.
	 *
	 * \warning The behavior of calling this function manually is undefined.
	 *
	 * \sa prepareForEvents()
	 * \sa update()
	 * \sa tick()
	 */
	virtual void display(FrameInfo frameInfo) = 0;

private:
	void runFrame();

	struct PhysFSManager {
		PhysFSManager(const char* programFilepath, const char* organizationName, const char* applicationName, const char* dataDirectoryFilepath,
			const char* archiveFilenameExtension);
	};

	struct SDLManager {
		SDLManager();
	};

	[[no_unique_address]] PhysFSManager physFSManager;
	[[no_unique_address]] SDLManager sdlManager{};
	std::uint64_t clockFrequency = 0;
	std::uint64_t tickClockInterval = 0;
	std::uint64_t minFrameClockInterval = 0;
	std::uint64_t maxTicksPerFrame = 0;
	std::uint64_t startClockTime = 0;
	std::uint64_t latestFrameClockTime = 0;
	std::uint64_t latestFpsMeasurementClockTime = 0;
	std::uint64_t processedTickClockTime = 0;
	float clockInterval = 0.0f;
	unsigned latestMeasuredFps = 0u;
	unsigned fpsCounter = 0u;
	FrameInfo frameInfo{};
	bool running = false;
};

} // namespace application
} // namespace donut

#endif
