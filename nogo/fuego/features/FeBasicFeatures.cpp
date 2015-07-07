//----------------------------------------------------------------------------
/** @file FeBasicFeatures.cpp  */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "FeBasicFeatures.h"

#include <iostream>
#include <string>
#include "FePatternBase.h"
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoOpeningKnowledge.h"
#include "GoPattern12Point.h"
#include "GoPattern3x3.h"
#include "GoSafetySolver.h"
#include "GoSetupUtil.h"
#include "SgPointSet.h"
#include "SgWrite.h"

using SgPointUtil::Pt;

// @todo  NEED TO MAKE SURE that move is evaluated with last move info intact.

//----------------------------------------------------------------------------
namespace {



inline FeBasicFeature ComputeFeature(FeBasicFeature baseFeature,
                                     int baseValue, int value)
{
    return static_cast<FeBasicFeature>(static_cast<int>(baseFeature)
                                       + value - baseValue);
}



    



void FindCfgFeatures(const GoBoard& bd, SgPoint focus,
                     FeBasicFeature baseFeature, int baseDistance,
                     const GoPointList& legalBoardMoves,
                     GoEvalArray<FeMoveFeatures>& features)
{
    const int MAX_DIST = 3;
    SgPointArray<int> dist = GoBoardUtil::CfgDistance(bd, focus, MAX_DIST);
    for (GoPointList::Iterator it(legalBoardMoves); it; ++it)
    {
        const SgPoint p = *it;
        SG_ASSERT(p != SG_PASS);
        SG_ASSERT(dist[p] >= baseDistance);
        SG_ASSERT(  dist[p] <= MAX_DIST
                 || dist[p] == std::numeric_limits<int>::max());
        const int distance = std::min(dist[p], MAX_DIST + 1);
        const FeBasicFeature f = ComputeFeature(baseFeature,
                                                baseDistance, distance);
        features[*it].Set(f);
    }
}

void FindCfgFeatures(const GoBoard& bd,
                     const GoPointList& legalBoardMoves,
                     GoEvalArray<FeMoveFeatures>& features)
{
    const SgPoint lastMove = bd.GetLastMove();
    if (! SgIsSpecialMove(lastMove))
        FindCfgFeatures(bd, lastMove,
                        FE_CFG_DISTANCE_LAST_1, 1,
                        legalBoardMoves,
                        features);

    const SgPoint lastMove2 = bd.Get2ndLastMove();
    if (! SgIsSpecialMove(lastMove2))
        FindCfgFeatures(bd, lastMove2,
                        FE_CFG_DISTANCE_LAST_OWN_0, 0,
                        legalBoardMoves,
                        features);
}

void FindCornerMoveFeatures(const GoBoard& bd,
                GoEvalArray<FeMoveFeatures>& features)
{
    const std::vector<SgPoint>
    corner(GoOpeningKnowledge::FindCornerMoves(bd));
    for (std::vector<SgPoint>::const_iterator it
         = corner.begin(); it != corner.end(); ++it)
    {
        features[*it].Set(FE_CORNER_OPENING_MOVE);
    }
}

void FindGamePhaseFeature(const GoBoard& bd,
                          const GoPointList& legalBoardMoves,
                          GoEvalArray<FeMoveFeatures>& features)
{
    const int phase = std::min(12, bd.MoveNumber() / 30 + 1);
    FeBasicFeature f = ComputeFeature(FE_GAME_PHASE_1, 1, phase);
    SG_ASSERT(f >= FE_GAME_PHASE_1);
    SG_ASSERT(f <= FE_GAME_PHASE_12);
    for (GoPointList::Iterator it(legalBoardMoves); it; ++it)
        features[*it].Set(f);
}





void FindLinePosFeatures(const GoBoard& bd,
                         const GoPointList& legalBoardMoves,
                         GoEvalArray<FeMoveFeatures>& features)
{
    for (GoPointList::Iterator it(legalBoardMoves); it; ++it)
    {
        const int line = std::min(5, bd.Line(*it));
        FeBasicFeature f = ComputeFeature(FE_LINE_1, 1, line);
        SG_ASSERT(f >= FE_LINE_1);
        SG_ASSERT(f <= FE_LINE_5_OR_MORE);
        features[*it].Set(f);

        const int pos = std::min(10, bd.Pos(*it));
        FeBasicFeature fp = ComputeFeature(FE_POS_1, 1, pos);
        SG_ASSERT(fp >= FE_POS_1);
        SG_ASSERT(fp <= FE_POS_10);
        features[*it].Set(fp);
    }
}


const int EDGE_START_INDEX_3x3 = 1000;
const int NU_2x3_EDGE_FEATURES = 135;
const int CENTER_START_INDEX_3x3 = 1200;
const int NU_3x3_CENTER_FEATURES = 954;
    
bool Is2x3EdgeID(int id)
{
    return id >= EDGE_START_INDEX_3x3
    && id < EDGE_START_INDEX_3x3 + NU_2x3_EDGE_FEATURES;
}

bool Is3x3CenterID(int id)
{
    return id >= CENTER_START_INDEX_3x3
        && id < CENTER_START_INDEX_3x3 + NU_3x3_CENTER_FEATURES;
}

inline int Find2x3EdgeFeature(const GoBoard& bd, SgPoint move)
{
    int code = GoPattern3x3::CodeOfEdgeNeighbors(bd, move);
    code = GoPattern3x3::Map2x3EdgeCode(code, bd.ToPlay());
    SG_ASSERT(EDGE_START_INDEX_3x3 + code < CENTER_START_INDEX_3x3);
    return EDGE_START_INDEX_3x3 + code;
}

inline int Find3x3CenterFeature(const GoBoard& bd, SgPoint move)
{
    int code = GoPattern3x3::CodeOf8Neighbors(bd, move);
    code = GoPattern3x3::Map3x3CenterCode(code, bd.ToPlay());
    SG_ASSERT(code < 1000);
    return CENTER_START_INDEX_3x3 + code;
}

inline int Find3x3Feature(const GoBoard& bd, SgPoint p)
{
    return bd.Pos(p) == 1  ? INVALID_PATTERN_INDEX
         : bd.Line(p) == 1 ? Find2x3EdgeFeature(bd, p)
                           : Find3x3CenterFeature(bd, p);
}

void Write3x3(std::ostream& stream, int index)
{
    SG_ASSERT(index != INVALID_PATTERN_INDEX);
    if (index < CENTER_START_INDEX_3x3)
        GoPattern3x3::Write2x3EdgePattern(stream,
            GoPattern3x3::DecodeEdgeIndex(index - EDGE_START_INDEX_3x3));
    else
        GoPattern3x3::Write3x3CenterPattern(stream,
            GoPattern3x3::DecodeCenterIndex(index - CENTER_START_INDEX_3x3));
}

int Distance(SgPoint p1, SgPoint p2)
{
    SG_ASSERT(! SgIsSpecialMove(p1));
    SG_ASSERT(! SgIsSpecialMove(p2));
    int dx = abs(SgPointUtil::Col(p1) - SgPointUtil::Col(p2));
    int dy = abs(SgPointUtil::Row(p1) - SgPointUtil::Row(p2));
    return dx + dy + std::max(dx, dy);
}

void SetDistancesLastMove(const SgPoint lastMove,
                          const GoPointList& legalBoardMoves,
                          GoEvalArray<FeMoveFeatures>& features)
{
    for (GoPointList::Iterator it(legalBoardMoves); it; ++it)
    {
        const int distance = Distance(*it, lastMove);
        SG_ASSERT(distance >= 2);
        if (distance <= 17)
        {
            FeBasicFeature f = ComputeFeature(FE_DIST_PREV_2, 2, distance);
            features[*it].Set(f);
        }
    }
}
    
void SetDistances2ndLastMove(const SgPoint lastMove2,
                          const GoPointList& legalBoardMoves,
                          GoEvalArray<FeMoveFeatures>& features)
{
    for (GoPointList::Iterator it(legalBoardMoves); it; ++it)
    {
        const int distance = Distance(*it, lastMove2);
        if (distance <= 17)
        {
            SG_ASSERT(distance >= 2);
            FeBasicFeature f =
            ComputeFeature(FE_DIST_PREV_OWN_2, 2, distance);
            features[*it].Set(f);
        }
    }
}

void FindDistPrevMoveFeatures(const GoBoard& bd,
                              const GoPointList& legalBoardMoves,
                              GoEvalArray<FeMoveFeatures>& features)
{
    const SgPoint lastMove = bd.GetLastMove();
    if (! SgIsSpecialMove(lastMove))
        SetDistancesLastMove(lastMove, legalBoardMoves, features);

    const SgPoint lastMove2 = bd.Get2ndLastMove();
    if (! SgIsSpecialMove(lastMove2))
        SetDistances2ndLastMove(lastMove2, legalBoardMoves, features);
}

int FindClosestStoneDistance(const GoBoard& bd,
                             SgPoint empty, SgBlackWhite color)
{
    int distance = 99999;

    for (GoBoard::Iterator it(bd); it; ++it) // @todo: slow, loop from
                                             // closest to furtherst instead
        if (  bd.GetColor(*it) == color
           && Distance(empty, *it) < distance)
        {
            distance = Distance(empty, *it);
        }

    SG_ASSERT(distance < 99999);
    return distance;
}

void FindClosestDistanceFeaturesForColor(const GoBoard& bd,
                                         SgBlackWhite color,
                                         const GoPointList& legalBoardMoves,
                                         GoEvalArray<FeMoveFeatures>& features,
                                         FeBasicFeature baseFeature)
{
    if (bd.All(color).IsEmpty())
        return;

    for (GoPointList::Iterator it(legalBoardMoves); it; ++it)
    {
        int distance = FindClosestStoneDistance(bd, *it, color);
        SG_ASSERT(distance >= 2);
        if (distance > MAX_CLOSEST_DISTANCE)
            distance = MAX_CLOSEST_DISTANCE;
        const FeBasicFeature f = ComputeFeature(baseFeature, 2, distance);
        features[*it].Set(f);
    }
}

void FindClosestDistanceFeatures(const GoBoard& bd,
                                 const GoPointList& legalBoardMoves,
                                 GoEvalArray<FeMoveFeatures>& features)
{
    FindClosestDistanceFeaturesForColor(bd, bd.ToPlay(),
                                   legalBoardMoves, features,
                                   FE_DIST_CLOSEST_OWN_STONE_2);
    FindClosestDistanceFeaturesForColor(bd, bd.Opponent(),
                                        legalBoardMoves, features,
                                        FE_DIST_CLOSEST_OPP_STONE_2);
}

} // namespace

//----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, FeBasicFeature f)
{
    static const char* s_string[_NU_FE_FEATURES] =
    {
        "FE_LINE_1",
        "FE_LINE_2",
        "FE_LINE_3",
        "FE_LINE_4",
        "FE_LINE_5+",
        "FE_DIST_PREV_2", // d(dx,dy) = |dx|+|dy|+max(|dx|,|dy|)
        "FE_DIST_PREV_3",
        "FE_DIST_PREV_4",
        "FE_DIST_PREV_5",
        "FE_DIST_PREV_6",
        "FE_DIST_PREV_7",
        "FE_DIST_PREV_8",
        "FE_DIST_PREV_9",
        "FE_DIST_PREV_10",
        "FE_DIST_PREV_11",
        "FE_DIST_PREV_12",
        "FE_DIST_PREV_13",
        "FE_DIST_PREV_14",
        "FE_DIST_PREV_15",
        "FE_DIST_PREV_16",
        "FE_DIST_PREV_17",
        "FE_DIST_PREV_OWN_2",
        "FE_DIST_PREV_OWN_3",
        "FE_DIST_PREV_OWN_4",
        "FE_DIST_PREV_OWN_5",
        "FE_DIST_PREV_OWN_6",
        "FE_DIST_PREV_OWN_7",
        "FE_DIST_PREV_OWN_8",
        "FE_DIST_PREV_OWN_9",
        "FE_DIST_PREV_OWN_10",
        "FE_DIST_PREV_OWN_11",
        "FE_DIST_PREV_OWN_12",
        "FE_DIST_PREV_OWN_13",
        "FE_DIST_PREV_OWN_14",
        "FE_DIST_PREV_OWN_15",
        "FE_DIST_PREV_OWN_16",
        "FE_DIST_PREV_OWN_17",
        "FE_POS_1", // Position of a point p according to GoBoard::Pos(p)
        "FE_POS_2",
        "FE_POS_3",
        "FE_POS_4",
        "FE_POS_5",
        "FE_POS_6",
        "FE_POS_7",
        "FE_POS_8",
        "FE_POS_9",
        "FE_POS_10",
        "FE_GAME_PHASE_1", // Game phase as in Wistuba - 30 moves per phase
        "FE_GAME_PHASE_2",
        "FE_GAME_PHASE_3",
        "FE_GAME_PHASE_4",
        "FE_GAME_PHASE_5",
        "FE_GAME_PHASE_6",
        "FE_GAME_PHASE_7",
        "FE_GAME_PHASE_8",
        "FE_GAME_PHASE_9",
        "FE_GAME_PHASE_10",
        "FE_GAME_PHASE_11",
        "FE_GAME_PHASE_12",
        "FE_CORNER_OPENING_MOVE",
        "FE_CFG_DISTANCE_LAST_1",
        "FE_CFG_DISTANCE_LAST_2",
        "FE_CFG_DISTANCE_LAST_3",
        "FE_CFG_DISTANCE_LAST_4_OR_MORE",
        "FE_CFG_DISTANCE_LAST_OWN_0",
        "FE_CFG_DISTANCE_LAST_OWN_1",
        "FE_CFG_DISTANCE_LAST_OWN_2",
        "FE_CFG_DISTANCE_LAST_OWN_3",
        "FE_CFG_DISTANCE_LAST_OWN_4_OR_MORE",
        "FE_DIST_CLOSEST_OWN_STONE_2",
        "FE_DIST_CLOSEST_OWN_STONE_3",
        "FE_DIST_CLOSEST_OWN_STONE_4",
        "FE_DIST_CLOSEST_OWN_STONE_5",
        "FE_DIST_CLOSEST_OWN_STONE_6",
        "FE_DIST_CLOSEST_OWN_STONE_7",
        "FE_DIST_CLOSEST_OWN_STONE_8",
        "FE_DIST_CLOSEST_OWN_STONE_9",
        "FE_DIST_CLOSEST_OWN_STONE_10",
        "FE_DIST_CLOSEST_OWN_STONE_11",
        "FE_DIST_CLOSEST_OWN_STONE_12",
        "FE_DIST_CLOSEST_OWN_STONE_13",
        "FE_DIST_CLOSEST_OWN_STONE_14",
        "FE_DIST_CLOSEST_OWN_STONE_15",
        "FE_DIST_CLOSEST_OWN_STONE_16",
        "FE_DIST_CLOSEST_OWN_STONE_17",
        "FE_DIST_CLOSEST_OWN_STONE_18",
        "FE_DIST_CLOSEST_OWN_STONE_19",
        "FE_DIST_CLOSEST_OWN_STONE_20_OR_MORE",
        "FE_DIST_CLOSEST_OPP_STONE_2",
        "FE_DIST_CLOSEST_OPP_STONE_3",
        "FE_DIST_CLOSEST_OPP_STONE_4",
        "FE_DIST_CLOSEST_OPP_STONE_5",
        "FE_DIST_CLOSEST_OPP_STONE_6",
        "FE_DIST_CLOSEST_OPP_STONE_7",
        "FE_DIST_CLOSEST_OPP_STONE_8",
        "FE_DIST_CLOSEST_OPP_STONE_9",
        "FE_DIST_CLOSEST_OPP_STONE_10",
        "FE_DIST_CLOSEST_OPP_STONE_11",
        "FE_DIST_CLOSEST_OPP_STONE_12",
        "FE_DIST_CLOSEST_OPP_STONE_13",
        "FE_DIST_CLOSEST_OPP_STONE_14",
        "FE_DIST_CLOSEST_OPP_STONE_15",
        "FE_DIST_CLOSEST_OPP_STONE_16",
        "FE_DIST_CLOSEST_OPP_STONE_17",
        "FE_DIST_CLOSEST_OPP_STONE_18",
        "FE_DIST_CLOSEST_OPP_STONE_19",
        "FE_DIST_CLOSEST_OPP_STONE_20_OR_MORE",
        "FE_NONE"
    };
    SG_ASSERT(f >= FE_LINE_1);
    SG_ASSERT(f < _NU_FE_FEATURES);
    stream << s_string[f];
    return stream;
}

//----------------------------------------------------------------------------

std::ostream& FeFeatures::operator<<(std::ostream& stream,
                         const FeFeatures::FeEvalDetail& f)
{
    stream << '(';
    WriteFeatureFromID(stream, f.m_feature);
    stream << ", w = " << std::setprecision(2) << f.m_w
           << ", v_sum = " << f.m_v_sum << ')';
    return stream;
}

//----------------------------------------------------------------------------

float FeFeatures::EvaluateActiveFeatures(const FeActiveArray& active,
                                         size_t nuActive,
                                         const FeFeatureWeights& weights)
{
    float value = 0.0;
    for (FeActiveIterator it = active.begin();
                          it < active.begin() + nuActive; ++it)
    {
        value += weights.m_w[*it];
        for (FeActiveIterator it2 = it + 1;
                              it2 < active.begin() + nuActive; ++it2)
            value += weights.Combine(*it, *it2);
    }
    return value;
}

float FeFeatures::EvaluateMoveFeatures(const FeMoveFeatures& features,
                                       const FeFeatureWeights& weights)
{
    FeActiveArray active;
    const size_t nuActive = features.ActiveFeatures(active);
    return EvaluateActiveFeatures(active, nuActive, weights);
}

std::vector<FeFeatures::FeEvalDetail>
FeFeatures::EvaluateMoveFeaturesDetail(const FeMoveFeatures& features,
                                       const FeFeatureWeights& weights)
{
    FeActiveArray active;
    const size_t nuActive = features.ActiveFeatures(active);
    std::vector<FeFeatures::FeEvalDetail> detail;
    for (FeActiveIterator it = active.begin();
                          it < active.begin() + nuActive; ++it)
    {
        const bool outOfBounds =
            static_cast<size_t>(*it) >= weights.m_w.size();
        if (outOfBounds)
        {
            //SgDebug() << "EvaluateMoveFeaturesDetail: skipping feature "
            //          << *it << '\n';
        }
        const float w = outOfBounds ? 0 : weights.m_w[*it];
        float v = 0.0;
        if (! outOfBounds)
            for (FeActiveIterator it2 = active.begin();
                                  it2 < active.begin() + nuActive; ++it2)
                if (it != it2)
                    v += weights.Combine(*it, *it2);
        detail.push_back(FeFeatures::FeEvalDetail(*it, w, v/2));
    }
    return detail;
}

void FeFeatures::FindBasicMoveFeaturesUI(const GoBoard& bd, SgPoint move,
                                         FeBasicFeatureSet& features)
{
    FeFullBoardFeatures f(bd);
    f.FindAllFeatures();
    features = f.BasicFeatures(move);
}

int FeFeatures::Get3x3Feature(const GoBoard& bd, SgPoint p)
{
    return Find3x3Feature(bd, p);
}

bool FeFeatures::IsBasicFeatureID(int id)
{
    return id >= 0 && id < _NU_FE_FEATURES;
}
    
bool FeFeatures::Is3x3PatternID(int id)
{
    return Is2x3EdgeID(id) || Is3x3CenterID(id);
}

void FeFeatures::WriteEvalDetail(std::ostream& stream,
                     const std::vector<FeEvalDetail>& detail)
{
    float w = 0;
    float v = 0;
    for (std::vector<FeFeatures::FeEvalDetail>::const_iterator it
         = detail.begin(); it != detail.end(); ++it)
    {
        stream << *it << '\n';
        w += (*it).m_w;
        v += (*it).m_v_sum;
    }
    stream << " Total w = " << w << " + v = " << v << " = " << w + v << '\n';
}

static bool IsPattern12PointID(int id) // @todo
{
    SG_UNUSED(id);
    return true;
}

void FeFeatures::WriteFeatureFromID(std::ostream& stream, int id)
{
    if (IsBasicFeatureID(id))
        stream << static_cast<FeBasicFeature>(id);
    else if (Is3x3PatternID(id))
        Write3x3(stream, id);
    else if (IsPattern12PointID(id))
        GoPattern12Point::PrintContext(id, stream);
    else
        SG_ASSERT(false);
}

void FeFeatures::WriteFeatureSet(std::ostream& stream,
                                 SgPoint move,
                                 const FeBasicFeatureSet& features)
{
    stream << SgWritePoint(move);
    for (int f = FE_LINE_1; f < _NU_FE_FEATURES; ++f)
        if (features.test(f))
            stream << ' ' << f;
}

void FeFeatures::WriteFeatureSetAsText(std::ostream& stream,
                                       SgPoint move,
                                       const FeBasicFeatureSet& features)
{
    stream << SgWritePoint(move);
    for (int f = FE_LINE_1; f < _NU_FE_FEATURES; ++f)
    {
        if (features.test(f))
            stream << ' ' << static_cast<FeBasicFeature>(f);
    }
}

//-------------------------------------
    

namespace {
    
void WriteFeatureSetNumeric(std::ostream& stream,
                     const FeBasicFeatureSet& features)
{
    for (int f = FE_LINE_1; f < _NU_FE_FEATURES; ++f)
        if (features.test(f))
            stream << ' ' << f;
}
} // namespace

//----------------------------------------------------------------------------

size_t FeMoveFeatures::ActiveFeatures(FeActiveArray& active) const
{
    size_t nuActive = 0;
    for (int i = 0; i < _NU_FE_FEATURES; ++i)
        if (m_basicFeatures.test(i))
        {
            active[nuActive] = i;
            if (++nuActive >= MAX_ACTIVE_LENGTH)
                return nuActive;
        }
    // invalid for pass move and (1,1) points
    if (m_3x3Index != INVALID_PATTERN_INDEX)
    {
        SG_ASSERT(nuActive < MAX_ACTIVE_LENGTH);
        active[nuActive] = m_3x3Index;
        ++nuActive;
    }
    if (m_12PointIndex != INVALID_PATTERN_INDEX)
    {
        SG_ASSERT(nuActive < MAX_ACTIVE_LENGTH);
        active[nuActive] = m_12PointIndex;
        ++nuActive;
    }
    return nuActive;
}

void FeMoveFeatures::FindMoveFeatures(const GoBoard& bd, SgPoint move)
{
    m_3x3Index = Find3x3Feature(bd, move);
}


void FeMoveFeatures::WriteNumeric(std::ostream& stream,
                                  const int isChosen,
                                  const int moveNumber,
                                  const bool writeComment) const
{
    const int SHAPE_SIZE = 3; // TODO make this variable
                              // when big pattern features are implemented
    stream << isChosen;
    WriteFeatureSetNumeric(stream, m_basicFeatures);
    WritePatternFeatureIndex(stream);
    if (writeComment)
        stream << " #0_" << moveNumber << ' ' << SHAPE_SIZE;
    stream << '\n';
}

void FeMoveFeatures::WritePatternFeatureIndex(std::ostream& stream) const
{
    if (m_3x3Index != INVALID_PATTERN_INDEX)
        stream << ' ' << m_3x3Index;
    if (m_12PointIndex != INVALID_PATTERN_INDEX)
        stream << ' ' << m_12PointIndex;
}

void FeMoveFeatures::WritePatternFeatures(std::ostream& stream) const
{
    if (m_12PointIndex != INVALID_PATTERN_INDEX)
    {
        stream << " m_12PointIndex " << m_12PointIndex;
        GoPattern12Point::PrintContext(m_12PointIndex, stream);
    }
    if (m_3x3Index != INVALID_PATTERN_INDEX)
    {
        stream << " 3x3-index " << m_3x3Index;
        Write3x3(stream, m_3x3Index);
    }
}

void FeMoveFeatures::WriteFeatures(std::ostream& stream,
                                   SgPoint move) const
{
    FeFeatures::WriteFeatureSetAsText(stream, move, m_basicFeatures);
    WritePatternFeatures(stream);
    stream << '\n';
}

//----------------------------------------------------------------------------
GoEvalArray<float> FeFullBoardFeatures::
EvaluateFeatures(const FeFeatureWeights& weights) const
{
    GoEvalArray<float> eval(0);
    for (GoPointList::Iterator it(m_legalMoves); it; ++it)
            eval[*it] = FeFeatures::EvaluateMoveFeatures(m_features[*it], weights);
    return eval;
}

void FeFullBoardFeatures::FindAllFeatures()
{
    for (GoPointList::Iterator it(m_legalMoves); it; ++it)
    {
        SG_ASSERT(m_bd.IsLegal(*it));
        m_features[*it].FindMoveFeatures(m_bd, *it);
    }
    FindFullBoardFeatures();
}

void FeFullBoardFeatures::FindFullBoardFeatures()
{
    if (m_bd.Size() >= 15)
    {
        FindCornerMoveFeatures(m_bd, m_features);
    }
    FindGamePhaseFeature(m_bd, m_legalMoves, m_features);
    FindDistPrevMoveFeatures(m_bd, m_legalMoves, m_features);
    FindLinePosFeatures(m_bd, m_legalMoves, m_features);
    FindCfgFeatures(m_bd, m_legalMoves, m_features);
    FindClosestDistanceFeatures(m_bd, m_legalMoves, m_features);
}

void FeFullBoardFeatures::WriteBoardFeatures(std::ostream& stream) const
{
    for (GoPointList::Iterator it(m_legalMoves); it; ++it)
    {
        SG_ASSERT(m_bd.IsLegal(*it));
        m_features[*it].WriteFeatures(stream, *it);
    }
}

void FeFullBoardFeatures::WriteNumeric(std::ostream& stream,
             const SgPoint chosenMove,
             const bool writeComment) const
{
    const int moveNumber = m_bd.MoveNumber() + 1; // + 1 because we did undo
    for (GoPointList::Iterator it(m_legalMoves); it; ++it)
    {
        SG_ASSERT(m_bd.IsLegal(*it));
        if (*it != chosenMove)
            m_features[*it].WriteNumeric(stream, 0, moveNumber, writeComment);
    }
    m_features[chosenMove].WriteNumeric(stream, 1, moveNumber, writeComment);
}

//----------------------------------------------------------------------------
