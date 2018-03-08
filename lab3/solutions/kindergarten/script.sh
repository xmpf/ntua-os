PID=$(ps -ef | grep kgarten-2 | awk '{print $2}' | head -n 1 )
top -H -p $PID
