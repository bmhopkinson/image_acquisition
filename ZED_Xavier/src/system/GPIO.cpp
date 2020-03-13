#include "system/GPIO.h"

#include <fstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>


// Static streams to export and unexport files
std::ofstream GPIO::export_stream("/sys/class/gpio/export");
std::ofstream GPIO::unexport_stream("/sys/class/gpio/unexport");


// Translate given direction into string used for sysfs configuration
std::string GPIO::direction_to_string(Direction direction) {
	switch (direction) {
		case OUTPUT: return "out";
		case INPUT:
		default: return "in";
	}
}


// Translate given edge into string used for sysfs configuration
std::string GPIO::edge_to_string(Edge edge) {
	switch (edge) {
		case RISING: return "rising";
		case FALLING: return "falling";
		case BOTH: return "both";
		case NONE:
		default: return "none";
	}
}


// Translate string into direction
GPIO::Direction GPIO::direction_from_string(std::string dir_str) {
	if (dir_str == "out") return OUTPUT;
	return INPUT;
}


// Translate string into edge
GPIO::Edge GPIO::edge_from_string(std::string edge_str) {
	if (edge_str == "rising") return RISING;
	if (edge_str == "falling") return FALLING;
	if (edge_str == "both") return BOTH;
	return NONE;
}


// Export a pin and set its direction, edge, and inversion.
// Returns true if successful, false if an error occurred.
// Note: an inverted output pin has a default value of low (+Vlogic) and is set to active low for reading
bool GPIO::export_pin(unsigned int pin, Direction direction, Edge edge, bool inverted) {
	// Check if export can be accessed
	if (export_stream.fail()) return false;

	// Export pin number and wait 30ms for it to process
	export_stream << pin << std::flush;
	this->pin = pin;
	pin_exported = true;
	usleep(50*1000);

	// Create string with pin directory
	std::string pin_dir = "/sys/class/gpio/gpio" + std::to_string(pin) + "/";

	// Attempt to set direction, unexport if error
	std::ofstream direction_stream(pin_dir + "direction");
	if (direction_stream.fail()) {
		unexport_pin();
		return false;
	}
	direction_stream << direction_to_string(direction) << std::flush;
	this->direction = direction;

	// Attempt to set edge, unexport if error
	std::ofstream edge_stream(pin_dir + "edge");
	if (edge_stream.fail()) {
		unexport_pin();
		return false;
	}
	edge_stream << edge_to_string(edge) << std::flush;
	this->edge = edge;

	// Set active low if inverted, unexport if error
	if (inverted) {
		std::ofstream al_stream(pin_dir + "active_low");
		if (al_stream.fail()) {
			unexport_pin();
			return false;
		}
		al_stream << 1 << std::flush;
	}
	this->inverted = inverted;

	// Attempt to open value file
	if (direction == INPUT) {
		value_fd = open((pin_dir + "value").c_str(), O_RDONLY);
	} else {
		value_fd = open((pin_dir + "value").c_str(), O_RDWR);
	}
	usleep(5);

	// Check if value file opened successfully
	if (value_fd < 0) {
		unexport_pin();
		return false;
	}

	// Set up pollfd
	poll_fd.fd = value_fd;
	poll_fd.events = POLLPRI;

	// Ensure outputs start low
	if (direction == OUTPUT) set_value(false);

	return true;
}


// Unexport the pin.
// Returns true if successful, false if already exported or an error occurred.
bool GPIO::unexport_pin() {
	// Check if pin already exported and unexport file can be accessed
	if ((not pin_exported) or unexport_stream.fail()) return false;

	// Unexport pin number and wait 30ms for it to process
	unexport_stream << pin << std::flush;
	pin_exported = false;
	return true;
}


// Returns true if pin is high, false if low.
// Throws ExportException if pin is not exported.
// Throws AccessException if there was an error accessing the value.
bool GPIO::get_value() {
	if (not pin_exported) throw gpio_export_exception;
	char val;
	lseek(value_fd, 0, SEEK_SET);
	if (read(value_fd, &val, 1) != 1) throw gpio_access_exception;
	return val == '1';
}


// Sets uninverted pin to high if value is true, low if false. Does the opposite if pin is inverted.
// Throws ExportException if pin is not exported as output.
// Throws AccessException if there was an error accessing the value.
void GPIO::set_value(bool value) {
	if (not (pin_exported and (direction == OUTPUT))) throw gpio_export_exception;
	const char* val = value ? "1" : "0";
	lseek(value_fd, 0, SEEK_SET);
	if (write(value_fd, val, 2) != 2) throw gpio_access_exception;
	cur_value = value;
}


// Wait for desired edge condition to occur.
// Waits for timeout milliseconds, or indefinitely if timeout is -1.
// Waiting for no edge is treated as no transitions occurring during the timeout period.
// Returns true if pin condition occurred, false if timed out.
// Throws ExportException if pin is not exported.
// Throws AccessException if there was an error accessing the value.
bool GPIO::wait_edge(Edge edge, int timeout) {
	if (not pin_exported) throw gpio_export_exception;

	// Waiting for no edge is the same as timeout waiting for any edge
	if (edge == NONE) return not wait_edge(BOTH, timeout);

	char val;

	// Setup duration measurement for timeout
	std::chrono::milliseconds elapsed(0);
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	// Use interrupts if hardware edge detection enabled for desired condition
	if (this->edge == BOTH or this->edge == edge) {
		while (true) {
			// Consume any prior read interrupt
			lseek(value_fd, 0, SEEK_SET);
			if (read(value_fd, &val, 1) != 1) throw gpio_access_exception;
	
			// Wait for next interrupt until remainder of timeout period, return false if timeout
			if (poll(&poll_fd, 1, timeout>0 ? timeout-elapsed.count() : timeout) == 0) return false;

			// Update elapsed time
			elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

			// Consume interrupt
			lseek(value_fd, 0, SEEK_SET);
			if (read(value_fd, &val, 1) != 1) throw gpio_access_exception;

			// Return true if pin value matches condition
			if (edge == BOTH or ((edge == RISING) != (val=='0'))) return true;
		}

	// Otherwise fall back to active polling
	} else {
		bool prev;
		do {
			// Read pin value
			lseek(value_fd, 0, SEEK_SET);
			if (read(value_fd, &val, 1) != 1) throw gpio_access_exception;

			// Get initial previous value if needed
			if (elapsed.count() == 0) prev = (val=='1');

			// Return true if transition detected and pin value matches condition
			if ((val=='1') != prev) {
				if (edge == BOTH or ((edge == RISING) != (val=='0'))) return true;
			}
			prev = (val=='1');

			// Update elapsed time, return false if at end of timeout
			elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
		} while (elapsed.count() < timeout);
		return false;
	}
}


// Set pin output to value for specified duration in microseconds, then return pin to previous value.
// If blocking (default) waits for end of pulse, otherwise detaches separate thread to perform pulse and returns immediately.
// Additional pulses will be ignored if a pulse is already running.
void GPIO::pulse(bool value, unsigned int duration_us, bool blocking) {
	if (blocking) {
		pulse_(value, duration_us);
	} else {
		std::thread(&GPIO::pulse_, this, value, duration_us).detach();
	}
}


// Private function to execute pulse
void GPIO::pulse_(bool value, unsigned int duration_us) {
	if (not pulsing) {
		pulsing = true;
		bool prev_value = cur_value;
		set_value(value);
		usleep(duration_us);
		set_value(prev_value);
		pulsing = false;
	}
}


// Returns the direction of the pin.
// Throws ExportException if pin is not exported.
GPIO::Direction GPIO::get_direction() {
	if (not pin_exported) throw gpio_export_exception;
	return direction;
}


// Returns the edge setting for the pin.
// Throws ExportException if pin is not exported.
GPIO::Edge GPIO::get_edge() {
	if (not pin_exported) throw gpio_export_exception;
	return edge;
}


// Returns true if the pin is inverted, false otherwise.
// Throws ExportException if pin is not exported.
bool GPIO::get_inverted() {
	if (not pin_exported) throw gpio_export_exception;
	return inverted;
}

bool GPIO::get_activity(){
    std::lock_guard<std::mutex> lock(monitor_data.activity_mutex);
    return monitor_data.activity;
}

double GPIO::get_duration(){
    std::lock_guard<std::mutex> lock(monitor_data.activity_dur_mutex);
    return monitor_data.activity_dur;
}

void GPIO::clear_activity() {
    std::lock_guard<std::mutex> lock(monitor_data.activity_mutex);
    std::lock_guard<std::mutex> lock2(monitor_data.activity_dur_mutex);
    monitor_data.activity = false;
    monitor_data.activity_dur = -1;
}

//lauch thread to monitor button presses - add duration monitoring
void GPIO::monitor_button(int timeout_m){
    if(timeout_m < 0){
        std::cout << "error GPIO::monitor_button timeout must be finite" << std::endl;
        return;
    }

    monitor_running = true;
    monitor_thread = std::thread(&GPIO::monitor_button_, this, timeout_m);

}

void GPIO::monitor_button_(int timeout_m){
    while(this->monitor_running){
        bool rising_edge = this->wait_edge(GPIO::RISING, timeout_m);  //should lock activity with mutext before updating;

        if(rising_edge) {
             std::chrono::milliseconds elapsed(0);
		     std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
             this->wait_edge(GPIO::FALLING, -1);
             elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
             //std::cout << "button press duration: " << std::to_string(elapsed.count()) << std::endl;
             std::lock_guard<std::mutex> lock(monitor_data.activity_dur_mutex); 
             monitor_data.activity_dur = elapsed.count();

             if(not monitor_data.activity) {
                std::lock_guard<std::mutex> lock(monitor_data.activity_mutex);
                monitor_data.activity = true;
             }
        } //end if(rising_edge)
    } //end while loop
}


GPIO::~GPIO(){
    if(monitor_running){
        monitor_running = false;
        monitor_thread.join();
    }
    unexport_pin(); 
}
