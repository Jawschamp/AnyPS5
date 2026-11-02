#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"
#include <cstddef>
#include <cstdint>

namespace WebCore {

enum CredentialPersistence : int {
    CredentialPersistenceNone,
    CredentialPersistenceForSession,
    CredentialPersistencePermanent
};

struct CredentialBase {
    NKWebKit::WtfAbi::String m_user;
    NKWebKit::WtfAbi::String m_password;
    CredentialPersistence m_persistence;
};

struct Credential : CredentialBase {};

struct ParsedContentRange {
    std::int64_t m_firstBytePosition;
    std::int64_t m_lastBytePosition;
    std::int64_t m_instanceLength;
};

struct ContentType {
    NKWebKit::WtfAbi::String m_type;
};

struct ProtectionSpaceBase {
    NKWebKit::WtfAbi::String m_host;
    NKWebKit::WtfAbi::String m_realm;
    int m_port;
    std::uint8_t m_serverType;
    std::uint8_t m_authenticationScheme;
    bool m_isHashTableDeletedValue;
};

enum class Mode : int {
    Rfc2045,
    MimeSniff
};

static_assert(sizeof(CredentialBase) == 24);
static_assert(offsetof(CredentialBase, m_user) == 0);
static_assert(offsetof(CredentialBase, m_password) == 8);
static_assert(offsetof(CredentialBase, m_persistence) == 16);
static_assert(sizeof(Credential) == sizeof(CredentialBase));
static_assert(sizeof(ParsedContentRange) == 24);
static_assert(offsetof(ParsedContentRange, m_firstBytePosition) == 0);
static_assert(offsetof(ParsedContentRange, m_lastBytePosition) == 8);
static_assert(offsetof(ParsedContentRange, m_instanceLength) == 16);
static_assert(sizeof(ContentType) == 8);
static_assert(sizeof(ProtectionSpaceBase) == 24);
static_assert(offsetof(ProtectionSpaceBase, m_realm) == 8);
static_assert(offsetof(ProtectionSpaceBase, m_port) == 16);
static_assert(offsetof(ProtectionSpaceBase, m_serverType) == 20);
static_assert(offsetof(ProtectionSpaceBase, m_authenticationScheme) == 21);
static_assert(offsetof(ProtectionSpaceBase, m_isHashTableDeletedValue) == 22);

}  // namespace WebCore

extern "C" {

const NKWebKit::WtfAbi::String& APS5_VABI _ZN7WebCore11ContentType15codecsParameterEv();
void APS5_VABI _ZN7WebCore11ContentTypeC1EON3WTF6StringE(WebCore::ContentType* self, NKWebKit::WtfAbi::String&& type);
void APS5_VABI _ZN7WebCore11ContentTypeC1ERKN3WTF6StringE(WebCore::ContentType* self, const NKWebKit::WtfAbi::String& type);
void APS5_VABI _ZN7WebCore11ContentTypeC2EON3WTF6StringE(WebCore::ContentType* self, NKWebKit::WtfAbi::String&& type);
void APS5_VABI _ZN7WebCore11ContentTypeC2ERKN3WTF6StringE(WebCore::ContentType* self, const NKWebKit::WtfAbi::String& type);
bool APS5_VABI _ZN7WebCore14CredentialBase7compareERKNS_10CredentialES3_(const WebCore::Credential& a, const WebCore::Credential& b);
void APS5_VABI _ZN7WebCore14CredentialBaseC1ERKN3WTF6StringES4_NS_21CredentialPersistenceE(WebCore::CredentialBase* self, const NKWebKit::WtfAbi::String& user, const NKWebKit::WtfAbi::String& password, WebCore::CredentialPersistence persistence);
void APS5_VABI _ZN7WebCore14CredentialBaseC1Ev(WebCore::CredentialBase* self);
void APS5_VABI _ZN7WebCore14CredentialBaseC2ERKN3WTF6StringES4_NS_21CredentialPersistenceE(WebCore::CredentialBase* self, const NKWebKit::WtfAbi::String& user, const NKWebKit::WtfAbi::String& password, WebCore::CredentialPersistence persistence);
void APS5_VABI _ZN7WebCore14CredentialBaseC2Ev(WebCore::CredentialBase* self);
void APS5_VABI _ZN7WebCore18ParsedContentRangeC1ERKN3WTF6StringE(WebCore::ParsedContentRange* self, const NKWebKit::WtfAbi::String& headerValue);
void APS5_VABI _ZN7WebCore18ParsedContentRangeC1Elll(WebCore::ParsedContentRange* self, std::int64_t firstBytePosition, std::int64_t lastBytePosition, std::int64_t instanceLength);
void APS5_VABI _ZN7WebCore18ParsedContentRangeC2ERKN3WTF6StringE(WebCore::ParsedContentRange* self, const NKWebKit::WtfAbi::String& headerValue);
void APS5_VABI _ZN7WebCore18ParsedContentRangeC2Elll(WebCore::ParsedContentRange* self, std::int64_t firstBytePosition, std::int64_t lastBytePosition, std::int64_t instanceLength);
bool APS5_VABI _ZN7WebCore18isValidContentTypeERKN3WTF6StringENS_4ModeE(const NKWebKit::WtfAbi::String& contentType, WebCore::Mode mode);
bool APS5_VABI _ZNK7WebCore19ProtectionSpaceBase15isPasswordBasedEv(const WebCore::ProtectionSpaceBase* self);
bool APS5_VABI _ZNK7WebCore19ProtectionSpaceBase26receivesCredentialSecurelyEv(const WebCore::ProtectionSpaceBase* self);
bool APS5_VABI _ZNK7WebCore19ProtectionSpaceBase7isProxyEv(const WebCore::ProtectionSpaceBase* self);
bool APS5_VABI _ZNK7WebCore14CredentialBase11hasPasswordEv(const WebCore::CredentialBase* self);
WebCore::CredentialPersistence APS5_VABI _ZNK7WebCore14CredentialBase11persistenceEv(const WebCore::CredentialBase* self);
const NKWebKit::WtfAbi::String& APS5_VABI _ZNK7WebCore14CredentialBase4userEv(const WebCore::CredentialBase* self);
bool APS5_VABI _ZNK7WebCore14CredentialBase7isEmptyEv(const WebCore::CredentialBase* self);
const NKWebKit::WtfAbi::String& APS5_VABI _ZNK7WebCore14CredentialBase8passwordEv(const WebCore::CredentialBase* self);
NKWebKit::WtfAbi::String APS5_VABI _ZNK7WebCore11ContentType13containerTypeEv(const WebCore::ContentType* self);
NKWebKit::WtfAbi::String APS5_VABI _ZNK7WebCore11ContentType9parameterERKN3WTF6StringE(const WebCore::ContentType* self, const NKWebKit::WtfAbi::String& parameterName);
NKWebKit::WtfAbi::String APS5_VABI _ZNK7WebCore18ParsedContentRange11headerValueEv(const WebCore::ParsedContentRange* self);

}
