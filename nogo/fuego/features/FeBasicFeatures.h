//----------------------------------------------------------------------------
/** @file FeBasicFeatures.h
    Non-pattern as in Remi Coulom's work, and 3x3 pattern features.
*/
//----------------------------------------------------------------------------

#ifndef FE_BASIC_FEATURES_H
#define FE_BASIC_FEATURES_H

//----------------------------------------------------------------------------

#include <bitset>
#include <iosfwd>
#include <boost/array.hpp>
#include "FeFeatureWeights.h"
#include "GoBoardUtil.h"
#include "GoEvalArray.h"
#include "GoLadder.h"

//----------------------------------------------------------------------------

// Coulom's classical features plus additions
enum FeBasicFeature{
    FE_LINE_1,
    FE_LINE_2,
    FE_LINE_3,
    FE_LINE_4,
    FE_LINE_5_OR_MORE,
    FE_DIST_PREV_2, // d(dx,dy) = |dx|+|dy|+max(|dx|,|dy|)
    FE_DIST_PREV_3,
    FE_DIST_PREV_4,
    FE_DIST_PREV_5,
    FE_DIST_PREV_6,
    FE_DIST_PREV_7,
    FE_DIST_PREV_8,
    FE_DIST_PREV_9,
    FE_DIST_PREV_10,
    FE_DIST_PREV_11,
    FE_DIST_PREV_12,
    FE_DIST_PREV_13,
    FE_DIST_PREV_14,
    FE_DIST_PREV_15,
    FE_DIST_PREV_16,
    FE_DIST_PREV_17,
    FE_DIST_PREV_OWN_2,
    FE_DIST_PREV_OWN_3,
    FE_DIST_PREV_OWN_4,
    FE_DIST_PREV_OWN_5,
    FE_DIST_PREV_OWN_6,
    FE_DIST_PREV_OWN_7,
    FE_DIST_PREV_OWN_8,
    FE_DIST_PREV_OWN_9,
    FE_DIST_PREV_OWN_10,
    FE_DIST_PREV_OWN_11,
    FE_DIST_PREV_OWN_12,
    FE_DIST_PREV_OWN_13,
    FE_DIST_PREV_OWN_14,
    FE_DIST_PREV_OWN_15,
    FE_DIST_PREV_OWN_16,
    FE_DIST_PREV_OWN_17,
    FE_POS_1, // Position of a point p according to GoBoard::Pos(p)
    FE_POS_2,
    FE_POS_3,
    FE_POS_4,
    FE_POS_5,
    FE_POS_6,
    FE_POS_7,
    FE_POS_8,
    FE_POS_9,
    FE_POS_10,
    FE_GAME_PHASE_1, // Game phase as in Wistuba - 30 moves per phase
    FE_GAME_PHASE_2,
    FE_GAME_PHASE_3,
    FE_GAME_PHASE_4,
    FE_GAME_PHASE_5,
    FE_GAME_PHASE_6,
    FE_GAME_PHASE_7,
    FE_GAME_PHASE_8,
    FE_GAME_PHASE_9,
    FE_GAME_PHASE_10,
    FE_GAME_PHASE_11,
    FE_GAME_PHASE_12,
    FE_CORNER_OPENING_MOVE,
    FE_CFG_DISTANCE_LAST_1,
    FE_CFG_DISTANCE_LAST_2,
    FE_CFG_DISTANCE_LAST_3,
    FE_CFG_DISTANCE_LAST_4_OR_MORE,
    FE_CFG_DISTANCE_LAST_OWN_0,
    FE_CFG_DISTANCE_LAST_OWN_1,
    FE_CFG_DISTANCE_LAST_OWN_2,
    FE_CFG_DISTANCE_LAST_OWN_3,
    FE_CFG_DISTANCE_LAST_OWN_4_OR_MORE,
    FE_DIST_CLOSEST_OWN_STONE_2,
    FE_DIST_CLOSEST_OWN_STONE_3,
    FE_DIST_CLOSEST_OWN_STONE_4,
    FE_DIST_CLOSEST_OWN_STONE_5,
    FE_DIST_CLOSEST_OWN_STONE_6,
    FE_DIST_CLOSEST_OWN_STONE_7,
    FE_DIST_CLOSEST_OWN_STONE_8,
    FE_DIST_CLOSEST_OWN_STONE_9,
    FE_DIST_CLOSEST_OWN_STONE_10,
    FE_DIST_CLOSEST_OWN_STONE_11,
    FE_DIST_CLOSEST_OWN_STONE_12,
    FE_DIST_CLOSEST_OWN_STONE_13,
    FE_DIST_CLOSEST_OWN_STONE_14,
    FE_DIST_CLOSEST_OWN_STONE_15,
    FE_DIST_CLOSEST_OWN_STONE_16,
    FE_DIST_CLOSEST_OWN_STONE_17,
    FE_DIST_CLOSEST_OWN_STONE_18,
    FE_DIST_CLOSEST_OWN_STONE_19,
    FE_DIST_CLOSEST_OWN_STONE_20_OR_MORE, // see MAX_CLOSEST_DISTANCE below
    FE_DIST_CLOSEST_OPP_STONE_2,
    FE_DIST_CLOSEST_OPP_STONE_3,
    FE_DIST_CLOSEST_OPP_STONE_4,
    FE_DIST_CLOSEST_OPP_STONE_5,
    FE_DIST_CLOSEST_OPP_STONE_6,
    FE_DIST_CLOSEST_OPP_STONE_7,
    FE_DIST_CLOSEST_OPP_STONE_8,
    FE_DIST_CLOSEST_OPP_STONE_9,
    FE_DIST_CLOSEST_OPP_STONE_10,
    FE_DIST_CLOSEST_OPP_STONE_11,
    FE_DIST_CLOSEST_OPP_STONE_12,
    FE_DIST_CLOSEST_OPP_STONE_13,
    FE_DIST_CLOSEST_OPP_STONE_14,
    FE_DIST_CLOSEST_OPP_STONE_15,
    FE_DIST_CLOSEST_OPP_STONE_16,
    FE_DIST_CLOSEST_OPP_STONE_17,
    FE_DIST_CLOSEST_OPP_STONE_18,
    FE_DIST_CLOSEST_OPP_STONE_19,
    FE_DIST_CLOSEST_OPP_STONE_20_OR_MORE,
    FE_NONE,
    _NU_FE_FEATURES

    // other ideas: tacticaldistance = less for lowlib stones
};

const int MAX_CLOSEST_DISTANCE = 20;

typedef std::bitset<_NU_FE_FEATURES> FeBasicFeatureSet;

std::ostream& operator<<(std::ostream& stream, FeBasicFeature f);

//----------------------------------------------------------------------------

const int INVALID_PATTERN_INDEX = -1;

const size_t MAX_ACTIVE_LENGTH = 20;

typedef boost::array<int, MAX_ACTIVE_LENGTH> FeActiveArray;

typedef boost::array<int, MAX_ACTIVE_LENGTH>::const_iterator FeActiveIterator;

//----------------------------------------------------------------------------

class FeMoveFeatures
{
public:
    FeMoveFeatures();

    /** Put active move features into array; return how many were found */
    size_t ActiveFeatures(FeActiveArray& active) const;

    const FeBasicFeatureSet& BasicFeatures() const;
    
    /** Find features for board move (not pass) */
    void FindMoveFeatures(const GoBoard& bd, SgPoint move);

    void Set(FeBasicFeature f);

    void Set12PointIndex(int index);

    /** See FeFullBoardFeatures::WriteNumeric() */
    void WriteNumeric(std::ostream& stream,
                      const int isChosen,
                      const int moveNumber,
                      const bool writeComment) const;

    void WriteFeatures(std::ostream& stream,
                       SgPoint move) const;

private:
    void WritePatternFeatureIndex(std::ostream& stream) const;

    void WritePatternFeatures(std::ostream& stream) const;

    FeBasicFeatureSet m_basicFeatures;

    int m_3x3Index;

    int m_12PointIndex;
};

inline FeMoveFeatures::FeMoveFeatures()
    :
    m_basicFeatures(),
    m_3x3Index(INVALID_PATTERN_INDEX),
    m_12PointIndex(INVALID_PATTERN_INDEX)
{ }

inline const FeBasicFeatureSet&
FeMoveFeatures::BasicFeatures() const
{
    return m_basicFeatures;
}

inline void FeMoveFeatures::Set(FeBasicFeature f)
{
    m_basicFeatures.set(f);
}

inline void FeMoveFeatures::Set12PointIndex(int index)
{
    m_12PointIndex = index;
}

//----------------------------------------------------------------------------

struct FeFullBoardFeatures
{
    FeFullBoardFeatures(const GoBoard& bd);

    const FeBasicFeatureSet& BasicFeatures(SgPoint move) const;

    GoEvalArray<float>
    EvaluateFeatures(const FeFeatureWeights& weights) const;

    /** Some features are computed by GoUct which is higher up, so need 
        to export this. It would be cleaner to move FeFullBoardFeatures
        down into GoUct as well. */
    GoEvalArray<FeMoveFeatures>& Features();

    void FindAllFeatures();
    
    const GoPointList& LegalMoves() const;

    /** Write in human-readable way */
    void WriteBoardFeatures(std::ostream& stream) const;

    /** Write features in the format of Wistuba's gamma learning code
     Each candidate move is described by a list of ID of its features.
     The first number in each line is a 0 for a non-played move,
     and 1 for the played move which must be written last.

     Example output:
     0 23 456 1 9
     0 45 67 999
     1 55 87 1234

     If writeComment is true, then the validator comment #X_Y Z is added.
     Here X is supposed to be a game number, which is not available in this
     context. Therefore, a 0 is written and later replaced by a standalone
     postprocessing tool.
     Y is the move number.
     Z is the size of the largest matching pattern feature.
     Right now it is set to constant 3.
     */
    void WriteNumeric(std::ostream& stream,
                      const SgPoint chosenMove,
                      const bool writeComment) const;

private:
    void FindFullBoardFeatures();

    const GoBoard& m_bd;

    GoEvalArray<FeMoveFeatures> m_features;

    GoPointList m_legalMoves;
};

inline FeFullBoardFeatures::FeFullBoardFeatures(const GoBoard& bd)
    :
    m_bd(bd),
    m_features(),
    m_legalMoves(GoBoardUtil::AllLegalMoves(bd))
{ }

inline const FeBasicFeatureSet&
FeFullBoardFeatures::BasicFeatures(SgPoint move) const
{
    return m_features[move].BasicFeatures();
}

inline GoEvalArray<FeMoveFeatures>& FeFullBoardFeatures::Features()
{
    return m_features;
}

inline const GoPointList& FeFullBoardFeatures::LegalMoves() const
{
    return m_legalMoves;
}

//----------------------------------------------------------------------------

namespace FeFeatures {

struct FeEvalDetail
{
    FeEvalDetail(int feature, float w, float v);
    int m_feature;
    float m_w;
    float m_v_sum;
};

inline FeEvalDetail::FeEvalDetail(int feature, float w, float v)
    :
    m_feature(feature),
    m_w(w),
    m_v_sum(v)
{ }

std::ostream& operator<<(std::ostream& stream, const FeEvalDetail& f);

//---------------------------------

/** Evaluate a given list of features, using weights */
float EvaluateActiveFeatures(const FeActiveArray& active,
                             size_t nuActive,
                             const FeFeatureWeights& weights);

/** Evaluate features for one move, using weights */
float EvaluateMoveFeatures(const FeMoveFeatures& features,
                           const FeFeatureWeights& weights);

/** For display, return details on each evaluated feature */
std::vector<FeEvalDetail>
EvaluateMoveFeaturesDetail(const FeMoveFeatures& features,
                           const FeFeatureWeights& weights);

/** Find features for a single move. Computes full board. */
void FindBasicMoveFeaturesUI(const GoBoard& bd, SgPoint move,
                           FeBasicFeatureSet& features);

void FindPassFeatures(const GoBoard& bd, FeBasicFeatureSet& features);

int Get3x3Feature(const GoBoard& bd, SgPoint p);

bool IsBasicFeatureID(int id);
    
bool Is3x3PatternID(int id);

/** Write detailed evaluation computed with EvaluateMoveFeaturesDetail() */
void WriteEvalDetail(std::ostream& stream,
                     const std::vector<FeEvalDetail>& detail);

/** Write feature in human-readable form */
void WriteFeatureFromID(std::ostream& stream, int id);
        
/** Write features for single move as integer list */
void WriteFeatureSet(std::ostream& stream,
                     SgPoint move,
                     const FeBasicFeatureSet& features);

/** Write features for single move in human-readable strings */
void WriteFeatureSetAsText(std::ostream& stream,
                           SgPoint move,
                           const FeBasicFeatureSet& features);

} // namespace FeFeatures

//----------------------------------------------------------------------------

#endif // FE_BASIC_FEATURES_H

