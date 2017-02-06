gdbus call --system \
--dest com.pelagicore.SoftwareContainerAgent \
--object-path /com/pelagicore/SoftwareContainerAgent \
--method com.pelagicore.SoftwareContainerAgent.SetCapabilities \
0 \
"['com.pelagicore.temperatureservice.gettemperature',
'com.pelagicore.temperatureservice.settemperature']"

