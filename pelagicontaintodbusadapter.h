class PelagicontainToDBusAdapter : 
	public Pelagicontain_adaptor,
	public DBus::IntrospectableAdaptor,
	public DBus::ObjectAdaptor
{
public:
	virtual std::string Echo(const std::string& argument);
	virtual void Launch(const std::string& appId);
}
