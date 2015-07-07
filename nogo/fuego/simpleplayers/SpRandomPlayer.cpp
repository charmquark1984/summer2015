//----------------------------------------------------------------------------
/** @file SpRandomPlayer.cpp
    See SpRandomPlayer.h */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SpRandomPlayer.h"

#include "GoEyeUtil.h"

//----------------------------------------------------------------------------

int SpRandomMoveGenerator::Score(SgPoint p)
{
    if (m_board.IsLegal(p))
        return 1;
    else
        return INT_MIN;
}

//----------------------------------------------------------------------------

