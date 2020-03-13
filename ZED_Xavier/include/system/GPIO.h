#ifndef GPIO_H_
#define GPIO_H_

#include <fstream>
#include <string>
#include <exception>
#include <poll.h>
#include<thread>
#include <mutex>

struct Monitor_Data {
    bool activity = false;
    std::mutex activity_mutex;
    double activity_dur = -1;
    std::mutex activity_dur_mutex;
};

class GPIO {
	public:
		class GPIOExportException: public std::exception {
			virtual const char* what() const throw() {
				return "Attempted to use unexported GPIO";
			}
		} gpio_export_exception;

		class GPIOAccessException: public std::exception {
			virtual const char* what() const throw() {
				return "Error accessing GPIO value";
			}
		} gpio_access_exception;

		enum Direction {INPUT, OUTPUT};
		static std::string direction_to_string(Direction direction);
		static Direction direction_from_string(std::string dir_str);

		enum Edge {NONE, RISING, FALLING, BOTH};
		static std::string edge_to_string(Edge edge);
		static Edge edge_from_string(std::string edge_str);

		bool export_pin(unsigned int pin, Direction direction, Edge edge, bool invert=false);
		bool export_pin(unsigned int pin, Direction direction, bool invert=false) { return export_pin(pin, direction, NONE, invert); }
		bool unexport_pin();

		bool get_value();
		void set_value(bool value);
		bool wait_edge(Edge edge, int timeout=-1);
		void pulse(bool value, unsigned int duration_us, bool blocking=true);
        void monitor_button(int timeout_m=500);
        bool get_activity(); 
        double get_duration();
        void clear_activity();

		Direction get_direction();
		Edge get_edge();
		bool get_inverted();
		bool exported() { return pin_exported; }

		~GPIO();

	private:
		unsigned int pin;
		Direction direction = INPUT;
		Edge edge = NONE;
		bool inverted = false;
		bool pin_exported = false;
		bool cur_value = false;
		bool pulsing = false;
		int value_fd;
		struct pollfd poll_fd;

		static std::ofstream export_stream;
		static std::ofstream unexport_stream;
		void pulse_(bool value, unsigned int duration_us);

        std::thread monitor_thread;
        bool monitor_running = false;
        void monitor_button_(int timeout_m);
        Monitor_Data monitor_data;
               
};

#endif
