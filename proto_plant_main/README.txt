* PUBLIC ADDRESSES

|------------------------+-----------+---------+-----------+---------------------------------------------------------|
| Name                   | Block     | Address | Type      | Description                                             |
|------------------------+-----------+---------+-----------+---------------------------------------------------------|
| Line 1 bag counter     | Registers |    0x21 | [0,65535] |                                                         |
| Line 1 grinder control | Registers |    0x32 | [0,255]   | Grinder duty cycle. Only lowest byte meaningful.        |
| Line 1 grinder on      | Coils     |    0xc9 | [0,1]     | 1 if grinder is running. Read only.                     |
| Line 1 main switch     | Registers |    0x34 | [0,255]   | Only low byte used. If true, line is active.            |
| Line 1 oven power      | Registers |    0x30 | [0,3]     | Only two lowest bits are meaningful: off, 3: full power |
| Line 2 bag counter     | Registers |    0x15 | [0,65535] |                                                         |
| Line 2 grinder control | Registers |    0x33 | [0,255]   | Grinder duty cycle. Only lowest byte meaningful.        |
| Line 2 grinder on      | Coils     |    0xca | [0,1]     | 1 if grinder is running. Read only.                     |
| Line 2 main switch     | Registers |    0x35 | [0,255]   | Only low byte used. If true, line is active.            |
| Line 2 oven power      | Registers |    0x31 | [0,3]     | Only two lowest bits are meaningful: off, 3: full power |
| Silo level             | Registers |    0x00 | [0,1000]  | RO. Silo fill level. 0: empty, 1000: 100% full          |
|------------------------+-----------+---------+-----------+---------------------------------------------------------|
