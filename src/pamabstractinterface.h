
#include <string>

class PAMAbstractInterface {

public:
	virtual void registerClient(const std::string &cookie,
		const std::string &appId) = 0;
	virtual void unregisterClient(const std::string &appId) = 0;
	virtual void updateFinished() = 0;
};