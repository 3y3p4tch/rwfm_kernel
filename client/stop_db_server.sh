pid_var=`ps -A|grep "database_server"|cut -d" " -f2`
sudo kill "${pid_var}"
