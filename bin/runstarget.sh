#!/bin/sh

echo "running ${1}..."

cd /opt/tests/buteo-sync-plugins

exec ${1} -maxwarnings 0
