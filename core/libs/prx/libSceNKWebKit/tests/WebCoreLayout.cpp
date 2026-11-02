#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

using NKWebKit::WtfAbi::String;

static void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "WebCore layout check failed at line %d\n", line);
        std::abort();
    }
}
#define Require(value) Check((value), __LINE__)

static String Narrow(const std::string& text) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(text.data()), static_cast<unsigned>(text.size()));
}

template<typename Function> static bool Throws(Function function) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

static void TestGridPosition() {
    WebCore::GridPosition grid;
    grid.m_type = WebCore::AutoPosition;
    grid.m_integerPosition = 0;

    String line = Narrow("a");
    unsigned before = line.impl->refCount;
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, 5, line);
    Require(grid.m_type == WebCore::ExplicitPosition && grid.m_integerPosition == 5);
    Require(grid.m_namedGridLine.impl == line.impl && line.impl->refCount == before + 2);
    Require(_ZNK7WebCore12GridPosition15integerPositionEv(&grid) == 5);

    String other = Narrow("b");
    _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(&grid, -3, other);
    Require(grid.m_type == WebCore::SpanPosition && grid.m_integerPosition == -3);
    Require(grid.m_namedGridLine.impl == other.impl && line.impl->refCount == before);
    Require(_ZNK7WebCore12GridPosition12spanPositionEv(&grid) == -3);

    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, 2000000, String());
    Require(grid.m_integerPosition == 1000000 && grid.m_namedGridLine.impl == nullptr);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, -2000000, other);
    Require(grid.m_integerPosition == -1000000);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, 1000000, other);
    Require(grid.m_integerPosition == 1000000);
    _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(&grid, 999999, other);
    Require(grid.m_integerPosition == 999999);
    _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(&grid, 0, other);
    Require(grid.m_integerPosition == 0);
    _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(&grid, std::numeric_limits<int>::min(), other);
    Require(grid.m_integerPosition == -1000000);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, std::numeric_limits<int>::max(), other);
    Require(grid.m_integerPosition == 1000000);

    _ZN7WebCore12GridPosition24setMaxPositionForTestingEj(10);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, 15, other);
    Require(grid.m_integerPosition == 10);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, -15, other);
    Require(grid.m_integerPosition == -10);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, 10, other);
    Require(grid.m_integerPosition == 10);
    _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(&grid, 9, other);
    Require(grid.m_integerPosition == 9);
    _ZN7WebCore12GridPosition24setMaxPositionForTestingEj(1000000);
    _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(&grid, 5000000, other);
    Require(grid.m_integerPosition == 1000000);

    Require(Throws([&] { _ZNK7WebCore12GridPosition15integerPositionEv(nullptr); }));
    Require(Throws([&] { _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(nullptr, 1, other); }));
}

static WebCore::Length IntLength(int value, WebCore::LengthType type) {
    WebCore::Length length{};
    length.m_intValue = value;
    length.m_type = type;
    length.m_hasQuirk = false;
    length.m_isFloat = false;
    return length;
}

static WebCore::Length FloatLength(float value, WebCore::LengthType type) {
    WebCore::Length length{};
    length.m_floatValue = value;
    length.m_type = type;
    length.m_hasQuirk = false;
    length.m_isFloat = true;
    return length;
}

static int ValueForLength(const WebCore::Length& length, int rawMaximum) {
    return _ZN7WebCore14valueForLengthERKNS_6LengthENS_10LayoutUnitE(length, WebCore::LayoutUnit{rawMaximum}).m_value;
}

static void TestLengthFunctions() {
    using WebCore::LengthType;
    Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(IntLength(10, LengthType::Fixed), 200.0f) == 10.0f);
    Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(FloatLength(12.5f, LengthType::Fixed), 200.0f) == 12.5f);
    Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(IntLength(50, LengthType::Percent), 200.0f) == 100.0f);
    Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(FloatLength(12.5f, LengthType::Percent), 32.0f) == 4.0f);
    Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(IntLength(0, LengthType::Auto), 77.5f) == 77.5f);
    Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(IntLength(0, LengthType::FillAvailable), 77.5f) == 77.5f);
    for (auto type : {LengthType::Relative, LengthType::Intrinsic, LengthType::MinIntrinsic, LengthType::MinContent, LengthType::MaxContent,
                      LengthType::FitContent, LengthType::Content, LengthType::Undefined})
        Require(_ZN7WebCore19floatValueForLengthERKNS_6LengthEf(IntLength(7, type), 100.0f) == 0.0f);
    Require(Throws([&] { _ZN7WebCore19floatValueForLengthERKNS_6LengthEf(IntLength(1, LengthType::Calculated), 100.0f); }));

    Require(ValueForLength(IntLength(10, LengthType::Fixed), 12800) == 640);
    Require(ValueForLength(FloatLength(1.5f, LengthType::Fixed), 0) == 96);
    Require(ValueForLength(FloatLength(-1.5f, LengthType::Fixed), 0) == -96);
    Require(ValueForLength(FloatLength(0.01f, LengthType::Fixed), 0) == 0);
    Require(ValueForLength(FloatLength(1e12f, LengthType::Fixed), 0) == std::numeric_limits<int>::max());
    Require(ValueForLength(FloatLength(-1e12f, LengthType::Fixed), 0) == std::numeric_limits<int>::min());
    Require(ValueForLength(FloatLength(std::nanf(""), LengthType::Fixed), 0) == std::numeric_limits<int>::min());
    Require(ValueForLength(IntLength(50, LengthType::Percent), 12800) == 6400);
    Require(ValueForLength(IntLength(25, LengthType::Percent), 672) == 168);
    Require(ValueForLength(IntLength(0, LengthType::Auto), 4321) == 4321);
    Require(ValueForLength(IntLength(0, LengthType::FillAvailable), -77) == -77);
    for (auto type : {LengthType::Relative, LengthType::Intrinsic, LengthType::MinIntrinsic, LengthType::MinContent, LengthType::MaxContent,
                      LengthType::FitContent, LengthType::Content, LengthType::Undefined})
        Require(ValueForLength(IntLength(7, type), 640) == 0);
    Require(Throws([&] { ValueForLength(IntLength(1, LengthType::Calculated), 640); }));
}

static WebCore::FloatQuad Quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    return WebCore::FloatQuad{{x1, y1}, {x2, y2}, {x3, y3}, {x4, y4}};
}

static WebCore::FloatRect United(const WebCore::FloatQuad* quads, unsigned size) {
    WebCore::FloatQuadVector vector{quads, size, size};
    return _ZN7WebCore19unitedBoundingBoxesERKN3WTF6VectorINS_9FloatQuadELm0ENS0_15CrashOnOverflowELm16ENS0_10FastMallocEEE(vector);
}

static bool RectIs(const WebCore::FloatRect& rect, float x, float y, float width, float height) {
    return rect.m_location.m_x == x && rect.m_location.m_y == y && rect.m_size.m_width == width && rect.m_size.m_height == height;
}

static void TestUnitedBoundingBoxes() {
    Require(RectIs(United(nullptr, 0), 0, 0, 0, 0));
    WebCore::FloatQuad quads[] = {
        Quad(1, 2, 5, 3, 4, 9, 2, 7),
        Quad(10, -3, 12, -3, 12, 0, 10, 0),
        Quad(20, 20, 20, 20, 20, 20, 20, 20),
    };
    Require(RectIs(United(quads, 1), 1, 2, 4, 7));
    Require(RectIs(United(quads, 2), 1, -3, 11, 12));
    Require(RectIs(United(quads, 3), 1, -3, 19, 23));
    Require(RectIs(United(quads + 2, 1), 20, 20, 0, 0));
    Require(Throws([&] { United(nullptr, 2); }));
}

static WebCore::ViewportArguments Arguments(int type) {
    WebCore::ViewportArguments arguments{};
    arguments.type = type;
    arguments.width = arguments.minWidth = arguments.maxWidth = -1;
    arguments.height = arguments.minHeight = arguments.maxHeight = -1;
    arguments.zoom = arguments.minZoom = arguments.maxZoom = -1;
    arguments.userZoom = arguments.orientation = arguments.shrinkToFit = -1;
    arguments.viewportFit = WebCore::ViewportFit::Auto;
    arguments.widthWasExplicit = false;
    return arguments;
}

static bool Attributes(const WebCore::ViewportAttributes& result, float width, float height, float initial, float minimum, float maximum) {
    return result.layoutSize.m_width == width && result.layoutSize.m_height == height && result.initialScale == initial
        && result.minimumScale == minimum && result.maximumScale == maximum;
}

static void TestComputeViewportAttributes() {
    constexpr int implicitType = 0;
    constexpr int viewportMetaType = 1;
    constexpr int cssDeviceAdaptationType = 2;

    auto meta = Arguments(viewportMetaType);
    meta.width = -2;
    auto result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(meta, 980, 1280, 720, 1.0f, WebCore::IntSize{1280, 720});
    Require(Attributes(result, 1280, 720, 1, 0.25f, 5));
    Require(result.userScalable == -1 && result.orientation == -1 && result.shrinkToFit == -1 && result.viewportFit == WebCore::ViewportFit::Auto);

    auto implicit = Arguments(implicitType);
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(implicit, 980, 720, 1280, 2.0f, WebCore::IntSize{720, 1280});
    Require(Attributes(result, 980, 980 * (640.0f / 360.0f), 360.0f / 980, 0.25f, 5));

    auto adaptation = Arguments(cssDeviceAdaptationType);
    adaptation.width = -2;
    adaptation.minZoom = 1;
    adaptation.maxZoom = 2;
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(adaptation, 980, 800, 600, 1.0f, WebCore::IntSize{800, 600});
    Require(Attributes(result, 800, 600, 1, 1, 2));

    auto scaled = Arguments(viewportMetaType);
    scaled.zoom = 0.5f;
    scaled.userZoom = 0;
    scaled.orientation = 1;
    scaled.shrinkToFit = 0;
    scaled.viewportFit = WebCore::ViewportFit::Cover;
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(scaled, 980, 1000, 500, 1.0f, WebCore::IntSize{1000, 500});
    Require(Attributes(result, 2000, 1000, 0.5f, 0.25f, 5));
    Require(result.userScalable == 0 && result.orientation == 1 && result.shrinkToFit == 0 && result.viewportFit == WebCore::ViewportFit::Cover);

    auto clamped = Arguments(viewportMetaType);
    clamped.width = 50000;
    clamped.height = 0.5f;
    clamped.zoom = 20;
    clamped.minZoom = 0.01f;
    clamped.maxZoom = 40;
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(clamped, 980, 1000, 500, 1.0f, WebCore::IntSize{1000, 500});
    Require(result.minimumScale == 0.1f && result.maximumScale == 10 && result.initialScale == 10);
    Require(result.layoutSize.m_width == 10000 && result.layoutSize.m_height == 50);

    auto heightBound = Arguments(viewportMetaType);
    heightBound.width = -2;
    heightBound.height = 250;
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(heightBound, 980, 1000, 500, 1.0f, WebCore::IntSize{1000, 500});
    Require(Attributes(result, 1000, 250, 2, 0.25f, 5));

    auto deviceHeight = Arguments(cssDeviceAdaptationType);
    deviceHeight.width = -3;
    deviceHeight.height = -2;
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(deviceHeight, 980, 1000, 500, 1.0f, WebCore::IntSize{1000, 500});
    Require(result.layoutSize.m_width == 500 && result.layoutSize.m_height == 1000);

    auto adaptationBounds = Arguments(cssDeviceAdaptationType);
    adaptationBounds.minWidth = 600;
    adaptationBounds.maxWidth = 800;
    adaptationBounds.zoom = 1;
    result = _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(adaptationBounds, 980, 1000, 500, 1.0f, WebCore::IntSize{1000, 500});
    Require(result.layoutSize.m_width == 1000 && result.layoutSize.m_height == 500 && result.initialScale == 1);
}

static void TestViewportScaleHelpers() {
    WebCore::ViewportAttributes attributes{};
    attributes.layoutSize = WebCore::FloatSize{1000, 500};
    attributes.initialScale = 2;
    attributes.minimumScale = 0.25f;
    attributes.maximumScale = 5;
    attributes.userScalable = 0;
    _ZN7WebCore50restrictScaleFactorToInitialScaleIfNotUserScalableERNS_18ViewportAttributesE(attributes);
    Require(attributes.minimumScale == 2 && attributes.maximumScale == 2 && attributes.initialScale == 2);
    attributes.minimumScale = 0.25f;
    attributes.maximumScale = 5;
    attributes.userScalable = 1;
    _ZN7WebCore50restrictScaleFactorToInitialScaleIfNotUserScalableERNS_18ViewportAttributesE(attributes);
    Require(attributes.minimumScale == 0.25f && attributes.maximumScale == 5);

    _ZN7WebCore40restrictMinimumScaleFactorToViewportSizeERNS_18ViewportAttributesENS_7IntSizeEf(attributes, WebCore::IntSize{800, 600}, 1.0f);
    Require(attributes.minimumScale == 600.0f / 500);
    attributes.minimumScale = 0.25f;
    _ZN7WebCore40restrictMinimumScaleFactorToViewportSizeERNS_18ViewportAttributesENS_7IntSizeEf(attributes, WebCore::IntSize{800, 600}, 2.0f);
    Require(attributes.minimumScale == 300.0f / 500);
    attributes.minimumScale = 3;
    _ZN7WebCore40restrictMinimumScaleFactorToViewportSizeERNS_18ViewportAttributesENS_7IntSizeEf(attributes, WebCore::IntSize{800, 600}, 1.0f);
    Require(attributes.minimumScale == 3);

    attributes.minimumScale = 0.25f;
    Require(_ZN7WebCore44computeMinimumScaleFactorForContentContainedERKNS_18ViewportAttributesERKNS_7IntSizeES5_(attributes, WebCore::IntSize{800, 600}, WebCore::IntSize{1600, 400}) == 1.5f);
    Require(_ZN7WebCore44computeMinimumScaleFactorForContentContainedERKNS_18ViewportAttributesERKNS_7IntSizeES5_(attributes, WebCore::IntSize{800, 600}, WebCore::IntSize{4000, 4000}) == 0.25f);
    Require(_ZN7WebCore44computeMinimumScaleFactorForContentContainedERKNS_18ViewportAttributesERKNS_7IntSizeES5_(attributes, WebCore::IntSize{800, 600}, WebCore::IntSize{800, 600}) == 1.0f);
}

int main() {
    TestGridPosition();
    TestLengthFunctions();
    TestUnitedBoundingBoxes();
    TestComputeViewportAttributes();
    TestViewportScaleHelpers();
    return 0;
}
