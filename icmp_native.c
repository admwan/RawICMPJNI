#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

// Compute checksum for ICMP packet
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// JNI Method to send an ICMP Echo Request
JNIEXPORT jboolean JNICALL Java_net_spikesync_pingerdaemonrabbitmqclient_RawICMPJNI_sendICMPPing(JNIEnv *env, jobject obj, jstring ipAddress) {
    const char *target_ip = (*env)->GetStringUTFChars(env, ipAddress, 0);

    // Open the raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        (*env)->ReleaseStringUTFChars(env, ipAddress, target_ip);
        return JNI_FALSE;
    }

    // Set timeout for recvfrom
    struct timeval tv;
    tv.tv_sec = 1;  // 1-second timeout
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        perror("Failed to set socket timeout");
        close(sockfd);
        (*env)->ReleaseStringUTFChars(env, ipAddress, target_ip);
        return JNI_FALSE;
    }

    // Configure destination address
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip, &dest.sin_addr);

    // Prepare ICMP packet
    struct icmp icmp_packet;
    memset(&icmp_packet, 0, sizeof(icmp_packet));
    icmp_packet.icmp_type = ICMP_ECHO;
    icmp_packet.icmp_code = 0;
    icmp_packet.icmp_id = getpid() & 0xFFFF;  // Ensure ID fits in 16 bits
    icmp_packet.icmp_seq = 1;
    icmp_packet.icmp_cksum = checksum(&icmp_packet, sizeof(icmp_packet));

    // Send the ICMP packet
    if (sendto(sockfd, &icmp_packet, sizeof(icmp_packet), 0, (struct sockaddr *)&dest, sizeof(dest)) <= 0) {
        perror("ICMP send failed");
        close(sockfd);
        (*env)->ReleaseStringUTFChars(env, ipAddress, target_ip);
        return JNI_FALSE;
    }

    // Prepare for response
    char recv_buffer[1024];
    struct sockaddr_in reply_addr;
    socklen_t addr_len = sizeof(reply_addr);
    ssize_t bytes_received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&reply_addr, &addr_len);

    if (bytes_received > 0) {
        printf("Received ICMP reply from %s\n", target_ip);
        close(sockfd);
        (*env)->ReleaseStringUTFChars(env, ipAddress, target_ip);
        return JNI_TRUE;
    } else {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            printf("Timeout waiting for response from %s\n", target_ip);
        } else {
            perror("recvfrom failed");
        }
    }

    // Ensure the socket is closed before returning
    if (close(sockfd) < 0) {
        perror("Error closing socket");
    }

    (*env)->ReleaseStringUTFChars(env, ipAddress, target_ip);
    return JNI_FALSE;
}
