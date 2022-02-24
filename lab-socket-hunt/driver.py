#!/usr/bin/python3

import argparse
import hashlib
import re
import subprocess
import sys
import time

NUM_LEVELS = 5
SEEDS = [7719, 33833, 20468, 19789, 59455]
CLIENT = './treasure_hunter'
BYTES_MINUS_CHUNK = 8

SUMS = ['127624217659f4ba97d5457391edc8f60758714b',
        '2483b89fefaee5a83c25ba92dda9bd004357d6b1',
        '285e8e43bf9d8b7204f6972a3be88b8a599b068d',
        '3334488d5b819492e13335105df59164acbf98a0',
        '384835f2469dbb37a6eb0bbf6f66e45677f61423',
        '3f362e32653be4ab829fd7b9838fdfe71c01385d',
        '5bc7244d527b79d2625d51ab514f0412d22acc2c',
        '7514a7a267acdfed8de2bcf0729a2037035c3acd',
        'b156795f7b657f2fe33639b4f0bb3f6193960f79',
        'c2f7e1078c91ab8ce0674680e7d2e7ab9a335a06',
        'c41d89fe9ebac2cf06fc8e3f0f0f8335ec9dce8f',
        'c657aeed6d2c2eeb40382898cdd4f3d25182c719',
        'd29e5524dda590d2eaf097e9e32b53cb9770e965',
        'dec427a457cc1cba9533630a2c511cd5f21aa1f0',
        'fee33676b4c20e5b267ba40b2eba3c2c7ee3d260',
        'ffb54044122cb1883513849d396cf144af3e0ed4']

RECV_RE = re.compile('^recv(from)?.* = (\d+)$')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', action='store', type=str)
    parser.add_argument('port', action='store', type=int)
    parser.add_argument('level', action='store', type=int,
        nargs='?', choices=range(NUM_LEVELS))
    args = parser.parse_args(sys.argv[1:])

    if args.level is None:
        levels = range(NUM_LEVELS)
    else:
        levels = [args.level]

    score = 0
    max_score = 0
    for level in levels:
        sys.stdout.write(f'Testing level {level}:\n')
        for seed in SEEDS:
            max_score += 4
            sys.stdout.write(f'    Seed %5d:' % (seed))
            sys.stdout.flush()
            cmd = ['strace', '-e', 'trace=%network',
                    CLIENT, args.server, str(args.port), str(level), str(seed)]
            p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            treasure = p.stdout
            strace_output = p.stderr
            h = hashlib.sha1(treasure).hexdigest()

            tot_bytes = 0
            output = strace_output.decode('utf-8').strip()
            for line in output.splitlines():
                m = RECV_RE.search(line)
                # skip DNS lookups
                if 'htons(53)' in line:
                    continue
                if m is not None:
                    received_bytes = int(m.group(2))
                    if received_bytes > 1:
                        tot_bytes += received_bytes - BYTES_MINUS_CHUNK
            
            allowed_lengths = [tot_bytes]
            if treasure[-1] == b'\n':
                allowed_lengths += 1

            if h in SUMS and tot_bytes in allowed_lengths:
                score += 4
                sys.stdout.write(f' PASSED\n')
            else:
                sys.stdout.write(f' FAILED\n')
            
    print(f'Score: {score}/{max_score}')
            
if __name__ == '__main__':
    main()
