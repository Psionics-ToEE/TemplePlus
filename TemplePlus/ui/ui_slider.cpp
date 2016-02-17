
#include "stdafx.h"
#include <util/fixes.h>
#include "common.h"
#include <config/config.h>

static class SliderHooks : public TempleFix
{
public: 
	
	const char* name() override { 
		return "Slider UI hooks";
	} 

	void apply() override 
	{
		if (config.usingCo8)
		{

			// ui_slider
			int writeval;
			writeval = 328 + 73;
			write(0x102FB6D8, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102FB6DC, &writeval, sizeof(int));

			writeval = 452 + 73;
			write(0x102FB6E8, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102FB6EC, &writeval, sizeof(int));



			// radial menu slider accept
			writeval = 328 + 73;
			write(0x102F9568, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102F956C, &writeval, sizeof(int));
			// decline
			writeval = 452 + 73;
			write(0x102F9578, &writeval, sizeof(int));

			writeval = 370 + 105;
			write(0x102F957C, &writeval, sizeof(int));
		}
	}
} hooks;