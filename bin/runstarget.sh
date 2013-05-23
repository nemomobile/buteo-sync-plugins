#!/bin/sh

echo "running ${1}..."

cd /opt/tests/buteo-sync-plugins
if [ -f /tmp/session_bus_address.user ]; 
then 
source /tmp/session_bus_address.user;
export DISPLAY=:1
fi	
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

exec ${1} -maxwarnings 0
