#  Thomas Laroche
#  M1 SIAME
# conding : utf-8

import os


print "1 - ecriture"
print "2 - lecture non-destructrice"
print "3 - lecture destructrice"
print "exit - exit"

write = open("/dev/write",'w+');
readND = open("/dev/readND",'w+');
readD = open("/dev/readD",'w+');


cmd ="";
while cmd = "exit":
    cmd = raw_input(">>> ");
    if cmd =="1":
        print "texte à ecrire"
        texte = raw_input(">>> ");
        write.write("texte blablablablablablabla");
    elif cmd =="2":
        print readND.read();
    elif cmd = "3":
        print readD.read();

print "Fin"
