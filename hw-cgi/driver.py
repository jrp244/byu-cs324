#!/usr/bin/env python3

import hashlib
import subprocess

QS = ['foo=bar', 'abc=123&def=456']
SUMS = [
        '990fa95231a668077ead7d8c4d507ec68025195f',
        'aebcb07b31ab0de071b43097fddc108b4567a9bf',
        '74fecf3429321d8e3f034c5684d7948d78b37f94',
        '19fa1bc24ef9d9d0ddda9ee6a6168c393fae5e27',
]


def main():
    for qs in QS:
        p = subprocess.run(['./cgiprog'], env={"QUERY_STRING": qs},
                stdout=subprocess.PIPE)
        val = hashlib.sha1(p.stdout).hexdigest()
        if val in SUMS:
            print('PASS')
        else:
            print('FAIL')

if __name__ == '__main__':
    main()
