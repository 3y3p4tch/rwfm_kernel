#!/bin/bash
export USE_RWFM=0
export hexNum=`hostid`
export HOSTID=`printf "%d" 0x${hexNum}`
export HOSTNAME=`hostname`
export ADD_USER=0
export ADD_GROUP=1

#
# Main starts here..
#

#config_file="rwfmd.cfg"
#if [[ ! -f $config_file ]];then
#    echo "$config_file missing!"
#    exit 1
#fi

#webhost=`cat $config_file | grep -w webhost | awk -F: '{print $2}' | sed -e 's/[", ]//g'`
#webport=`cat $config_file | grep -w webport | awk -F: '{print $2}' | sed -e 's/[", ]//g'`
#rwfmd_port=`cat $config_file | grep rwfmd_port | awk -F: '{print $2}' | sed -e 's/[",]//g' | awk '{print $1}'`
#baseurl="http://$webhost:$webport/rwfm"
#group_url="$baseurl/add/group/"
#user_url="$baseurl/add/user/"
#echo $group_url
#echo $user_url

rm -f preload.so preload.h database_server
make || {
        echo "failed to build preload library."
        exit 1
}
echo "library built"

mkdir -p /lib/secos/
cp -f preload.so /lib/secos/
echo "library copied"

mkdir -p /opt/secos/bin/
#cp daemon.py rwfmd.py rwfm secure_shell /opt/secos/bin/
cp -f database_server secure_shell enable_rwfm /opt/secos/bin/
chmod +x /opt/secos/bin/*
#cp rwfmd.cfg /etc/

export PATH=$PATH:/opt/secos/bin/

mkfifo /tmp/request_fifo
mkfifo /tmp/response_fifo

chmod 777 /tmp/request_fifo /tmp/response_fifo

/opt/secos/bin/database_server &
echo "database server started"

#python /opt/secos/bin/rwfmd.py restart || {
#        echo "failed to start daemon."
#        exit 1
#}
#echo "daemon started"

perl -w list_users_and_groups.pl ${HOSTNAME}

#update_users || {
#        echo "failed to update users in webapp."
#        exit 1
#}

#echo
#echo "users added"
#echo

#update_groups || {
#        echo "failed to update groups in webapp."
#        exit 1
#}

#echo
#echo "groups added"
#echo

exit 0
