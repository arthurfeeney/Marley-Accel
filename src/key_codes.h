#ifndef KEY_CODES_H
#define KEY_CODES_H

/*
 * key_code map. The key codes are at the index corressponding to their
 * mouse interrupts. The first value in each subarray is the number of
 * key codes contained in that subarray.
 * Subarrays have 6 elements because at most 5 keys can be held at a time
 * and the first agument is the length of the subarray
 */
const int key_code_map[][6] = {
    {0},                                             // 0x0, release keys
    {1, BTN_LEFT},                                   // 0x1
    {1, BTN_RIGHT},                                  // 0x2
    {2, BTN_LEFT, BTN_RIGHT},                        // 0x3
    {1, BTN_MIDDLE},                                 // 0x4
    {2, BTN_LEFT, BTN_MIDDLE},                       // 0x5
    {2, BTN_RIGHT, BTN_MIDDLE},                      // 0x6
    {3, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE},            // 0x7
    {1, BTN_SIDE},                                   // 0x8
    {2, BTN_LEFT, BTN_SIDE},                         // 0x9
    {2, BTN_RIGHT, BTN_SIDE},                        // 0xA
    {3, BTN_LEFT, BTN_RIGHT, BTN_SIDE},              // 0xB
    {2, BTN_MIDDLE, BTN_SIDE},                       // 0xC
    {3, BTN_LEFT, BTN_MIDDLE, BTN_SIDE},             // 0xD
    {3, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE},            // 0xE
    {4, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE},  // 0xF
    {2, BTN_LEFT, BTN_EXTRA},                        // 0x11
    {2, BTN_RIGHT, BTN_EXTRA},                       // 0x12
    {3, BTN_LEFT, BTN_RIGHT, BTN_EXTRA},             // 0x13
    {2, BTN_MIDDLE, BTN_EXTRA},                      // 0x14
    {3, BTN_LEFT, BTN_MIDDLE, BTN_EXTRA},            // 0x15
    {3, BTN_RIGHT, BTN_MIDDLE, BTN_EXTRA},           // 0x16
    {4, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_EXTRA}, // 0x17
    {2, BTN_SIDE, BTN_EXTRA},                        // 0x18
    {3, BTN_LEFT, BTN_SIDE, BTN_EXTRA},              // 0x19
    {3, BTN_RIGHT, BTN_SIDE, BTN_EXTRA},             // 0x1A
    {4, BTN_LEFT, BTN_RIGHT, BTN_SIDE, BTN_EXTRA},   // 0x1B
    {3, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA},            // 0x1C
    {4, BTN_LEFT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA},  // 0x1D
    {4, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA}, // 0x1E
    {5, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA}, // 0x1F
};

#endif
