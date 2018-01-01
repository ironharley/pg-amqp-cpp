#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <string>

#ifndef _PACKAGE
#define _PACKAGE "bs.jmtv.ent.logging"
#endif
class ApplicationLogger {
public:
	enum TypeLog {
		Debug, Info, Warn, Error
	};

	ApplicationLogger() {
	}
	ApplicationLogger(TypeLog type) {
		msglevel = type;
		operator <<("\t" + getLabel(type) + "\t");
	}
	~ApplicationLogger() {
		if (opened) {
			std::cout << std::endl;
		}
		opened = false;
	}
	template<class T>
	ApplicationLogger &operator<<(const T &msg) {
		std::cout << msg;
		opened = true;
		return *this;
	}
private:

	TypeLog level = Warn;
	bool opened = false;
	TypeLog msglevel = Debug;

	unsigned long get_thread_id() {
		static_assert(sizeof(std::thread::id)==sizeof(uint64_t),"this function only works if size of thead::id is equal to the size of uint_64");
		auto id = std::this_thread::get_id();
		uint64_t* ptr = (uint64_t*) &id;
		return (unsigned long) (*ptr);
	}

	inline std::string getLabel(TypeLog type) {

		auto now = std::chrono::system_clock::now();
		std::time_t this_time = std::chrono::system_clock::to_time_t(now);
		char* time_string = std::ctime(&this_time);
		int len = strlen(time_string);
		sprintf(time_string, "%.*s", len - 1, time_string);

		std::string label;
		switch (type) {
		case Debug:
			label = "DEBUG\t";
			break;
		case Info:
			label = "INFO \t";
			break;
		case Warn:
			label = "WARN\t";
			break;
		case Error:
			label = "ERROR\t";
			break;
		}
		return label.append(std::string("[")).append(time_string).append(
				std::string(": ")).append(_PACKAGE).append(std::string("]:[")).append(
				std::to_string(get_thread_id())).append("]: ");
	}
};

#endif  /* LOG_H */

