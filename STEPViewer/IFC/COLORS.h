#pragma once


struct COLOR
{
	unsigned char		R, G, B, A;
};

struct ENTITY_COLOR
{
	unsigned char		R, G, B, A;
	wchar_t				* entityName;
};


#define			ENTITY_COLOR_CNT	2
