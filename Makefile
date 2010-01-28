sjni_test: sjni.cpp sjni.h
	g++ sjni.cpp -I/opt/sun-jdk-1.6.0.17/include/ -I/opt/sun-jdk-1.6.0.17/include/linux/ -L /opt/sun-jdk-1.6.0.17/jre/lib/i386/client/ -ljvm -o sjni_test -ggdb
