#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/WebCoreLayoutExports.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>


namespace {

using NKWebKit::WtfAbi::String;

constexpr int GridMaxPosition = 1000000;
std::optional<int> g_maxPositionForTesting;

template<typename T> T& Checked(T* value) {
    if (!value) throw std::invalid_argument("null WebCore object");
    return *value;
}

int GridMax() {
    return g_maxPositionForTesting.value_or(GridMaxPosition);
}

int ClampGridPosition(int value) {
    int max = GridMax();
    int min = -max;
    if (value >= max) return max;
    if (value <= min) return min;
    return value;
}

void Assign(String& target, const String& source) {
    String retained(source);
    std::swap(target.impl, retained.impl);
}

int ClampToInteger(double value) {
    if (value >= static_cast<double>(std::numeric_limits<int>::max())) return std::numeric_limits<int>::max();
    if (!(value > static_cast<double>(std::numeric_limits<int>::min()))) return std::numeric_limits<int>::min();
    return static_cast<int>(value);
}

constexpr int FixedPointDenominator = 64;

float LengthValue(const WebCore::Length& length) {
    return length.m_isFloat ? length.m_floatValue : static_cast<float>(length.m_intValue);
}

[[noreturn]] void ThrowCalculatedLength() {
    throw std::invalid_argument("calculated WebCore length");
}


int TruncateToInt(float value) {
    if (!(value > -2147483648.0f && value < 2147483648.0f)) return std::numeric_limits<int>::min();
    return static_cast<int>(value);
}

constexpr int ValueAuto = -1;
constexpr int ValueDeviceWidth = -2;
constexpr int ValueDeviceHeight = -3;
constexpr int ImplicitType = 0;
constexpr int ViewportMetaType = 1;
constexpr int CSSDeviceAdaptationType = 2;

float MinFloat(float a, float b) {
    return std::min(a, b);
}

float MaxFloat(float a, float b) {
    return std::max(a, b);
}

float CompareIgnoringAuto(float value1, float value2, float (*compare)(float, float)) {
    if (value1 == ValueAuto) return value2;
    if (value2 == ValueAuto) return value1;
    return compare(value1, value2);
}

float ClampLengthValue(float value) {
    if (value != ValueAuto) return std::min<float>(10000, std::max<float>(value, 1));
    return value;
}

float ClampScaleValue(float value) {
    if (value != ValueAuto) return std::min<float>(10, std::max<float>(value, 0.1));
    return value;
}

WebCore::FloatSize ConvertToUserSpace(const WebCore::FloatSize& deviceSize, float devicePixelRatio) {
    WebCore::FloatSize result = deviceSize;
    if (devicePixelRatio != 1) {
        float scale = 1 / devicePixelRatio;
        result.m_width *= scale;
        result.m_height *= scale;
    }
    return result;
}

void ResolveDeviceKeyword(float& value, const WebCore::FloatSize& deviceSize) {
    switch (TruncateToInt(value)) {
    case ValueDeviceWidth:
        value = deviceSize.m_width;
        break;
    case ValueDeviceHeight:
        value = deviceSize.m_height;
        break;
    }
}

WebCore::ViewportAttributes Resolve(const WebCore::ViewportArguments& arguments, const WebCore::FloatSize& initialViewportSize, const WebCore::FloatSize& deviceSize, int defaultWidth) {
    float resultWidth = arguments.width;
    float resultMaxWidth = arguments.maxWidth;
    float resultMinWidth = arguments.minWidth;
    float resultHeight = arguments.height;
    float resultMinHeight = arguments.minHeight;
    float resultMaxHeight = arguments.maxHeight;
    float resultZoom = arguments.zoom;
    float resultMinZoom = arguments.minZoom;
    float resultMaxZoom = arguments.maxZoom;

    ResolveDeviceKeyword(resultWidth, deviceSize);
    ResolveDeviceKeyword(resultHeight, deviceSize);

    if (arguments.type == CSSDeviceAdaptationType) {
        ResolveDeviceKeyword(resultMinWidth, deviceSize);
        ResolveDeviceKeyword(resultMaxWidth, deviceSize);
        ResolveDeviceKeyword(resultMinHeight, deviceSize);
        ResolveDeviceKeyword(resultMaxHeight, deviceSize);

        if (resultMinWidth != ValueAuto || resultMaxWidth != ValueAuto)
            resultWidth = CompareIgnoringAuto(resultMinWidth, CompareIgnoringAuto(resultMaxWidth, deviceSize.m_width, MinFloat), MaxFloat);

        if (resultMinHeight != ValueAuto || resultMaxHeight != ValueAuto)
            resultHeight = CompareIgnoringAuto(resultMinHeight, CompareIgnoringAuto(resultMaxHeight, deviceSize.m_height, MinFloat), MaxFloat);

        if (resultMinZoom != ValueAuto && resultMaxZoom != ValueAuto)
            resultMaxZoom = std::max(resultMinZoom, resultMaxZoom);

        if (resultZoom != ValueAuto)
            resultZoom = CompareIgnoringAuto(resultMinZoom, CompareIgnoringAuto(resultMaxZoom, resultZoom, MinFloat), MaxFloat);

        if (resultWidth == ValueAuto && resultZoom == ValueAuto)
            resultWidth = deviceSize.m_width;

        if (resultWidth == ValueAuto && resultHeight == ValueAuto)
            resultWidth = deviceSize.m_width / resultZoom;

        if (resultWidth == ValueAuto)
            resultWidth = resultHeight * deviceSize.m_width / deviceSize.m_height;

        if (resultHeight == ValueAuto)
            resultHeight = resultWidth * deviceSize.m_height / deviceSize.m_width;

        if (resultZoom != ValueAuto || resultMaxZoom != ValueAuto) {
            resultWidth = CompareIgnoringAuto(resultWidth, deviceSize.m_width / CompareIgnoringAuto(resultZoom, resultMaxZoom, MinFloat), MaxFloat);
            resultHeight = CompareIgnoringAuto(resultHeight, deviceSize.m_height / CompareIgnoringAuto(resultZoom, resultMaxZoom, MinFloat), MaxFloat);
        }

        resultWidth = std::max<float>(1, resultWidth);
        resultHeight = std::max<float>(1, resultHeight);
    }

    if (arguments.type != CSSDeviceAdaptationType && arguments.type != ImplicitType) {
        resultWidth = ClampLengthValue(resultWidth);
        resultHeight = ClampLengthValue(resultHeight);
        resultZoom = ClampScaleValue(resultZoom);
        resultMinZoom = ClampScaleValue(resultMinZoom);
        resultMaxZoom = ClampScaleValue(resultMaxZoom);
    }

    WebCore::ViewportAttributes result{};

    if (resultMinZoom == ValueAuto)
        result.minimumScale = float(0.25);
    else
        result.minimumScale = resultMinZoom;

    if (resultMaxZoom == ValueAuto) {
        result.maximumScale = 5;
        result.minimumScale = std::min<float>(5, result.minimumScale);
    } else {
        result.maximumScale = resultMaxZoom;
    }
    result.maximumScale = std::max(result.minimumScale, result.maximumScale);

    result.initialScale = resultZoom;
    if (resultZoom == ValueAuto) {
        result.initialScale = initialViewportSize.m_width / defaultWidth;
        if (resultWidth != ValueAuto)
            result.initialScale = initialViewportSize.m_width / resultWidth;
        if (resultHeight != ValueAuto)
            result.initialScale = std::max<float>(result.initialScale, initialViewportSize.m_height / resultHeight);
    }

    result.initialScale = std::min(result.maximumScale, std::max(result.minimumScale, result.initialScale));

    if (resultWidth == ValueAuto) {
        if (resultZoom == ValueAuto)
            resultWidth = static_cast<float>(defaultWidth);
        else if (resultHeight != ValueAuto)
            resultWidth = resultHeight * (initialViewportSize.m_width / initialViewportSize.m_height);
        else
            resultWidth = initialViewportSize.m_width / result.initialScale;
    }

    if (resultHeight == ValueAuto)
        resultHeight = resultWidth * (initialViewportSize.m_height / initialViewportSize.m_width);

    if (arguments.type == ViewportMetaType) {
        resultWidth = std::max<float>(resultWidth, initialViewportSize.m_width / result.initialScale);
        resultHeight = std::max<float>(resultHeight, initialViewportSize.m_height / result.initialScale);
    }

    result.layoutSize.m_width = resultWidth;
    result.layoutSize.m_height = resultHeight;
    result.userScalable = arguments.userZoom;
    result.orientation = arguments.orientation;
    result.shrinkToFit = arguments.shrinkToFit;
    result.viewportFit = arguments.viewportFit;
    return result;
}

float Min4(float a, float b, float c, float d) {
    return std::min(std::min(a, b), std::min(c, d));
}

float Max4(float a, float b, float c, float d) {
    return std::max(std::max(a, b), std::max(c, d));
}

WebCore::FloatRect BoundingBox(const WebCore::FloatQuad& quad) {
    float left = Min4(quad.m_p1.m_x, quad.m_p2.m_x, quad.m_p3.m_x, quad.m_p4.m_x);
    float top = Min4(quad.m_p1.m_y, quad.m_p2.m_y, quad.m_p3.m_y, quad.m_p4.m_y);
    float right = Max4(quad.m_p1.m_x, quad.m_p2.m_x, quad.m_p3.m_x, quad.m_p4.m_x);
    float bottom = Max4(quad.m_p1.m_y, quad.m_p2.m_y, quad.m_p3.m_y, quad.m_p4.m_y);
    return WebCore::FloatRect{WebCore::FloatPoint{left, top}, WebCore::FloatSize{right - left, bottom - top}};
}

}  // namespace

extern "C" {

void APS5_VABI _ZN7WebCore12GridPosition19setExplicitPositionEiRKN3WTF6StringE(WebCore::GridPosition* self, int position, const String& namedGridLine) {
    auto& grid = Checked(self);
    grid.m_type = WebCore::ExplicitPosition;
    grid.m_integerPosition = ClampGridPosition(position);
    Assign(grid.m_namedGridLine, namedGridLine);
}

void APS5_VABI _ZN7WebCore12GridPosition15setSpanPositionEiRKN3WTF6StringE(WebCore::GridPosition* self, int position, const String& namedGridLine) {
    auto& grid = Checked(self);
    grid.m_type = WebCore::SpanPosition;
    grid.m_integerPosition = ClampGridPosition(position);
    Assign(grid.m_namedGridLine, namedGridLine);
}

void APS5_VABI _ZN7WebCore12GridPosition24setMaxPositionForTestingEj(unsigned maxPosition) {
    g_maxPositionForTesting = static_cast<int>(maxPosition);
}

int APS5_VABI _ZNK7WebCore12GridPosition15integerPositionEv(const WebCore::GridPosition* self) {
    return Checked(self).m_integerPosition;
}

int APS5_VABI _ZNK7WebCore12GridPosition12spanPositionEv(const WebCore::GridPosition* self) {
    return Checked(self).m_integerPosition;
}

float APS5_VABI _ZN7WebCore19floatValueForLengthERKNS_6LengthEf(const WebCore::Length& length, float maximumValue) {
    switch (length.m_type) {
    case WebCore::LengthType::Fixed:
        return LengthValue(length);
    case WebCore::LengthType::Percent:
        return static_cast<float>(maximumValue * LengthValue(length) / 100.0f);
    case WebCore::LengthType::FillAvailable:
    case WebCore::LengthType::Auto:
        return maximumValue;
    case WebCore::LengthType::Calculated:
        ThrowCalculatedLength();
    default:
        return 0;
    }
}

WebCore::LayoutUnit APS5_VABI _ZN7WebCore14valueForLengthERKNS_6LengthENS_10LayoutUnitE(const WebCore::Length& length, WebCore::LayoutUnit maximumValue) {
    switch (length.m_type) {
    case WebCore::LengthType::Fixed:
        return WebCore::LayoutUnit{ClampToInteger(static_cast<double>(LengthValue(length)) * FixedPointDenominator)};
    case WebCore::LengthType::Percent: {
        float maximum = static_cast<float>(maximumValue.m_value) / FixedPointDenominator;
        float value = static_cast<float>(maximum * LengthValue(length) / 100.0f);
        return WebCore::LayoutUnit{ClampToInteger(static_cast<double>(value) * FixedPointDenominator)};
    }
    case WebCore::LengthType::FillAvailable:
    case WebCore::LengthType::Auto:
        return maximumValue;
    case WebCore::LengthType::Calculated:
        ThrowCalculatedLength();
    default:
        return WebCore::LayoutUnit{0};
    }
}

WebCore::FloatRect APS5_VABI _ZN7WebCore19unitedBoundingBoxesERKN3WTF6VectorINS_9FloatQuadELm0ENS0_15CrashOnOverflowELm16ENS0_10FastMallocEEE(const WebCore::FloatQuadVector& quads) {
    if (!quads.m_size) return WebCore::FloatRect{};
    if (!quads.m_buffer) throw std::invalid_argument("null WTF vector buffer");
    WebCore::FloatRect result = BoundingBox(quads.m_buffer[0]);
    for (unsigned i = 1; i < quads.m_size; ++i) {
        WebCore::FloatRect other = BoundingBox(quads.m_buffer[i]);
        float minX = std::min(result.m_location.m_x, other.m_location.m_x);
        float minY = std::min(result.m_location.m_y, other.m_location.m_y);
        float maxX = std::max(result.m_location.m_x + result.m_size.m_width, other.m_location.m_x + other.m_size.m_width);
        float maxY = std::max(result.m_location.m_y + result.m_size.m_height, other.m_location.m_y + other.m_size.m_height);
        result.m_location = WebCore::FloatPoint{minX, minY};
        result.m_size = WebCore::FloatSize{maxX - minX, maxY - minY};
    }
    return result;
}

WebCore::ViewportAttributes APS5_VABI _ZN7WebCore25computeViewportAttributesENS_17ViewportArgumentsEiiifNS_7IntSizeE(
    WebCore::ViewportArguments args, int desktopWidth, int deviceWidth, int deviceHeight, float devicePixelRatio, WebCore::IntSize visibleViewport) {
    WebCore::FloatSize initialViewportSize = ConvertToUserSpace(
        WebCore::FloatSize{static_cast<float>(visibleViewport.m_width), static_cast<float>(visibleViewport.m_height)}, devicePixelRatio);
    WebCore::FloatSize deviceSize = ConvertToUserSpace(
        WebCore::FloatSize{static_cast<float>(deviceWidth), static_cast<float>(deviceHeight)}, devicePixelRatio);
    return Resolve(args, initialViewportSize, deviceSize, desktopWidth);
}

float APS5_VABI _ZN7WebCore44computeMinimumScaleFactorForContentContainedERKNS_18ViewportAttributesERKNS_7IntSizeES5_(
    const WebCore::ViewportAttributes& result, const WebCore::IntSize& viewportSize, const WebCore::IntSize& contentSize) {
    WebCore::FloatSize viewport{static_cast<float>(viewportSize.m_width), static_cast<float>(viewportSize.m_height)};
    return std::max<float>(result.minimumScale, std::max(viewport.m_width / contentSize.m_width, viewport.m_height / contentSize.m_height));
}

void APS5_VABI _ZN7WebCore40restrictMinimumScaleFactorToViewportSizeERNS_18ViewportAttributesENS_7IntSizeEf(
    WebCore::ViewportAttributes& result, WebCore::IntSize visibleViewport, float devicePixelRatio) {
    WebCore::FloatSize viewportSize = ConvertToUserSpace(
        WebCore::FloatSize{static_cast<float>(visibleViewport.m_width), static_cast<float>(visibleViewport.m_height)}, devicePixelRatio);
    result.minimumScale = std::max<float>(result.minimumScale, std::max(viewportSize.m_width / result.layoutSize.m_width, viewportSize.m_height / result.layoutSize.m_height));
}

void APS5_VABI _ZN7WebCore50restrictScaleFactorToInitialScaleIfNotUserScalableERNS_18ViewportAttributesE(WebCore::ViewportAttributes& result) {
    if (!result.userScalable) result.maximumScale = result.minimumScale = result.initialScale;
}

}
