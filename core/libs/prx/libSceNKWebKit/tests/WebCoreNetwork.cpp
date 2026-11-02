#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

using NKWebKit::WtfAbi::String;

static void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "WebCore network check failed at line %d\n", line);
        std::abort();
    }
}
#define Require(value) Check((value), __LINE__)

static String Wide(const std::u16string& text) {
    return _ZN3WTF10StringImpl6createEPKDsj(text.data(), static_cast<unsigned>(text.size()));
}

static String Narrow(const std::string& text) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(text.data()), static_cast<unsigned>(text.size()));
}

static std::u16string Widen(const std::string& text) {
    return std::u16string(text.begin(), text.end());
}

static std::string Text(const String& value) {
    std::string result;
    if (!value.impl) return result;
    for (unsigned i = 0; i < value.impl->length; ++i) {
        char16_t c = (value.impl->hashAndFlags & 4) ? static_cast<const unsigned char*>(value.impl->data)[i] : static_cast<const char16_t*>(value.impl->data)[i];
        result.push_back(static_cast<char>(c));
    }
    return result;
}

static void ForBothWidths(const std::string& text, const std::function<void(const String&)>& body) {
    body(Narrow(text));
    body(Wide(Widen(text)));
}

static bool Valid(const std::string& text, WebCore::Mode mode) {
    bool result = true;
    ForBothWidths(text, [&](const String& value) {
        bool current = _ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(value, mode);
        Require(current == _ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(value, mode));
        result = current;
    });
    return result;
}

static WebCore::ParsedContentRange RangeFrom(const std::string& text) {
    WebCore::ParsedContentRange first{};
    WebCore::ParsedContentRange second{};
    ForBothWidths(text, [&](const String& value) {
        _ZN7WebCore18ParsedContentRangeC1ERKN3WTF6StringE(&first, value);
        _ZN7WebCore18ParsedContentRangeC2ERKN3WTF6StringE(&second, value);
        Require(first.m_firstBytePosition == second.m_firstBytePosition);
        Require(first.m_lastBytePosition == second.m_lastBytePosition);
        Require(first.m_instanceLength == second.m_instanceLength);
    });
    return first;
}

static bool IsValid(const WebCore::ParsedContentRange& range) {
    return range.m_instanceLength != std::numeric_limits<std::int64_t>::min();
}

static WebCore::ParsedContentRange RangeFromValues(std::int64_t first, std::int64_t last, std::int64_t instance) {
    WebCore::ParsedContentRange one{};
    WebCore::ParsedContentRange two{};
    _ZN7WebCore18ParsedContentRangeC1Elll(&one, first, last, instance);
    _ZN7WebCore18ParsedContentRangeC2Elll(&two, first, last, instance);
    Require(one.m_firstBytePosition == two.m_firstBytePosition);
    Require(one.m_lastBytePosition == two.m_lastBytePosition);
    Require(one.m_instanceLength == two.m_instanceLength);
    return one;
}

static void TestParsedContentRangeFromString() {
    constexpr std::int64_t unknown = std::numeric_limits<std::int64_t>::max();
    Require(IsValid(RangeFrom("bytes 0-1/2")));
    Require(IsValid(RangeFrom("bytes 0-1/*")));
    Require(RangeFrom("bytes 0-1/2").m_firstBytePosition == 0);
    Require(RangeFrom("bytes 0-1/2").m_lastBytePosition == 1);
    Require(RangeFrom("bytes 0-1/2").m_instanceLength == 2);
    Require(RangeFrom("bytes 0-1/*").m_instanceLength == unknown);

    for (const char* text : {"bytes  0-1/*", "bytes 0 -1/*", "bytes 0- 1/*", "bytes 0-1 /*", "bytes 0-1/ *", "bytes 0-1/* ", "bytes 0-1/ 2", "bytes 0-1/2 "})
        Require(!IsValid(RangeFrom(text)));
    for (const char* text : {"bytes abcd-1/2", "bytes 0-abcd/2", "bytes 0-1/abcd"})
        Require(!IsValid(RangeFrom(text)));
    for (const char* text : {"bytes 1-0/2", "bytes 0-2/1", "bytes 2/0-1", "abcd 0/1-2"})
        Require(!IsValid(RangeFrom(text)));
    for (const char* text : {"bytes -0-1/*", "bytes -1/*", "bytes 0--0/2", "bytes 0-1/-2"})
        Require(!IsValid(RangeFrom(text)));
    Require(IsValid(RangeFrom("bytes 9223372036854775805-9223372036854775806/9223372036854775807")));
    Require(!IsValid(RangeFrom("bytes 9223372036854775808-9223372036854775809/9223372036854775810")));

    for (const char* text : {"", "bytes", "bytes ", "bytes -", "bytes 0-", "bytes 0-1", "bytes 0-1/", "BYTES 0-1/2", "bytes 0-1/2/3"})
        Require(!IsValid(RangeFrom(text)));

    auto partial = RangeFrom("bytes 5-x/10");
    Require(!IsValid(partial) && partial.m_firstBytePosition == 5 && partial.m_lastBytePosition == 0);
    partial = RangeFrom("bytes 5-9/x");
    Require(!IsValid(partial) && partial.m_firstBytePosition == 5 && partial.m_lastBytePosition == 9);
    partial = RangeFrom("bytes 7-3/9");
    Require(!IsValid(partial) && partial.m_firstBytePosition == 7 && partial.m_lastBytePosition == 3);

    WebCore::ParsedContentRange fromNull{};
    String null;
    _ZN7WebCore18ParsedContentRangeC1ERKN3WTF6StringE(&fromNull, null);
    Require(!IsValid(fromNull));
    Require(IsValid(RangeFrom("bytes 0-0/1")));
    Require(!IsValid(RangeFrom("bytes 0-1/1")));
    Require(IsValid(RangeFrom("bytes 3-3/*")));
}

static void TestParsedContentRangeFromValues() {
    constexpr std::int64_t unknown = std::numeric_limits<std::int64_t>::max();
    Require(IsValid(RangeFromValues(0, 1, 2)));
    Require(IsValid(RangeFromValues(0, 1, unknown)));
    Require(!IsValid(RangeFromValues(1, 0, 2)));
    Require(!IsValid(RangeFromValues(0, 2, 1)));
    Require(!IsValid(RangeFromValues(0, 0, 0)));
    Require(!IsValid(RangeFromValues(-1, 1, 2)));
    Require(!IsValid(RangeFromValues(0, -1, 2)));
    Require(!IsValid(RangeFromValues(0, 1, -2)));
    Require(!IsValid(RangeFromValues(-2, -1, 2)));
    auto range = RangeFromValues(4, 9, 20);
    Require(range.m_firstBytePosition == 4 && range.m_lastBytePosition == 9 && range.m_instanceLength == 20);
    auto invalid = RangeFromValues(9, 4, 20);
    Require(invalid.m_firstBytePosition == 9 && invalid.m_lastBytePosition == 4);
    try {
        _ZN7WebCore18ParsedContentRangeC1Elll(nullptr, 0, 1, 2);
        Require(false);
    } catch (const std::invalid_argument&) {
    }
}

static void TestParsedContentRangeHeaderValue() {
    constexpr std::int64_t unknown = std::numeric_limits<std::int64_t>::max();
    auto known = RangeFromValues(0, 1, 2);
    auto open = RangeFromValues(0, 1, unknown);
    Require(Text(_ZNK7WebCore18ParsedContentRange11headerValueEv(&known)) == "bytes 0-1/2");
    Require(Text(_ZNK7WebCore18ParsedContentRange11headerValueEv(&open)) == "bytes 0-1/*");
    WebCore::ParsedContentRange invalid{0, 0, std::numeric_limits<std::int64_t>::min()};
    auto empty = _ZNK7WebCore18ParsedContentRange11headerValueEv(&invalid);
    Require(Text(empty).empty());
    Require(empty.impl == nullptr);
    auto large = RangeFrom("bytes 9223372036854775805-9223372036854775806/9223372036854775807");
    Require(Text(_ZNK7WebCore18ParsedContentRange11headerValueEv(&large)) == "bytes 9223372036854775805-9223372036854775806/*");
    Require(large.m_instanceLength == std::numeric_limits<std::int64_t>::max());
    auto nearLimit = RangeFrom("bytes 9223372036854775804-9223372036854775805/9223372036854775806");
    Require(Text(_ZNK7WebCore18ParsedContentRange11headerValueEv(&nearLimit)) == "bytes 9223372036854775804-9223372036854775805/9223372036854775806");
}

struct ContentTypeCase {
    const char* input;
    bool valid;
};

static void TestIsValidContentTypeMimeSniff() {
    constexpr auto mode = WebCore::Mode::MimeSniff;
    const ContentTypeCase cases[] = {
        {"text/plain", true}, {" text/plain", true}, {" text/plain ", true}, {"text /plain", false}, {"text/ plain", false},
        {"text / plain", false}, {"te xt/plain", false}, {"text/pla in", false}, {"text", false}, {"text/", false}, {"/plain", false},
        {"text/plain;", true}, {"text/plain;;", true}, {"text/plain;;;", true}, {"text/plain;test", true}, {"text/plain; test", true},
        {"text/plain;test=", true}, {"text/plain;test=value", true}, {"text/plain;;test=value", true}, {"text/plain;;;test=value", true},
        {"text/plain;test=value;", true}, {"text/plain;test=value;;", true}, {"text/plain;test=value;;;", true}, {"text/plain; test=value", true},
        {"text/plain;test =value", true}, {"text/plain;test= value", true}, {"text/plain;test=value ", true}, {"text/plain;=;test=value", true},
        {"text/plain;test=value;=", true}, {"text/plain;wrong=;test=value", true}, {"text/plain;=wrong;test=value", true},
        {"text/plain;test=value;wrong=", true}, {"text/plain;test=value;=wrong", true}, {"text/plain ;test=value", true},
        {"text/plain\n;test=value", true}, {"text/plain\r;test=value", true}, {"text/plain\t;test=value", true},
        {"text/plain;test=value ;test=value", true}, {"text/plain;test=value\n;test=value", true}, {"text/plain;test=value\r;test=value", true},
        {"text/plain;test=value\t;test=value", true}, {"text/plain;test=\"value\"", true}, {"text/plain;test=\"value", true},
        {"text/plain;test=\"value\"foo", true}, {"text/plain;test=\"value\"foo;foo=bar", true},
        {"", false}, {" ", false}, {"  ", false}, {"\t", false}, {";", false}, {";=", false}, {"=", false}, {"=;", false}, {"text/ ", false},
        {"{/}", false}, {"x/x ;x=x", true}, {"x/x\n;x=x", true}, {"x/x\r;x=x", true}, {"x/x\t;x=x", true}, {"\ttext/plain", true},
        {"\ntext/plain", true}, {"\rtext/plain", true}, {"\btext/plain", false}, {"\x0Btext/plain", false}, {"text/plain ", true},
        {"text/plain\t", true}, {"text/plain\n", true}, {"text/plain\r", true}, {"text/plain\b", false}, {"text/plain\x0B", false},
        {"\ttext/plain\t", true}, {"\ntext/plain\n", true}, {"\rtext/plain\r", true}, {"\btext/plain\b", false}, {"text/PLAIN", true},
        {"text\t/plain", false}, {"text/\tplain", false}, {"text\t/\tplain", false}, {"te\txt/plain", false}, {"text/pla\tin", false},
        {"text\n/plain", false}, {"text/\nplain", false}, {"text\n/\nplain", false}, {"te\nxt/plain", false}, {"text/pla\nin", false},
        {"text\r/plain", false}, {"text/\rplain", false}, {"text\r/\rplain", false}, {"te\rxt/plain", false}, {"text/pla\rin", false},
        {"text/plain;\ttest", true}, {"text/plain;\ntest", true}, {"text/plain;\rtest", true}, {"text/plain;\btest", true},
        {"text/plain;test ", true}, {"text/plain;test\t", true}, {"text/plain;test\n", true}, {"text/plain;test\r", true}, {"text/plain;test\b", true},
        {"text/plain;TEST=value", true}, {"text/plain;test=VALUE", true}, {"text/plain;\ttest=value", true}, {"text/plain;\ntest=value", true},
        {"text/plain;\rtest=value", true}, {"text/plain;\btest=value", true}, {"text/plain;test\t=value", true}, {"text/plain;test\n=value", true},
        {"text/plain;test\r=value", true}, {"text/plain;test\b=value", true}, {"text/plain;test=\tvalue", true}, {"text/plain;test=\nvalue", true},
        {"text/plain;test=\rvalue", true}, {"text/plain;test=\bvalue", true}, {"text/plain;test=value\t", true}, {"text/plain;test=value\n", true},
        {"text/plain;test=value\r", true}, {"text/plain;test=value\b", true}, {"text/plain;test=\"", true}, {"text/plain;test=\"val\\ue\"", true},
        {"text/plain;test=\"val\\\"ue\"", true}, {"text/plain;test='value'", true}, {"text/plain;test='value", true}, {"text/plain;test=value'", true},
        {"text/plain;test={value}", true},
    };
    for (const auto& entry : cases) {
        if (Valid(entry.input, mode) != entry.valid) {
            std::fprintf(stderr, "MimeSniff mismatch for [%s]\n", entry.input);
            std::abort();
        }
    }
    Require(!_ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(String(), mode));
}

static void TestIsValidContentTypeRfc2045() {
    constexpr auto mode = WebCore::Mode::Rfc2045;
    const ContentTypeCase cases[] = {
        {"text/plain", true}, {" text/plain", true}, {" text/plain ", true}, {"text /plain", false}, {"text/ plain", false},
        {"text / plain", false}, {"te xt/plain", false}, {"text/pla in", true}, {"text", false}, {"text/", false}, {"/plain", false},
        {"text/plain;", false}, {"text/plain;;", false}, {"text/plain;;;", false}, {"text/plain;test", false}, {"text/plain; test", false},
        {"text/plain;test=", false}, {"text/plain;test=value", true}, {"text/plain;;test=value", false}, {"text/plain;;;test=value", false},
        {"text/plain;test=value;", true}, {"text/plain;test=value;;", false}, {"text/plain;test=value;;;", false}, {"text/plain; test=value", true},
        {"text/plain;test =value", false}, {"text/plain;test= value", false}, {"text/plain;test=value ", false}, {"text/plain;=;test=value", false},
        {"text/plain;test=value;=", false}, {"text/plain;wrong=;test=value", false}, {"text/plain;=wrong;test=value", false},
        {"text/plain;test=value;wrong=", false}, {"text/plain;test=value;=wrong", false}, {"text/plain;test=\"value\"", true},
        {"text/plain;test=\"value", false}, {"text/plain;test=\"value\"foo", false}, {"text/plain;test=\"value\"foo;foo=bar", false},
        {"text/plain\r\n", false}, {"text/plain;a=b\n", false}, {"text/plain\n;a=b", false},
    };
    for (const auto& entry : cases) {
        if (Valid(entry.input, mode) != entry.valid) {
            std::fprintf(stderr, "Rfc2045 mismatch for [%s]\n", entry.input);
            std::abort();
        }
    }
    Require(!_ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(String(), mode));
    String value = Narrow("text/plain");
    try {
        _ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(value, static_cast<WebCore::Mode>(2));
        Require(false);
    } catch (const std::invalid_argument&) {
    }
}

static WebCore::ContentType MakeContentType(const std::string& text) {
    WebCore::ContentType type;
    String value = Narrow(text);
    _ZN7WebCore11ContentTypeC1ERKN3WTF6StringE(&type, value);
    Require(type.m_type.impl == value.impl);
    return type;
}

static std::string Parameter(const WebCore::ContentType& type, const std::string& name) {
    std::string result;
    bool first = true;
    ForBothWidths(name, [&](const String& value) {
        auto parameter = _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(&type, value);
        if (!first) Require(Text(parameter) == result);
        result = Text(parameter);
        first = false;
    });
    auto missing = _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(&type, String());
    Require(missing.impl == nullptr);
    return result;
}

static bool ParameterIsNull(const WebCore::ContentType& type, const std::string& name) {
    String value = Narrow(name);
    return _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(&type, value).impl == nullptr;
}

static void TestContentType() {
    const auto& codecs = _ZN7WebCore11ContentType15codecsParameterEv();
    Require(Text(codecs) == "codecs");
    Require(&codecs == &_ZN7WebCore11ContentType15codecsParameterEv());

    WebCore::ContentType moved;
    String source = Narrow("video/mp4");
    auto* sourceImpl = source.impl;
    _ZN7WebCore11ContentTypeC1EON3WTF6StringE(&moved, std::move(source));
    Require(moved.m_type.impl == sourceImpl && source.impl == nullptr);
    WebCore::ContentType movedAgain;
    String second = Narrow("audio/ogg");
    _ZN7WebCore11ContentTypeC2EON3WTF6StringE(&movedAgain, std::move(second));
    Require(second.impl == nullptr && Text(movedAgain.m_type) == "audio/ogg");
    WebCore::ContentType copied;
    _ZN7WebCore11ContentTypeC2ERKN3WTF6StringE(&copied, movedAgain.m_type);
    Require(copied.m_type.impl == movedAgain.m_type.impl);

    auto mp4 = MakeContentType("video/mp4; codecs=\"avc1.4d401e, mp4a.40.2\"");
    Require(Text(_ZNK7WebCore11ContentType13containerTypeEv(&mp4)) == "video/mp4");
    Require(Parameter(mp4, "codecs") == "avc1.4d401e, mp4a.40.2");
    Require(Parameter(mp4, "CODECS") == "avc1.4d401e, mp4a.40.2");
    Require(ParameterIsNull(mp4, "profiles"));

    auto spaced = MakeContentType("  text/html ;charset=utf-8 ; other = x ");
    Require(Text(_ZNK7WebCore11ContentType13containerTypeEv(&spaced)) == "text/html");
    Require(Parameter(spaced, "charset") == "utf-8");
    Require(Parameter(spaced, "other") == "x");

    auto suffix = MakeContentType("audio/x; xcodecs=foo");
    Require(Parameter(suffix, "codecs") == "foo");

    auto noSemicolon = MakeContentType("text/plain");
    Require(ParameterIsNull(noSemicolon, "charset"));
    Require(Text(_ZNK7WebCore11ContentType13containerTypeEv(&noSemicolon)) == "text/plain");

    auto noEquals = MakeContentType("a/b; codecs");
    Require(ParameterIsNull(noEquals, "codecs"));

    auto oneQuote = MakeContentType("a/b; codecs=\"x");
    Require(Parameter(oneQuote, "codecs") == "\"x");

    auto beforeEquals = MakeContentType("a/b; \"q\" codecs=x; y=z");
    Require(Parameter(beforeEquals, "codecs") == "x");

    auto quotedSemicolon = MakeContentType("a/b; codecs=\"x;y\"; z=1");
    Require(Parameter(quotedSemicolon, "codecs") == "x;y");

    auto emptyValue = MakeContentType("a/b; codecs=");
    auto emptyResult = _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(&emptyValue, _ZN7WebCore11ContentType15codecsParameterEv());
    Require(emptyResult.impl != nullptr && emptyResult.impl->length == 0);

    auto trailing = MakeContentType("a/b; codecs=x ;");
    Require(Parameter(trailing, "codecs") == "x");

    auto emptyName = MakeContentType("a/b; codecs=x");
    auto emptyNameString = Narrow("");
    auto emptyNameResult = _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(&emptyName, emptyNameString);
    Require(Text(emptyNameResult) == "x");

    auto nameCaseInSource = MakeContentType("a/b; CoDeCs=\"x\"");
    Require(Parameter(nameCaseInSource, "codecs") == "x");

    WebCore::ContentType nullType;
    Require(_ZNK7WebCore11ContentType13containerTypeEv(&nullType).impl == nullptr);
    Require(ParameterIsNull(nullType, "codecs"));

    auto whitespaceOnly = MakeContentType(" \t ; x=y");
    auto whitespaceResult = _ZNK7WebCore11ContentType13containerTypeEv(&whitespaceOnly);
    Require(whitespaceResult.impl != nullptr && whitespaceResult.impl->length == 0);

    auto wide = WebCore::ContentType{Wide(u"video/webm; codecs=\"vp9, opus\"")};
    Require(Text(_ZNK7WebCore11ContentType13containerTypeEv(&wide)) == "video/webm");
    Require(Parameter(wide, "codecs") == "vp9, opus");

    try {
        _ZNK7WebCore11ContentType13containerTypeEv(nullptr);
        Require(false);
    } catch (const std::invalid_argument&) {
    }
}

static void TestCredentialBase() {
    constexpr auto none = WebCore::CredentialPersistenceNone;
    constexpr auto session = WebCore::CredentialPersistenceForSession;
    constexpr auto permanent = WebCore::CredentialPersistencePermanent;

    WebCore::Credential empty;
    _ZN7WebCore14CredentialBaseC1Ev(&empty);
    Require(_ZNK7WebCore14CredentialBase7isEmptyEv(&empty));
    Require(!_ZNK7WebCore14CredentialBase11hasPasswordEv(&empty));
    Require(_ZNK7WebCore14CredentialBase11persistenceEv(&empty) == none);
    Require(_ZNK7WebCore14CredentialBase4userEv(&empty).impl != nullptr);
    Require(_ZNK7WebCore14CredentialBase8passwordEv(&empty).impl != nullptr);
    Require(_ZNK7WebCore14CredentialBase4userEv(&empty).impl->length == 0);

    WebCore::Credential other;
    _ZN7WebCore14CredentialBaseC2Ev(&other);
    Require(_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(empty, other));

    String user = Narrow("alice");
    String password = Wide(u"s3cret");
    WebCore::Credential credential;
    _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&credential, user, password, session);
    Require(!_ZNK7WebCore14CredentialBase7isEmptyEv(&credential));
    Require(_ZNK7WebCore14CredentialBase11hasPasswordEv(&credential));
    Require(_ZNK7WebCore14CredentialBase11persistenceEv(&credential) == session);
    Require(_ZNK7WebCore14CredentialBase4userEv(&credential).impl == user.impl);
    Require(_ZNK7WebCore14CredentialBase8passwordEv(&credential).impl == password.impl);
    Require(Text(_ZNK7WebCore14CredentialBase4userEv(&credential)) == "alice");
    Require(Text(_ZNK7WebCore14CredentialBase8passwordEv(&credential)) == "s3cret");

    WebCore::Credential twin;
    _ZN7WebCore14CredentialBaseC2ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&twin, Narrow("alice"), Narrow("s3cret"), session);
    Require(_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(credential, twin));
    Require(_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(credential, credential));

    WebCore::Credential differentPersistence;
    _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&differentPersistence, user, password, permanent);
    Require(!_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(credential, differentPersistence));

    WebCore::Credential differentUser;
    _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&differentUser, Narrow("bob"), password, session);
    Require(!_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(credential, differentUser));

    WebCore::Credential differentPassword;
    _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&differentPassword, user, Narrow("other"), session);
    Require(!_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(credential, differentPassword));

    WebCore::Credential nullInputs;
    _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&nullInputs, String(), String(), permanent);
    Require(_ZNK7WebCore14CredentialBase7isEmptyEv(&nullInputs));
    Require(_ZNK7WebCore14CredentialBase4userEv(&nullInputs).impl != nullptr);
    Require(_ZNK7WebCore14CredentialBase8passwordEv(&nullInputs).impl != nullptr);
    Require(_ZNK7WebCore14CredentialBase11persistenceEv(&nullInputs) == permanent);
    WebCore::Credential permanentEmpty;
    _ZN7WebCore14CredentialBaseC1Ev(&permanentEmpty);
    permanentEmpty.m_persistence = permanent;
    Require(_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(nullInputs, permanentEmpty));

    WebCore::Credential userOnly;
    _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(&userOnly, user, String(), none);
    Require(!_ZNK7WebCore14CredentialBase7isEmptyEv(&userOnly));
    Require(!_ZNK7WebCore14CredentialBase11hasPasswordEv(&userOnly));

    WebCore::Credential nullFields;
    new (&nullFields.m_user) String();
    new (&nullFields.m_password) String();
    nullFields.m_persistence = none;
    Require(_ZNK7WebCore14CredentialBase7isEmptyEv(&nullFields));
    Require(!_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(nullFields, empty));
    Require(_ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(nullFields, nullFields));

    try {
        _ZNK7WebCore14CredentialBase4userEv(nullptr);
        Require(false);
    } catch (const std::invalid_argument&) {
    }
}

static void TestProtectionSpaceBase() {
    for (int serverType = 0; serverType <= 9; ++serverType) {
        for (int scheme : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 50, 100, 255}) {
            WebCore::ProtectionSpaceBase space;
            new (&space.m_host) String();
            new (&space.m_realm) String();
            space.m_port = 80;
            space.m_serverType = static_cast<std::uint8_t>(serverType);
            space.m_authenticationScheme = static_cast<std::uint8_t>(scheme);
            space.m_isHashTableDeletedValue = false;
            bool proxy = serverType >= 5 && serverType <= 8;
            bool secure = serverType == 2 || serverType == 4 || serverType == 6 || scheme == 3;
            bool password = !(scheme == 7 || scheme == 8 || scheme == 100);
            Require(_ZNK7WebCore19ProtectionSpaceBase7isProxyEv(&space) == proxy);
            Require(_ZNK7WebCore19ProtectionSpaceBase26receivesCredentialSecurelyEv(&space) == secure);
            Require(_ZNK7WebCore19ProtectionSpaceBase15isPasswordBasedEv(&space) == password);
        }
    }
    try {
        _ZNK7WebCore19ProtectionSpaceBase7isProxyEv(nullptr);
        Require(false);
    } catch (const std::invalid_argument&) {
    }
}

int main() {
    TestParsedContentRangeFromString();
    TestParsedContentRangeFromValues();
    TestParsedContentRangeHeaderValue();
    TestIsValidContentTypeMimeSniff();
    TestIsValidContentTypeRfc2045();
    TestContentType();
    TestCredentialBase();
    TestProtectionSpaceBase();
    return 0;
}
