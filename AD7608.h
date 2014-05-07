#ifndef __AD7608_H__
#define __AD7608_H__

#include <stdbool.h>

bool InitializeAD7608();
void SampleAD7608(int channels[static 8]);

#endif

