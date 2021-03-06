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
#include "./leds.h"
#include "./ledpwm.h"
#include "./color.h"

#include "fsl_debug_console.h"

#include <algorithm>
#include <math.h>

static constexpr size_t circleLeds = Leds::maxLedsPerPort;

struct Segment {
	size_t count;
	double xmin;
	double ymin;
	double xmax;
	double ymax;
};

struct Strip {
	size_t count;
	Segment segments[10];
};

static constexpr Strip strip_setup[5] = {
{ 9,
  {{ 10,  1335.668701,  1315.000000,  1056.737915,  1004.000000 },
   { 14,  1129.560669,  1004.562500,  1279.919556,   430.748260 },
   { 11,  1066.025635,   717.762146,   625.788574,   879.292603 },
   { 11,  1046.492920,   612.579529,   584.469421,   712.503052 },
   { 14,  1001.417419,   520.169128,   389.142395,   564.496094 },
   { 17,   967.610840,   430.012604,   226.119492,   418.743042 },
   { 20,   873.703613,   316.565643,    16.557043,   238.983093 },
   { 17,  1198.998169,   298.534332,   486.805908,   137.488693 },
   { 20,   358.340851,   114.198257,  -500.000000,    36.000000 },
   {}
  }
},



{ 9,
  {{ 10,  2784.912354,  1357.043457,  3055.364990,  1047.506104 },
   { 14,  2976.139648,  1020.313354,  2873.173584,   445.688568 },
   { 11,  3060.400635,   791.144470,  3486.546875,   982.153870 },
   { 11,  3098.326660,   699.673401,  3547.064697,   822.664185 },
   { 14,  3144.398438,   599.617249,  3735.714844,   692.802979 },
   { 17,  3161.675537,   512.697937,  3882.596680,   561.050659 },
   { 20,  3288.765381,   411.647278,  4145.484131,   333.257050 },
   { 17,  2968.682861,   370.671661,  3674.093750,   255.309570 },
   { 20,  3823.497070,   243.962479,  4680.810547,   322.000000 }
  }
},

{ 5,
  {{ 20,  1280.877441,  1763.843262,   504.679657,  2265.158936 },
   {  9,   957.713379,  2113.153564,   736.230225,  2434.277344 },
   {  9,   982.881897,  2151.406494,   885.227966,  2526.710938 },
   { 10,   868.898560,  1941.011963,   424.140320,  1945.869507 },
   {  9,   834.964050,  1993.380127,   452.329102,  2083.781738 },
   {}, {} , {}, {}, {}
  }
},

{ 5,
  {{ 20,  2786.660645,  1828.090576,  3510.507812,  2375.712402 },
   {  9,  3199.424805,  2085.794922,  3557.824707,  2221.693848 },
   { 10,  3145.060791,  2033.448730,  3570.912354,  2082.774902 },
   {  9,  3092.710205,  2211.627197,  3260.835938,  2561.944092 },
   {  9,  3054.453857,  2259.946777,  3099.757324,  2645.496582 },
   {}, {}, {}, {}, {}
  }
},

{ 8,
  {{  5,  2798.993164,  1571.392822,  2988.260742,  1580.452759 },
   {  5,  2607.979980,  1060.348145,  2744.394043,   944.321594 },
   {  5,  2082.377197,   797.650085,  2083.892090,   613.573914 },
   {  5,  1515.692627,  1005.966736,  1385.408203,   880.976746 },
   {  5,  1290.724731,  1499.879761,  1099.842896,  1495.334717 },
   {  5,  1504.330688,  2070.288574,  1371.773804,  2214.216553 },
   {  5,  2006.053223,  2292.464111,  1994.590210,  2486.981445 },
   {  5,  2532.352799,  2121.449087,  2658.743656,  2266.922313 },
   {}, {}
  }
}

};

Leds &Leds::instance() {
    static Leds leds;
    if (!leds.initialized) {
        leds.initialized = true;
        leds.init();
    }
    return leds;
}

void Leds::convert() {
    static color::convert converter;
    static std::array<uint8_t, maxLedsPerPort * maxBytesPerLed> data;
	auto convertPixel = [this] (uint8_t *ptr, uint32_t type, const led &l) {
		switch(type) {
			case WS2816: {
				color::rgba<uint16_t> pixel(color::rgba<uint16_t>(converter.CIELUV2sRGB(l.col*brightness)).fix_for_ws2816());
				*ptr++ = ( pixel.g >> 8 ) & 0xFF;
				*ptr++ = ( pixel.g >> 0 ) & 0xFF;
				*ptr++ = ( pixel.r >> 8 ) & 0xFF;
				*ptr++ = ( pixel.r >> 0 ) & 0xFF;
				*ptr++ = ( pixel.b >> 8 ) & 0xFF;
				*ptr++ = ( pixel.b >> 0 ) & 0xFF;
			} break;
			case WS2812: {
				color::rgba<uint8_t> pixel(color::rgba<uint8_t>(converter.CIELUV2sRGB(l.col*brightness)));
				*ptr++ = ( pixel.g >> 0 ) & 0xFF;
				*ptr++ = ( pixel.r >> 0 ) & 0xFF;
				*ptr++ = ( pixel.b >> 0 ) & 0xFF;
			} break;
		}
		return ptr;
	};

	for (size_t c = 0; c < totalPortCount; c++) {
		uint8_t *ptr = data.data();
		for (size_t d = 0; d < ports[c].count; d++) {
			ptr = convertPixel(ptr, ports[c].type, ports[c].leds[d]);
		}
		LedsPWMDMA::instance().prepare(c, data.data(), ptr - data.data());
	}
}

std::array<std::array<vector::float4, Leds::maxLedsPerPort>, Leds::maxPorts> Leds::get() const {
	std::array<std::array<vector::float4, maxLedsPerPort>, maxPorts> col;
	for (size_t c = 0; c < maxPorts; c++) {
		for (size_t d = 0; d < maxLedsPerPort; d++) {
			col[c][d] = ports[c].leds[d].col;
		}
	}
	return col; 
}

void Leds::set(const std::array<std::array<vector::float4, Leds::maxLedsPerPort>, Leds::maxPorts> &col) {
	for (size_t c = 0; c < maxPorts; c++) {
		for (size_t d = 0; d < maxLedsPerPort; d++) {
			ports[c].leds[d].col = col[c][d];
		}
	}
}

void Leds::init() {

	// Compute bounds to get normalization factors
	vector::float4 bounds(+10000.0f, +10000.0f, -10000.0f, -10000.0f);

	for (size_t c = 0; c < 5; c++ ) {
		for (size_t d = 0; d < strip_setup[c].count; d++ ) {
			const Segment &s = strip_setup[c].segments[d];
			bounds.x = std::min(bounds.x, float(s.xmin));
			bounds.y = std::min(bounds.y, float(s.ymin));
			bounds.z = std::max(bounds.z, float(s.xmin));
			bounds.w = std::max(bounds.w, float(s.ymin));

			bounds.x = std::min(bounds.x, float(s.xmax));
			bounds.y = std::min(bounds.y, float(s.ymax));
			bounds.z = std::max(bounds.z, float(s.xmax));
			bounds.w = std::max(bounds.w, float(s.ymax));
		}
	}

	float offx = (bounds.x + bounds.z ) / 2.0f;
	float offy = (bounds.y + bounds.w ) / 2.0f;
	float aspx = 1.0f;
	float aspy = offx / offy;
 
	size_t maxLedsOnPorts = 0;

	totalLedCount = 0;
	totalPortCount = 0;
	for (size_t c = 0; c < 5; c++ ) {
		size_t portLedCount = 0;
		ports[c].type = WS2816;
		for (size_t d = 0; d < strip_setup[c].count; d++ ) {
			const Segment &s = strip_setup[c].segments[d];
			for (size_t e = 0; e < s.count; e++ ) {
				// Normalize
				float x = (std::lerp(float(s.xmin), float(s.xmax), float(e)/float(s.count-1)) - offx) / offx;
				float y = (std::lerp(float(s.ymin), float(s.ymax), float(e)/float(s.count-1)) - offy) / offy;
				ports[c].leds[portLedCount].segment = d;
				ports[c].leds[portLedCount].index = portLedCount;
				ports[c].leds[portLedCount].col = { x, y, 0.0f, 0.0f};
				if (c == 4) {
			        float i = float(d) * ( 2.0f * float(pi) / float(strip_setup[c].count) );
					float j = float(d) * ( 1.0f / float(strip_setup[c].count) );
					ports[c].leds[portLedCount].map = { x, y, i, j };
				} else {
					ports[c].leds[portLedCount].map = { x, y, x / aspx, y / aspy,};
				}
				totalLedCount++;
				portLedCount++;
			}
		}
		ports[totalPortCount].count = portLedCount;
		maxLedsOnPorts = std::max(maxLedsOnPorts, portLedCount);
		totalPortCount++;
	}

    float i = - float(pi) * 0.5f;
    float j = 0;
	ports[totalPortCount].type = WS2812;
    for (size_t c = 0; c < circleLeds; c++) {
		ports[totalPortCount].leds[c].index = c;
		ports[totalPortCount].leds[c].col = { cosf(i), -sinf(i),  i,  j };
		ports[totalPortCount].leds[c].map = { cosf(i), -sinf(i),  i,  j };
        i += 2.0f * float(pi) / float(circleLeds);
        j += 1.0f / float(circleLeds);
		totalLedCount++;
    }
    ports[totalPortCount].count = circleLeds;
    totalPortCount++;
	maxLedsOnPorts = std::max(maxLedsOnPorts, circleLeds);

	PRINTF("Total LEDs %d\r\n", int(totalLedCount));
	PRINTF("Max LEDs on a port %d\r\n", int(maxLedsOnPorts));
}
