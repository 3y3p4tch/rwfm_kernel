pid_var=`ps -A|grep "database_server"|cut -d" " -f1`
sudo kill "${pid_var}"
