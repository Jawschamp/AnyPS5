#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/WebCoreNetworkExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using NKWebKit::WtfAbi::String;
using NKWebKit::WtfAbi::StringImpl;
using WebCore::Mode;

constexpr std::uint8_t HTTPS = 2;
constexpr std::uint8_t FTPS = 4;
constexpr std::uint8_t ProxyHTTP = 5;
constexpr std::uint8_t ProxyHTTPS = 6;
constexpr std::uint8_t ProxyFTP = 7;
constexpr std::uint8_t ProxySOCKS = 8;
constexpr std::uint8_t HTTPDigest = 3;
constexpr std::uint8_t ClientCertificateRequested = 7;
constexpr std::uint8_t ServerTrustEvaluationRequested = 8;
constexpr std::uint8_t UnknownScheme = 100;

constexpr std::size_t NotFound = std::numeric_limits<std::size_t>::max();

template<typename T> T& Checked(T* value) {
    if (!value) throw std::invalid_argument("null WebCore object");
    return *value;
}

unsigned LengthOf(const String& string) {
    return string.impl ? string.impl->length : 0;
}

char16_t UnitAt(const StringImpl* impl, unsigned index) {
    return (impl->hashAndFlags & 4) ? static_cast<const unsigned char*>(impl->data)[index] : static_cast<const char16_t*>(impl->data)[index];
}

std::u16string Units(const String& string) {
    std::u16string result;
    if (!string.impl) return result;
    result.reserve(string.impl->length);
    for (unsigned i = 0; i < string.impl->length; ++i) result.push_back(UnitAt(string.impl, i));
    return result;
}

String Make(const std::u16string& units) {
    if (units.size() > 0x7FFFFFFFU) throw std::length_error("WebCore string exceeds maximum length");
    return _ZN3WTF10StringImpl20create8BitIfPossibleEPKDsj(units.data(), static_cast<unsigned>(units.size()));
}

String MakeAscii(const std::string& text) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(text.data()), static_cast<unsigned>(text.size()));
}

String EmptyString() {
    return _ZN3WTF10StringImpl6createEPKhj(nullptr, 0);
}

bool EqualStrings(const String& a, const String& b) {
    if (a.impl == b.impl) return true;
    if (!a.impl || !b.impl) return false;
    if (a.impl->length != b.impl->length) return false;
    for (unsigned i = 0; i < a.impl->length; ++i) {
        if (UnitAt(a.impl, i) != UnitAt(b.impl, i)) return false;
    }
    return true;
}

bool IsDigit(char16_t c) {
    return c >= u'0' && c <= u'9';
}

std::optional<std::int64_t> ParseDigits(const std::u16string& digits) {
    if (digits.empty()) return std::nullopt;
    std::int64_t value = 0;
    for (char16_t c : digits) {
        if (!IsDigit(c)) return std::nullopt;
        int digit = c - u'0';
        if (value > (std::numeric_limits<std::int64_t>::max() - digit) / 10) return std::nullopt;
        value = value * 10 + digit;
    }
    return value;
}

constexpr std::int64_t InvalidLength = std::numeric_limits<std::int64_t>::min();
constexpr std::int64_t UnknownLength = std::numeric_limits<std::int64_t>::max();

bool AreContentRangeValuesValid(std::int64_t first, std::int64_t last, std::int64_t instance) {
    if (first < 0) return false;
    if (last < first) return false;
    if (instance == UnknownLength) return true;
    return last < instance;
}

bool OnlyDigits(const std::u16string& text) {
    for (char16_t c : text) {
        if (!IsDigit(c)) return false;
    }
    return true;
}

bool ParseContentRange(const std::u16string& value, std::int64_t& first, std::int64_t& last, std::int64_t& instance) {
    constexpr std::size_t prefixLength = 6;
    if (value.compare(0, prefixLength, u"bytes ") != 0 || value.size() < prefixLength) return false;
    std::size_t dash = value.find(u'-', prefixLength);
    if (dash == std::u16string::npos) return false;
    std::size_t slash = value.find(u'/', dash + 1);
    if (slash == std::u16string::npos) return false;
    std::u16string firstText = value.substr(prefixLength, dash - prefixLength);
    if (!OnlyDigits(firstText)) return false;
    auto firstValue = ParseDigits(firstText);
    if (!firstValue) return false;
    first = *firstValue;
    std::u16string lastText = value.substr(dash + 1, slash - (dash + 1));
    if (!OnlyDigits(lastText)) return false;
    auto lastValue = ParseDigits(lastText);
    if (!lastValue) return false;
    last = *lastValue;
    std::u16string instanceText = value.substr(slash + 1);
    if (instanceText == u"*") {
        instance = UnknownLength;
    } else {
        if (!OnlyDigits(instanceText)) return false;
        auto instanceValue = ParseDigits(instanceText);
        if (!instanceValue) return false;
        instance = *instanceValue;
    }
    return AreContentRangeValuesValid(first, last, instance);
}

void InitializeParsedContentRange(WebCore::ParsedContentRange* self, const String& headerValue) {
    auto& range = Checked(self);
    range.m_firstBytePosition = 0;
    range.m_lastBytePosition = 0;
    range.m_instanceLength = InvalidLength;
    if (!ParseContentRange(Units(headerValue), range.m_firstBytePosition, range.m_lastBytePosition, range.m_instanceLength))
        range.m_instanceLength = InvalidLength;
}

void InitializeParsedContentRange(WebCore::ParsedContentRange* self, std::int64_t first, std::int64_t last, std::int64_t instance) {
    auto& range = Checked(self);
    range.m_firstBytePosition = first;
    range.m_lastBytePosition = last;
    range.m_instanceLength = instance;
    if (!AreContentRangeValuesValid(first, last, instance)) range.m_instanceLength = InvalidLength;
}

void InitializeCredential(WebCore::CredentialBase* self, const String& user, const String& password, WebCore::CredentialPersistence persistence) {
    auto& credential = Checked(self);
    new (&credential.m_user) String(LengthOf(user) ? user : EmptyString());
    new (&credential.m_password) String(LengthOf(password) ? password : EmptyString());
    credential.m_persistence = persistence;
}

void InitializeEmptyCredential(WebCore::CredentialBase* self) {
    auto& credential = Checked(self);
    new (&credential.m_user) String(EmptyString());
    new (&credential.m_password) String(EmptyString());
    credential.m_persistence = WebCore::CredentialPersistenceNone;
}

struct Token {
    bool isNull{true};
    std::size_t start{0};
    std::size_t length{0};
};

bool IsHTTPSpace(char16_t c) {
    return c == u' ' || c == u'\n' || c == u'\t' || c == u'\r';
}

bool IsRfc2045TokenCharacter(char16_t c) {
    return c < 0x80 && c > u' ' && c != u'"' && c != u'(' && c != u')' && c != u',' && c != u'/' && (c < u':' || c > u'@') && (c < u'[' || c > u']');
}

bool IsHTTPTokenCharacter(char16_t c) {
    return (c >= u'a' && c <= u'z') || (c >= u'A' && c <= u'Z') || IsDigit(c) || c == u'!' || c == u'#' || c == u'$' || c == u'%' || c == u'&'
        || c == u'\'' || c == u'*' || c == u'+' || c == u'-' || c == u'.' || c == u'^' || c == u'_' || c == u'`' || c == u'|' || c == u'~';
}

void SkipSpaces(const std::u16string& input, std::size_t& index) {
    while (index < input.size() && IsHTTPSpace(input[index])) ++index;
}

template<typename Predicate> Token ParseToken(const std::u16string& input, std::size_t& index, Predicate accepts, Mode mode, bool skipTrailingWhitespace = false) {
    std::size_t start = index;
    std::size_t& end = index;
    if (end >= input.size()) return {};
    while (end < input.size() && accepts(input[end])) {
        if (mode == Mode::Rfc2045 && !IsRfc2045TokenCharacter(input[end])) break;
        ++end;
    }
    if (end == start) return {};
    if (skipTrailingWhitespace) {
        if (mode == Mode::Rfc2045) {
            while (input[end - 1] == u' ') --end;
        } else {
            while (IsHTTPSpace(input[end - 1])) --end;
        }
    }
    return {false, start, end - start};
}

bool ContainsNonTokenCharacters(const std::u16string& input, const Token& token, Mode mode) {
    if (mode == Mode::MimeSniff) {
        if (!token.length) return true;
        for (std::size_t i = 0; i < token.length; ++i) {
            if (!IsHTTPTokenCharacter(input[token.start + i])) return true;
        }
        return false;
    }
    for (std::size_t i = 0; i < token.length; ++i) {
        if (!IsRfc2045TokenCharacter(input[token.start + i])) return true;
    }
    return false;
}

Token ParseQuotedString(const std::u16string& input, std::size_t& index) {
    std::size_t quotedStart = index + 1;
    std::size_t& end = index;
    if (end >= input.size()) return {};
    if (input[end++] != u'"' || end >= input.size()) return {};
    bool lastWasBackslash = false;
    char current;
    while ((current = static_cast<char>(input[end++])) != '"' || lastWasBackslash) {
        if (end >= input.size()) return {};
        if (current == '\\' && !lastWasBackslash) {
            lastWasBackslash = true;
            continue;
        }
        if (lastWasBackslash) lastWasBackslash = false;
    }
    if (input[end - 1] == u'"') ++end;
    std::size_t length = end - quotedStart;
    if (quotedStart + length > input.size()) length = input.size() - quotedStart;
    return {false, quotedStart, length};
}

void SkipHTTPQuotedString(const std::u16string& input, std::size_t& position) {
    ++position;
    while (true) {
        ParseToken(input, position, [](char16_t c) { return c != u'"' && c != u'\\'; }, Mode::MimeSniff);
        if (position >= input.size()) break;
        char16_t quoteOrBackslash = input[position++];
        if (quoteOrBackslash == u'\\') {
            if (position >= input.size()) break;
            ++position;
        } else {
            break;
        }
    }
}

bool ParseContentType(const std::u16string& input, Mode mode) {
    if (mode == Mode::Rfc2045 && input.find_first_of(u"\r\n") != std::u16string::npos) return false;
    std::size_t index = 0;
    const std::size_t length = input.size();
    SkipSpaces(input, index);
    if (index >= length) return false;
    std::size_t contentTypeStart = index;
    auto type = ParseToken(input, index, [](char16_t c) { return c != u'/'; }, mode);
    if (type.isNull || ContainsNonTokenCharacters(input, type, mode)) return false;
    if (index >= length || input[index++] != u'/') return false;
    auto subtype = ParseToken(input, index, [](char16_t c) { return c != u';'; }, mode, mode == Mode::MimeSniff);
    if (subtype.isNull || ContainsNonTokenCharacters(input, subtype, mode)) return false;
    std::size_t semicolon = input.find(u';', contentTypeStart);
    if (semicolon == std::u16string::npos) return true;
    index = semicolon + 1;
    while (true) {
        SkipSpaces(input, index);
        auto key = ParseToken(input, index, [](char16_t c) { return c != u';' && c != u'='; }, mode);
        if (mode == Mode::Rfc2045 && (key.isNull || index >= length)) return false;
        if (mode == Mode::Rfc2045) {
            if (index >= length || input[index++] != u'=') return false;
        } else {
            if (index >= length) break;
            if (input[index] != u'=' && input[index] != u';') return false;
            if (input[index++] == u';') continue;
        }
        Token value;
        bool haveQuotedValue = false;
        if (index < length && input[index] == u'"') {
            if (mode == Mode::MimeSniff) {
                haveQuotedValue = true;
                SkipHTTPQuotedString(input, index);
                ParseToken(input, index, [](char16_t c) { return c != u';'; }, mode);
            } else {
                value = ParseQuotedString(input, index);
            }
        } else {
            value = ParseToken(input, index, [](char16_t c) { return c != u';'; }, mode, mode == Mode::MimeSniff);
        }
        if (!haveQuotedValue && value.isNull) {
            if (mode == Mode::MimeSniff) continue;
            return false;
        }
        if (mode == Mode::Rfc2045 && index < length && input[index++] != u';') return false;
        if (index >= length) return true;
    }
    return true;
}

bool IsASCIIWhitespace(char16_t c) {
    return c == u' ' || c == u'\n' || c == u'\t' || c == u'\r' || c == u'\f';
}

std::u16string Trim(const std::u16string& text, bool (*predicate)(char16_t)) {
    std::size_t start = 0;
    while (start < text.size() && predicate(text[start])) ++start;
    if (start == text.size()) return {};
    std::size_t end = text.size();
    while (end > start && predicate(text[end - 1])) --end;
    return text.substr(start, end - start);
}

std::size_t FindIgnoringASCIICase(const std::u16string& source, const std::u16string& match, std::size_t start) {
    if (match.empty()) return std::min(start, source.size());
    if (start > source.size() || match.size() > source.size() - start) return NotFound;
    auto lower = [](char16_t c) { return c >= u'A' && c <= u'Z' ? static_cast<char16_t>(c + 32) : c; };
    for (std::size_t i = start; i + match.size() <= source.size(); ++i) {
        bool same = true;
        for (std::size_t j = 0; j < match.size() && same; ++j) same = lower(source[i + j]) == lower(match[j]);
        if (same) return i;
    }
    return NotFound;
}

std::size_t Find(const std::u16string& source, char16_t c, std::size_t start) {
    std::size_t found = source.find(c, static_cast<unsigned>(start));
    return found == std::u16string::npos ? NotFound : found;
}

}  // namespace

extern "C" {

const String& APS5_VABI _ZN7WebCore11ContentType15codecsParameterEv() {
    static const String* codecs = new String(MakeAscii("codecs"));
    return *codecs;
}

void APS5_VABI _ZN7WebCore11ContentTypeC1EON3WTF6StringE(WebCore::ContentType* self, String&& type) {
    new (&Checked(self).m_type) String(std::move(type));
}

void APS5_VABI _ZN7WebCore11ContentTypeC1ERKN3WTF6StringE(WebCore::ContentType* self, const String& type) {
    new (&Checked(self).m_type) String(type);
}

void APS5_VABI _ZN7WebCore11ContentTypeC2EON3WTF6StringE(WebCore::ContentType* self, String&& type) {
    new (&Checked(self).m_type) String(std::move(type));
}

void APS5_VABI _ZN7WebCore11ContentTypeC2ERKN3WTF6StringE(WebCore::ContentType* self, const String& type) {
    new (&Checked(self).m_type) String(type);
}

String APS5_VABI _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(const WebCore::ContentType* self, const String& parameterName) {
    auto type = Units(Checked(self).m_type);
    auto semicolon = Find(type, u';', 0);
    if (semicolon == NotFound || !parameterName.impl) return String();
    auto nameStart = FindIgnoringASCIICase(type, Units(parameterName), semicolon + 1);
    if (nameStart == NotFound) return String();
    auto equalSign = Find(type, u'=', nameStart + LengthOf(parameterName));
    if (equalSign == NotFound) return String();
    auto quote = Find(type, u'"', equalSign + 1);
    auto secondQuote = Find(type, u'"', quote + 1);
    std::size_t start;
    std::size_t end;
    if (quote != NotFound && secondQuote != NotFound) {
        start = quote + 1;
        end = secondQuote;
    } else {
        start = equalSign + 1;
        end = Find(type, u';', start);
    }
    std::u16string value = start >= type.size() ? std::u16string() : type.substr(start, end == NotFound ? std::u16string::npos : end - start);
    return Make(Trim(value, IsASCIIWhitespace));
}

String APS5_VABI _ZNK7WebCore11ContentType13containerTypeEv(const WebCore::ContentType* self) {
    auto& type = Checked(self).m_type;
    if (!type.impl) return String();
    auto units = Units(type);
    auto semicolon = Find(units, u';', 0);
    if (semicolon != NotFound) units.resize(semicolon);
    return Make(Trim(units, IsASCIIWhitespace));
}

bool APS5_VABI _ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(const WebCore::Credential& a, const WebCore::Credential& b) {
    if (a.m_persistence != b.m_persistence) return false;
    if (!EqualStrings(a.m_user, b.m_user)) return false;
    if (!EqualStrings(a.m_password, b.m_password)) return false;
    return true;
}

void APS5_VABI _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(
    WebCore::CredentialBase* self, const String& user, const String& password, WebCore::CredentialPersistence persistence) {
    InitializeCredential(self, user, password, persistence);
}

void APS5_VABI _ZN7WebCore14CredentialBaseC1Ev(WebCore::CredentialBase* self) {
    InitializeEmptyCredential(self);
}

void APS5_VABI _ZN7WebCore14CredentialBaseC2ERKN3WTF6StringES4_NS_21CredentialPersistenceE(
    WebCore::CredentialBase* self, const String& user, const String& password, WebCore::CredentialPersistence persistence) {
    InitializeCredential(self, user, password, persistence);
}

void APS5_VABI _ZN7WebCore14CredentialBaseC2Ev(WebCore::CredentialBase* self) {
    InitializeEmptyCredential(self);
}

bool APS5_VABI _ZNK7WebCore14CredentialBase11hasPasswordEv(const WebCore::CredentialBase* self) {
    return LengthOf(Checked(self).m_password) != 0;
}

WebCore::CredentialPersistence APS5_VABI _ZNK7WebCore14CredentialBase11persistenceEv(const WebCore::CredentialBase* self) {
    return Checked(self).m_persistence;
}

const String& APS5_VABI _ZNK7WebCore14CredentialBase4userEv(const WebCore::CredentialBase* self) {
    return Checked(self).m_user;
}

bool APS5_VABI _ZNK7WebCore14CredentialBase7isEmptyEv(const WebCore::CredentialBase* self) {
    auto& credential = Checked(self);
    return !LengthOf(credential.m_user) && !LengthOf(credential.m_password);
}

const String& APS5_VABI _ZNK7WebCore14CredentialBase8passwordEv(const WebCore::CredentialBase* self) {
    return Checked(self).m_password;
}

void APS5_VABI _ZN7WebCore18ParsedContentRangeC1ERKN3WTF6StringE(WebCore::ParsedContentRange* self, const String& headerValue) {
    InitializeParsedContentRange(self, headerValue);
}

void APS5_VABI _ZN7WebCore18ParsedContentRangeC1Elll(WebCore::ParsedContentRange* self, std::int64_t firstBytePosition, std::int64_t lastBytePosition, std::int64_t instanceLength) {
    InitializeParsedContentRange(self, firstBytePosition, lastBytePosition, instanceLength);
}

void APS5_VABI _ZN7WebCore18ParsedContentRangeC2ERKN3WTF6StringE(WebCore::ParsedContentRange* self, const String& headerValue) {
    InitializeParsedContentRange(self, headerValue);
}

void APS5_VABI _ZN7WebCore18ParsedContentRangeC2Elll(WebCore::ParsedContentRange* self, std::int64_t firstBytePosition, std::int64_t lastBytePosition, std::int64_t instanceLength) {
    InitializeParsedContentRange(self, firstBytePosition, lastBytePosition, instanceLength);
}

String APS5_VABI _ZNK7WebCore18ParsedContentRange11headerValueEv(const WebCore::ParsedContentRange* self) {
    auto& range = Checked(self);
    if (range.m_instanceLength == InvalidLength) return String();
    std::string text = "bytes " + std::to_string(range.m_firstBytePosition) + "-" + std::to_string(range.m_lastBytePosition) + "/";
    text += range.m_instanceLength == UnknownLength ? std::string("*") : std::to_string(range.m_instanceLength);
    return MakeAscii(text);
}

bool APS5_VABI _ZNK7WebCore19ProtectionSpaceBase7isProxyEv(const WebCore::ProtectionSpaceBase* self) {
    auto serverType = Checked(self).m_serverType;
    return serverType == ProxyHTTP || serverType == ProxyHTTPS || serverType == ProxyFTP || serverType == ProxySOCKS;
}

bool APS5_VABI _ZNK7WebCore19ProtectionSpaceBase26receivesCredentialSecurelyEv(const WebCore::ProtectionSpaceBase* self) {
    auto& space = Checked(self);
    return space.m_serverType == HTTPS || space.m_serverType == FTPS || space.m_serverType == ProxyHTTPS || space.m_authenticationScheme == HTTPDigest;
}

bool APS5_VABI _ZNK7WebCore19ProtectionSpaceBase15isPasswordBasedEv(const WebCore::ProtectionSpaceBase* self) {
    switch (Checked(self).m_authenticationScheme) {
    case ClientCertificateRequested:
    case ServerTrustEvaluationRequested:
    case UnknownScheme:
        return false;
    default:
        return true;
    }
}

bool APS5_VABI _ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(const String& contentType, Mode mode) {
    if (mode != Mode::Rfc2045 && mode != Mode::MimeSniff) throw std::invalid_argument("invalid WebCore content type mode");
    auto units = Units(contentType);
    if (mode == Mode::MimeSniff) units = Trim(units, IsHTTPSpace);
    return ParseContentType(units, mode);
}

}
