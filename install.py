#  Thomas Laroche
#  M1 SIAME
# conding : utf-8

import os

os.sys("insmod tp2-5.ko");
print "Module installé"

os.sys("mknod /dev/write c 248 0 ");
os.sys("mknod /dev/readND c 248 1 ");
os.sys("mknod /dev/readD c 248 2 ");
print "Devices Ajoutés"

