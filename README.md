
* Installing client.

  Run following commands from the machine you want to protect.

  #git clone https://github.com/parjanya-vyas/secureOS_kernel
  
  #cd secureOS_kernel/client/
  
  Configure rwfmd.cfg to use user Rules engine and run install script -

  #./install.sh

  Once installation is done, run secure shell as  
 
  #/opt/secos/bin/secure_shell

  Enable rwfm as follows - 

  #rwfm enable

  You can see the rwfmd daemon log @  /var/log/rwfmd.log

Run your test program.
