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
    key = b'\x51\x55\x45\x52\x59\x5f\x53\x54\x52\x49\x4e\x47'.decode('utf-8')
    for val in QS:
        p = subprocess.run(['./cgiprog'], env={key: val},
                stdout=subprocess.PIPE)
        val = hashlib.sha1(p.stdout).hexdigest()
        if val in SUMS:
            print('PASS')
        else:
            print('FAIL')

if __name__ == '__main__':
    main()
