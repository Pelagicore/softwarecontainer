#pragma once

#include <map>
#include <memory>
#include <vector>

#include "pelagicontain-config.h"
#include "pelagicontain-log.h"
#include "pelagicore-common.h"

#include <jansson.h>

namespace pelagicontain {

	typedef std::map<std::string, std::string> GatewayConfiguration;

	enum IPCCommand : uint8_t {
		RUN_APP = '1', KILL_APP = '2', SET_ENV_VAR = '3', SYS_CALL = '4'
	};

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

	inline sigc::connection addProcessListener(pid_t pid, std::function<void(pid_t, int)> function, Glib::RefPtr<Glib::MainContext> context = Glib::MainContext::get_default()) {
		return context->signal_child_watch().connect(function, pid);
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
			parseConfig(config);
		}

		~JSonParser() {
		    if (m_root) {
		        json_decref(m_root);
		    }
		}

		bool isValueTrue(const std::string& field) {
			json_t* value = json_object_get(m_root, field.c_str());
			if (!json_is_boolean(value)) {
				log_error("Value is not a boolean.");
				json_decref(value);
			}

			return json_is_true(value);
		}

		std::string getValueAsString(const std::string& field) {
			std::string s;
			json_t* value = json_object_get(m_root, field.c_str());
			if (value != nullptr) {
				if (json_is_string(value)) {
					s = json_string_value(value);
				} else
					log_error("Value is not a string.");

				json_decref(value);
			}

			return s;
		}

	std::vector<std::string> getValueAsStringArray(const std::string& field) {
		std::vector < std::string > s;
		json_t* value = json_object_get(m_root, field.c_str());
		if (value != nullptr) {
			if (json_is_array(value)) {
				for (size_t i = 0; i < json_array_size(value); i++) {
					json_t *arrayElement = json_array_get(value, i);
					if (json_is_string(arrayElement))
						s.push_back(json_string_value(arrayElement));
					else
						log_error() << "Value is not a string";

				}
			} else
				log_error() << "Value is not an array";

			json_decref(value);
		}

		return s;
	}

	json_t* root() {
		return m_root;
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


