#ifndef __RKEY__H__
#define __RKEY__H__

/*
 * ather
 * The University of Queensland, 2022
 */

const uint64_t N = 249;

const uint64_t D = 15;

const uint64_t E = 11;

uint64_t
powr(uint64_t x, uint64_t y, uint64_t m)
{
        uint64_t z = x;
        while (y > 1) {
                z %= m;
                z *= x;
                y -= 1;
        }
        return z % m;
}

#endif //__RKEY__H__