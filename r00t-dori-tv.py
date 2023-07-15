#!/usr/bin/env python3
from telnetlib import Telnet
from socket import socket, AF_INET, SOCK_STREAM
from time import sleep

IP = "10.2.12.29"

def is_port_open(ip: str, port: int):
    sock = socket(AF_INET, SOCK_STREAM)
    result = sock.connect_ex((ip, port))
    sock.close()
    return result == 0

def telnet_bootstrap_tv(telnet: Telnet):
    telnet.read_until(b"#")
    telnet.write(b"/var/lib/webosbrew/startup.sh && killall telnetd\n")
    telnet.read_all()

def auto_tv(ip: str):
    if is_port_open(ip, 22):
        return True

    if is_port_open(ip, 23):
        with Telnet(IP, 23) as telnet:
            telnet_bootstrap_tv(telnet)

    return is_port_open(ip, 22)

def auto_tv_loop(ip: str):
    for _ in range(60):
        if auto_tv(ip):
            return True
        sleep(1)
    return False

if __name__ == "__main__":
    if not auto_tv_loop(IP):
        exit(1)
