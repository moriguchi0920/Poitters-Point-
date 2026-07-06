//---------------------------------------------------------------------------
//! @file   ProcTiming.h
//! @brief  処理タイミング
//---------------------------------------------------------------------------

#include "ProcTiming.h"

std::string GetProcTimingName(ProcTiming proc)
{
	const std::string func_table[] = {
		"_system_PreUpdate",	  //
		"_system_Update",		  //
		"_system_LateUpdate",	  //
		"_system_PrePhysics",	  //
		"_system_PostPhysics",	  //
		"_system_PostUpdate",	  //
		"_system_PreDraw",		  //
		"_system_Draw",			  //
		"_system_LateDraw",		  //
		"_system_PostDraw",		  //
								  //
		"_system_Shadow",		  //
		"_system_Gbuffer",		  //
		"_system_Light",		  //
		"_system_HDR",			  //
		"_system_Filter",		  //
		"_system_UI",			  //
	};

	static_assert(static_cast<u32>(ProcTiming::NUM) <= static_cast<u32>(sizeof(func_table) / sizeof(func_table[0])));
	assert(static_cast<u32>(proc) < static_cast<u32>(ProcTiming::NUM));

	return func_table[static_cast<int>(proc)];
}
