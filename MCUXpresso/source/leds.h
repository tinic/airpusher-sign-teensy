/*
Copyright 2021 Tinic Uro

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef LEDS_H_
#define LEDS_H_

#include <array>

#include "./vector.h"

class Leds {
public:
    static Leds &instance();

    void convert();

    size_t portCount() const { return totalPortCount; }
    size_t portLedCount(size_t p) const { return ports[p].count; }

    size_t seg(size_t p, size_t i) { return ports[p].leds[i].segment; }
    vector::float4 &map(size_t p, size_t i) { return ports[p].leds[i].map; }
    vector::float4 &col(size_t p, size_t i) { return ports[p].leds[i].col; }

    void setCol(size_t p, size_t i, const vector::float4 &col) {
    	ports[p].leds[i].col = col;
    }

    static constexpr size_t maxLedsPerPort = 144;
    static constexpr size_t maxPorts = 6; // Up to 8 on Teensy 4.0
    static constexpr size_t maxBytesPerLed = 6; // WS2816 has 6 bytes, WS2812 has 4

private:

    float brightness = 1.0f;

    static constexpr float pi = 3.14159265359f;

    enum ledType {
    	WS2816,
		WS2812
    };


    void init();
    bool initialized = false;

    struct led {
    	vector::float4 col;
    	vector::float4 map;
    	size_t 		   segment;
    	size_t         index;
    };

    struct port {
    	uint32_t type;
    	size_t   count;
        std::array<led, maxLedsPerPort> leds;
    };

    std::array<port, maxPorts> ports;

    size_t totalPortCount = 0;
    size_t totalLedCount = 0;
};

#endif /* LEDS_H_ */
