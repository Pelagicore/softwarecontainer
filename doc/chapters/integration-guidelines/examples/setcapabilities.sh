gdbus call --system \
--dest com.pelagicore.SoftwareContainerAgent \
--object-path /com/pelagicore/SoftwareContainerAgent \
--method com.pelagicore.SoftwareContainerAgent.SetCapabilities \
0 \
"['network.accept-ping',
'test.cap.netcls']"

