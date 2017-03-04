#  Thomas Laroche
#  M1 SIAME
# conding : utf-8

import os


print "1 - ecriture"
print "2 - lecture non-destructrice"
print "3 - lecture destructrice"
print "exit - exit"

write = open("/dev/tp1",'w+');
readND = open("/dev/tp2",'w+');
readD = open("/dev/tp3",'w+');


cmd ="";
while cmd = "exit":
    cmd = raw_input(">>> ");
    if cmd =="1":
        print "texte à ecrire"
        texte = raw_input(">>> ");
        os.sys("echo \""texte"\" > /dev/tp1");
    elif cmd =="2":
        os.sys("cat /dev/tp2");
    elif cmd = "3":
        os.sys("cat /dev/tp3");

print "Fin"
