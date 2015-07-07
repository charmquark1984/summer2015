//----------------------------------------------------------------------------
/** @file GoUctFeatures.cpp  */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoUctFeatures.h"

#include "FeBasicFeatures.h"
#include "GoPattern12Point.h"
#include "GoUctAdditiveKnowledgeGreenpeep.h"
#include "GoUctPlayoutPolicy.h"
#include "GoUctPlayoutUtil.h"

namespace {
    
const bool USE_12_POINT_FEATURES = false;



        void Find12PointFeatures(const GoBoard& bd,
                                 GoEvalArray<FeMoveFeatures>& features,
                                 const GoPointList& legalMoves);
void
Find12PointFeatures(const GoBoard& bd,
                    GoEvalArray<FeMoveFeatures>& features,
                    const GoPointList& legalMoves)
{
    /** Index for Feature IDs */
    const int START_INDEX_12_POINT = 3000;

    const SgBlackWhite toPlay = bd.ToPlay();
    const SgBlackWhite opponent = bd.Opponent();
    for (GoPointList::Iterator it(legalMoves); it; ++it)
    {
        const SgPoint p = *it;
        unsigned int context =
        GoPattern12Point::Context(bd, p, toPlay, opponent);
        features[p].Set12PointIndex(START_INDEX_12_POINT + context);
    }
}


    
} // namespace

void GoUctFeatures::
FindAllFeatures(const GoBoard& bd,
                GoUctPlayoutPolicy<GoBoard>& policy,
                FeFullBoardFeatures& f)
{
    SG_UNUSED(policy);
    f.FindAllFeatures();
    if (USE_12_POINT_FEATURES)
        Find12PointFeatures(bd, f.Features(), f.LegalMoves());
}

void GoUctFeatures::
FindMoveFeaturesUI(const GoBoard& bd,
                   GoUctPlayoutPolicy<GoBoard>& policy,
                   SgPoint move,
                   FeMoveFeatures& features)
{
    SG_ASSERT(move != SG_PASS);
    if (! bd.IsLegal(move))
        return;
    FeFullBoardFeatures f(bd);
    FindAllFeatures(bd, policy, f);
    features = f.Features()[move];
}

void GoUctFeatures::WriteFeatures(std::ostream& stream,
                                  GoUctPlayoutPolicy<GoBoard>& policy,
                                  const GoBoard& constBd,
                                  const bool writeComment)
{
    SgPoint chosenMove = constBd.GetLastMove();
    GoModBoard mod(constBd);
    GoBoard& bd = mod.Board();
    if (chosenMove != SG_NULLMOVE)
    {
        bd.Undo();
        FeFullBoardFeatures f(bd);
        FindAllFeatures(bd, policy, f);
        f.WriteNumeric(stream, chosenMove, writeComment);
        bd.Play(chosenMove);
    }
}

//FindMCOwnerFeatures(bd, move, features);
#if UNUSED // TODO
int NuWins()
{
    return 42; // TODO
}

void FindMCOwnerFeatures(const GoBoard& bd, SgPoint move,
                         FeBasicFeatureSet& features)
{
    // TODO run 63 simulations
    SG_UNUSED(bd);
    SG_UNUSED(move);

    FeBasicFeature f = FE_NONE;
    int n = NuWins() / 8;
    switch(n)
    {
        case 0: f = FE_MC_OWNER_1;
            break;
        case 1: f = FE_MC_OWNER_2;
            break;
        case 2: f = FE_MC_OWNER_3;
            break;
        case 3: f = FE_MC_OWNER_4;
            break;
        case 4: f = FE_MC_OWNER_5;
            break;
        case 5: f = FE_MC_OWNER_6;
            break;
        case 6: f = FE_MC_OWNER_7;
            break;
        case 7: f = FE_MC_OWNER_8;
            break;
        default: SG_ASSERT(false);
            break;
    }
    if (f != FE_NONE)
        features.set(f);
}
#endif
