#include <map>
#include <memory>
#include <vector>

#include "pelagicontain-config.h"
#include "pelagicontain-log.h"

#pragma once


namespace pelagicontain {

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

}

using namespace pelagicontain;


