gdbus call --system \
--dest com.pelagicore.SoftwareContainerAgent \
--object-path /com/pelagicore/SoftwareContainerAgent \
--method com.pelagicore.SoftwareContainerAgent.BindMount \
0 \
"$HOME/softwarecontainer" \
"/app" \
false
