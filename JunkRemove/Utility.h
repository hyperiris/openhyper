#pragma once

// Size of string with out terminator
#define SIZESTR(x) (sizeof(x) - 1)

// Data and function alignment (w/processor pack)
#define ALIGN(_x_) __declspec(align(_x_))

