gcc -shared -o libicmp_native.so -fPIC \
    -I/opt/jdk-21.0.2/include \
    -I/opt/jdk-21.0.2/include/linux \
    icmp_native.c
