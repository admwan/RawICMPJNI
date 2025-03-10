package net.spikesync.pingerdaemonrabbitmqclient;

public class RawICMPJNI {
    static {
        System.loadLibrary("icmp_native"); // Load native library (libicmp_native.so / icmp_native.dll)
    }

    // Declare the native method
    public native boolean sendICMPPing(String ip);

    public static void main(String[] args) {
        net.spikesync.pingerdaemonrabbitmqclient.RawICMPJNI ping = new net.spikesync.pingerdaemonrabbitmqclient.RawICMPJNI();
        String target = "192.168.50.234"; // Replace with your container IP

        if (ping.sendICMPPing(target)) {
            System.out.println(target + " is reachable via ICMP.");
        } else {
            System.out.println(target + " is NOT reachable.");
        }
    }
}
