#pragma once

#include <sys/wait.h>

#include <map>
#include <memory>
#include <vector>

#include "pelagicontain-config.h"
#include "pelagicontain-log.h"
#include "pelagicore-common.h"

#include <jansson.h>

namespace pelagicontain {

	typedef std::map<std::string, std::string> GatewayConfiguration;

	enum class ReturnCode {
		FAILURE,
		SUCCESS
	};

	inline bool isError(ReturnCode code) {
		return (code != ReturnCode::SUCCESS);
	}

	inline bool isSuccess(ReturnCode code) {
		return !isError(code);
	}

	typedef std::map<std::string, std::string> EnvironmentVariables;

	static constexpr const char* APP_BINARY = "/appbin/containedapp";

	inline bool isLXC_C_APIEnabled() {
		return true;
	}

	inline bool isContainerDeleteEnabled() {
		return false;
	}

	/**
	 * That class contains references to sigc++ connections and automatically disconnects them on destruction
	 */
	class SignalConnectionsHandler {

	public:

		/**
		 * Create a new connection
		 */
		sigc::connection& newConnection() {
			m_connections.resize(m_connections.size() + 1);
			return m_connections[m_connections.size() - 1 ];
		}

		~SignalConnectionsHandler() {
			for(auto& connection: m_connections)
				connection.disconnect();
		}

private:
		std::vector<sigc::connection> m_connections;

	};

	inline void addProcessListener(SignalConnectionsHandler& connections, pid_t pid, std::function<void(pid_t, int)> function
			, Glib::RefPtr<Glib::MainContext> context // = Glib::MainContext::get_default()
	) {
		auto connection = context->signal_child_watch().connect(function, pid);
		connections.newConnection() = connection;
	}

	inline int waitForProcessTermination(pid_t pid) {
		int status;
		waitpid(pid, &status, 0);
		return status;
	}

	/*
	 * Check if path is a directory
	 */
	bool isDirectory(const std::string &path);
	bool isSocket(const std::string &path);
	std::string parentPath(const std::string &path);
	ReturnCode touch(const std::string& path);

	template<typename Type>
	class ObservableProperty {
	public:
		typedef std::function<void(const Type&)> Listener;

		ObservableProperty(Type& value) : m_value(value) {
		}

		void addListener(Listener listener) {
			m_listeners.push_back(listener);
		}

		operator const Type&() {
			return m_value;
		}

	protected:
		std::vector<Listener> m_listeners;

	private:
		const Type& m_value;

	};

	template<typename Type>
	class ObservableWritableProperty: public ObservableProperty<Type> {
	public:
		ObservableWritableProperty() : ObservableProperty<Type>(m_value) {
		}

		void setValueNotify(Type value) {
			m_value = value;
			for(auto& listener : ObservableProperty<Type>::m_listeners) {
				listener(getValue());
			}
		}

		const Type& getValue() const {
			return m_value;
		}

		ObservableWritableProperty& operator=(const Type& type) {
			m_value = type;
			return *this;
		}

	private:
		Type m_value;

	};


	class JSonParser {

		LOG_DECLARE_CLASS_CONTEXT("JSON", "JSON parser");

	public:
		JSonParser(const std::string &config) {
			if (config != "")
				parseConfig(config);
			else
				parseConfig("{}");
		}

		JSonParser(json_t* element) {
			m_root = element;
			json_incref(m_root);
		}

		~JSonParser() {
		    if (m_root) {
		        json_decref(m_root);
		    }
		}

		bool isArray() {
			return json_is_array(m_root);
		}

		void read(const std::string& field, std::vector<JSonParser>& elements) {
			json_t* value = json_object_get(m_root, field.c_str());
			if (value != nullptr) {

				if (json_is_array (value)) {
					for (size_t i = 0; i < json_array_size(value); i++) {
						json_t *arrayElement = json_array_get(value, i);
						elements.push_back(JSonParser(arrayElement));
					}
				} else
					log_error() << "Value is not an array";
			}
		}

		size_t elementCount() {
			return json_array_size(m_root);
		}

		JSonParser arrayElementAt(size_t index) {
			if (json_is_array(m_root)) {
				json_t *arrayElement = json_array_get(m_root, index);
				return JSonParser(arrayElement);
			} else
				log_error() << "Value is not an array";

			return JSonParser("");
		}

		void readString(const std::string& field, std::string& v) {
			read(field, v);
		}

		void read(const std::string& field, std::string& v) {
			json_t* value = json_object_get(m_root, field.c_str());
			if (value != nullptr) {
				if (json_is_string(value)) {
					v = json_string_value(value);
				} else
					log_error("Value is not a string.");

				json_decref(value);
			}
		}

		void readBoolean(const std::string& field, bool& v) {
			return read(field,v);
		}

		void read(const std::string& field, bool& v) {
			json_t* value = json_object_get(m_root, field.c_str());
			if (value != nullptr) {
				if (json_is_boolean(value)) {
					v = json_is_true(value);
				} else
					log_error("Value is not a bool.");

				json_decref(value);
			}
		}

		void readStringArray(const std::string& field, std::vector < std::string >& v) {
			json_t* value = json_object_get(m_root, field.c_str());
			if (value != nullptr) {
				if (json_is_array(value)) {
					for (size_t i = 0; i < json_array_size(value); i++) {
						json_t *arrayElement = json_array_get(value, i);
						if (json_is_string(arrayElement))
							v.push_back(json_string_value(arrayElement));
						else
							log_error() << "Value is not a string";

					}
				} else
					log_error() << "Value is not an array";

				json_decref(value);
			}
		}

	json_t* root() {
		return m_root;
	}

	bool isValid() {
		return (m_root != nullptr);
	}

	private:

		void parseConfig(const std::string &config) {
			json_error_t error;

			// Get root JSON object
			m_root = json_loads(config.c_str(), 0, &error);

			if (m_root == nullptr)
				log_error() << "Error on line " << error.line << ". " << error.text;
		}

		json_t* m_root;

	};

}

using namespace pelagicontain;
using logging::StringBuilder;

