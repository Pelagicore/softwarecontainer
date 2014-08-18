

enum IPCCommand : uint8_t {
	RUN_APP = '1', KILL_APP = '2', SET_ENV_VAR = '3', SYS_CALL = '4'
};

namespace pelagicontain {
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

}

using namespace pelagicontain;

inline bool isLXC_C_APIEnabled() {
	return false;
}

inline bool isContainerDeleteEnabled() {
	return false;
}
