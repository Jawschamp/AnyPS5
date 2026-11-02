#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/WebCoreTypes.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"
#include <cstddef>
#include <cstdint>

namespace WebCore {

enum GridPositionType : int {
    AutoPosition,
    ExplicitPosition,
    SpanPosition,
    NamedGridAreaPosition
};

struct GridPosition {
    GridPositionType m_type;
    int m_integerPosition;
    NKWebKit::WtfAbi::String m_namedGridLine;
};

enum class LengthType : std::uint8_t {
    Auto,
    Relative,
    Percent,
    Fixed,
    Intrinsic,
    MinIntrinsic,
    MinContent,
    MaxContent,
    FillAvailable,
    FitContent,
    Calculated,
    Content,
    Undefined
};

struct Length {
    union {
        int m_intValue;
        float m_floatValue;
        unsigned m_calculationValueHandle;
    };
    LengthType m_type;
    bool m_hasQuirk;
    bool m_isFloat;
};

struct FloatQuadVector {
    const FloatQuad* m_buffer;
    unsigned m_capacity;
    unsigned m_size;
};

enum class ViewportFit : std::uint8_t {
    Auto,
    Contain,
    Cover
};

struct ViewportAttributes {
    FloatSize layoutSize;
    float initialScale;
    float minimumScale;
    float maximumScale;
    float userScalable;
    float orientation;
    float shrinkToFit;
    ViewportFit viewportFit;
};

struct ViewportArguments {
    int type;
    float width;
    float minWidth;
    float maxWidth;
    float height;
    float minHeight;
    float maxHeight;
    float zoom;
    float minZoom;
    float maxZoom;
    float userZoom;
    float orientation;
    float shrinkToFit;
    ViewportFit viewportFit;
    bool widthWasExplicit;
};

static_assert(sizeof(GridPosition) == 16);
static_assert(offsetof(GridPosition, m_integerPosition) == 4);
static_assert(offsetof(GridPosition, m_namedGridLine) == 8);
static_assert(sizeof(Length) == 8);
static_assert(offsetof(Length, m_type) == 4);
static_assert(offsetof(Length, m_hasQuirk) == 5);
static_assert(offsetof(Length, m_isFloat) == 6);
static_assert(sizeof(FloatQuadVector) == 16);
static_assert(offsetof(FloatQuadVector, m_capacity) == 8);
static_assert(offsetof(FloatQuadVector, m_size) == 12);
static_assert(sizeof(ViewportAttributes) == 36);
static_assert(offsetof(ViewportAttributes, initialScale) == 8);
static_assert(offsetof(ViewportAttributes, viewportFit) == 32);
static_assert(sizeof(ViewportArguments) == 56);
static_assert(offsetof(ViewportArguments, width) == 4);
static_assert(offsetof(ViewportArguments, viewportFit) == 52);
static_assert(offsetof(ViewportArguments, widthWasExplicit) == 53);

}  // namespace WebCore

extern "C" {

void APS5_VABI _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(WebCore::GridPosition* self, int position, const NKWebKit::WtfAbi::String& namedGridLine);
void APS5_VABI _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(WebCore::GridPosition* self, int position, const NKWebKit::WtfAbi::String& namedGridLine);
void APS5_VABI _ZN7WebCore12GridPosition24setMaxPositionForTestingEj(unsigned maxPosition);
WebCore::LayoutUnit APS5_VABI _ZN7WebCore14valueForLengthERKNS_6LengthENS_10LayoutUnitE(const WebCore::Length& length, WebCore::LayoutUnit maximumValue);
float APS5_VABI _ZN7WebCore19floatValueForLengthERKNS_6LengthEf(const WebCore::Length& length, float maximumValue);
WebCore::FloatRect APS5_VABI _ZN7WebCore19unitedBoundingBoxesERKN3WTF6VectorINS_9FloatQuadELm0ENS0_15CrashOnOverflowELm16ENS0_10FastMallocEEE(const WebCore::FloatQuadVector& quads);
WebCore::ViewportAttributes APS5_VABI _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(WebCore::ViewportArguments args, int desktopWidth, int deviceWidth, int deviceHeight, float devicePixelRatio, WebCore::IntSize visibleViewport);
void APS5_VABI _ZN7WebCore40restrictMinimumScaleFactorToViewportSizeERNS_18ViewportAttributesENS_7IntSizeEf(WebCore::ViewportAttributes& result, WebCore::IntSize visibleViewport, float devicePixelRatio);
float APS5_VABI _ZN7WebCore44computeMinimumScaleFactorForContentContainedERKNS_18ViewportAttributesERKNS_7IntSizeES5_(const WebCore::ViewportAttributes& result, const WebCore::IntSize& viewportSize, const WebCore::IntSize& contentSize);
void APS5_VABI _ZN7WebCore50restrictScaleFactorToInitialScaleIfNotUserScalableERNS_18ViewportAttributesE(WebCore::ViewportAttributes& result);
int APS5_VABI _ZNK7WebCore12GridPosition12spanPositionEv(const WebCore::GridPosition* self);
int APS5_VABI _ZNK7WebCore12GridPosition15integerPositionEv(const WebCore::GridPosition* self);

}
