#!/usr/bin/env python3

import hashlib
import os
import socket
import sys
import threading

PORT = 32400
SERVERS = [ 'canada', 'cambodia', 'belgium', 'australia',
    'atlanta', 'houston', 'hongkong', 'lasvegas',
    'alabama', 'alaska', 'arizona', 'hawaii' ]

lock = threading.Lock()
status = {}

def check_server(server):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(2)
    s.connect((server, PORT))
    s.send(b'')
    try:
        s.recv(1)
        stat = True
    except socket.timeout:
        stat = False

    with lock:
        status[server] = stat

def user_specific_index(total):
    uid = os.geteuid()
    digest = hashlib.sha1(uid.to_bytes(4, byteorder='big')).hexdigest()
    hashed_int = int(digest[-8:], 16)
    return hashed_int % total

def get_status():
    threads = []
    for server in SERVERS:
        t = threading.Thread(target=check_server, args=(server,))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()

def show_full_status():
    sys.stdout.write('Overall server status:\n')
    i = 0
    for server in SERVERS:
        if status[server]:
            stat = 'OK'
        else:
            stat = 'DOWN'
        sys.stdout.write('%15s: %5s  ' % (server, stat))
        i += 1
        if i % 3 == 0:
            sys.stdout.write('\n')

def show_preferred_server():
    up_servers = [s for s in SERVERS if status[s]]
    index = user_specific_index(len(up_servers))
    sys.stdout.write('Preferred server (for your user): %s\n' % \
            up_servers[index])

def main():
    get_status()
    show_full_status()
    show_preferred_server()

if __name__ == '__main__':
    main()