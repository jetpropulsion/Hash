# Hash
Simple hashing algorithms and partial reimplementation in assembly language.
This is eight years old code, so accept that things are changed and modify it accordingly, if really needed.
Names of the hashing algorithms correspond to their original form, as found on the respective authors' sites.

NOTE: This is Microsoft preferred "C++" style as found in their code examples, in fact closer to pure C. Project file and solution is upgraded from previous version of Microsoft Visual Studio to Visual Studio Community edition 2015. If you need to compile it with older compiler, please create new project, adding Hash.cpp and Hash_x86.asm to it.

Algorithms covered:

1. Simple sum of octets read
2. Murmur (and x86 reimplementation)
3. Jenkins (and x86 reimplementation)
4. DJB2 (and x86 reimplementation)
5. SDBM
6. Rotating
7. EMACS
8. PJW
9. AVL
10. CRC32 (and x86 reimplementation)
11. FNV1a (and x86 reimplementation)
