#include "wsaapi.h"
#include "wsseapi.h"
#include "soapH.h"
#include "soapStub.h"
#include "onvif_debug.h"
#include "onvif_server_interface.h"
#include "onvif_device_interface.h"
#include "onvif_event_manager.h"
// #include "base64.h"
// #include "sha1.h"


static int onvif_fault(struct soap* soap, char* value1, char* value2)
{
    soap->fault = (struct SOAP_ENV__Fault*)soap_malloc(soap,
                  (sizeof(struct SOAP_ENV__Fault)));
    if(soap->fault == NULL)
    {
        printf("Failed to malloc for fault.\n");
        return SOAP_FAULT;
    }

    soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc(soap,
                                  (sizeof(struct SOAP_ENV__Code)));
    if(soap->fault->SOAP_ENV__Code == NULL)
    {
        printf("Failed to malloc for SOAP_ENV__Code.\n");
        return SOAP_FAULT;
    }

    soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = "SOAP-ENV:Sender";
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)
            soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));;
    if(soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode == NULL)
    {
        printf("Failed to malloc for SOAP_ENV__Subcode.\n");
        return SOAP_FAULT;
    }

    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = value1;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode =
        (struct SOAP_ENV__Code*)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));

    if(soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode == NULL)
    {
        printf("Failed to malloc for SOAP_ENV__Code.\n");
        return SOAP_FAULT;
    }

    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Value
        = value2;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Subcode
        = NULL;
    soap->fault->faultcode = NULL;
    soap->fault->faultstring = NULL;
    soap->fault->faultactor = NULL;
    soap->fault->detail = NULL;
    soap->fault->SOAP_ENV__Reason = NULL;
    soap->fault->SOAP_ENV__Node = NULL;
    soap->fault->SOAP_ENV__Role = NULL;
    soap->fault->SOAP_ENV__Detail = NULL;
    return SOAP_OK;
}

int onvif_clear_wsse_head(struct soap* soap)
{
    const char* username = soap_wsse_get_Username(soap);
    if(username == NULL) {
        printf("username is NULL\n");
        return SOAP_OK;
    }
    soap_wsse_delete_Security(soap);
    return SOAP_OK;
}

static int onvif_check_security(struct soap *soap)
{
    // check user and passwd
    printf("Authentication: start---\n");
    struct _wsse__Security* wsse__Security;
    struct _wsse__UsernameToken* UsernameToken;

    if(strcmp(gOnvifUsers.current->name, "admin") == 0) {
        printf("Authentication: default user\n");
        if(strcmp(gOnvifUsers.current->password, "cvitek1234") == 0) {
            printf("Authentication: default passwd\n");
            const char *tmp_username = soap_wsse_get_Username(soap);
            if(tmp_username == NULL) {
                return SOAP_OK;
            }
            soap_wsse_delete_Security(soap);
            return SOAP_OK;
        }
    }

    if(soap->header == NULL) {
        printf("Authentication: soap header is NULL\n");
        goto end;
    }
    if(soap->header->wsse__Security == NULL) {
        printf("Authentication: security is NULL\n");
        goto end;
    }

    wsse__Security = soap->header->wsse__Security;
    UsernameToken = wsse__Security->UsernameToken;

    char tmp_pw[128] = {0};
    int offset = 0;
    int tmp_len = 0;
#if 1
    if(UsernameToken != NULL) {
        if(UsernameToken->Username != NULL) {
            printf("Authentication: username: %s\n", UsernameToken->Username);
        } else {
            printf("Authentication: username is NULL\n");
        }

        if(UsernameToken->Password != NULL) {
            if(UsernameToken->Password->__item != NULL) {
                printf("Authentication: password item: %s\n", UsernameToken->Password->__item);
            } else {
                printf("Authentication: password item is NULL\n");
            }
            if(UsernameToken->Password->Type != NULL) {
                // printf("password type: %s\n", UsernameToken->Password->Type);
                if(strcmp(UsernameToken->Password->Type,
                    "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest") == 0) {
                        printf("Authentication: password type is: PasswordDigest\n");
                } else {
                    printf("Authentication: password type: %s\n", UsernameToken->Password->Type);
                }
            } else {
                printf("Authentication: password type is NULL\n");
            }
        } else {
            printf("Authentication: password is NULL\n");
        }

        if(UsernameToken->Nonce != NULL) {
            // char *__item;
            // char *EncodingType;
            // char *wsu__Id;
            if(UsernameToken->Nonce->__item != NULL) {
                printf("Authentication: username nonce item is: %s\n", UsernameToken->Nonce->__item);
                // tmp_len = base64_decode(UsernameToken->Nonce->__item, strlen(UsernameToken->Nonce->__item),
                //     tmp_pw);
                // log_dbg("tmp_len: %d item len: %d", tmp_len, strlen(UsernameToken->Nonce->__item));
            } else {
                printf("Authentication: username nonce item is NULL\n");
            }
            if(UsernameToken->Nonce->EncodingType != NULL) {
                if(strcmp(UsernameToken->Nonce->EncodingType,
                    "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary") == 0) {
                        printf("Authentication: username nonce encoding type is: Base64Binary\n");
                } else {
                    printf("Authentication: username nonce encoding type is: %s\n", UsernameToken->Nonce->EncodingType);
                }
            } else {
                printf("Authentication: username nonce encoding type is NULL\n");
            }
            if(UsernameToken->Nonce->wsu__Id != NULL) {
                printf("Authentication: username nonce wsu id is: %s\n", UsernameToken->Nonce->wsu__Id);
            } else {
                printf("Authentication: username nonce wsu id is NULL\n");
            }
        } else {
            printf("Authentication: nonce is NULL\n");
        }

        if(UsernameToken->Salt != NULL) {
            printf("Authentication: username salt is: %s\n", UsernameToken->Salt);
        } else {
            printf("Authentication: username salt is NULL\n");
        }

        if(UsernameToken->Iteration != NULL) {
            printf("Authentication: username Iteration is: %d\n", *UsernameToken->Iteration);
        } else {
            printf("Authentication: username Iteration is NULL\n");
        }

        if(UsernameToken->wsu__Created != NULL) {
            printf("Authentication: username wsu__Created is: %s\n", UsernameToken->wsu__Created);
        } else {
            printf("Authentication: username wsu__Created is NULL\n");
        }

        if(UsernameToken->wsu__Id != NULL) {
            printf("Authentication: username wsu__Id is: %s\n", UsernameToken->wsu__Id);
        } else {
            printf("Authentication: username wsu__Id is NULL\n");
        }
    }
#endif
    // check username and password
    const char *username = soap_wsse_get_Username(soap);
    const char *password;

    // if(username == NULL) {
    //     log_err("username is NULL");
    //     return 401;
    // }

    // if(!username) {
    //     soap_wsse_delete_Security(soap);
    //     log_err("!username");
    //     return soap->error;
    // }

    if(username == NULL) {
        // #define SOAP_UNAUTH_ERROR 401
        log_war("Authentication: username is NULL");
        soap_wsse_delete_Security(soap);
        // return SOAP_UNAUTH_ERROR;
        goto error;
    } else {
        printf("Authentication: username is: %s\n", username);
    }

    if(soap_wsse_verify_Password(soap, gOnvifUsers.current->password)) {
        printf("Authentication: failed\n");
        soap_wsse_delete_Security(soap);
        return soap->error;
    } else {
        printf("Authentication: OK\n");
    }
    soap_wsse_delete_Security(soap);
end:
    printf("Authentication: end---\n");
    return SOAP_OK;
    // end of check
error:
    printf("Authentication: error---\n");
    return SOAP_ERR;
}

SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode,
                                          char *faultstring, char *faultactor,
                                          struct SOAP_ENV__Detail *detail,
                                          struct SOAP_ENV__Code *_SOAP_ENV__Code,
                                          struct SOAP_ENV__Reason *SOAP_ENV__Reason,
                                          char *SOAP_ENV__Node, char *SOAP_ENV__Role,
                                          struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *soap, struct wsdd__HelloType *wsdd__Hello)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *soap, struct wsdd__ByeType *wsdd__Bye)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *soap,
                                        struct wsdd__ProbeType *wsdd__Probe)
{
    LOG_FUNC_IN();
    // WSSE_SECURITY_DELETE;
    log_com("probe type %s", wsdd__Probe->Types);

    struct wsdd__ScopesType *pScopes = NULL;
    char str_tmp[ONVIF_IP_ADDR_LENGTH] = {0};
    char scopes_message[] =
        "onvif://www.onvif.org/type/NetworkVideoTransmitter\r\n"
        "onvif://www.onvif.org/Profile/Streaming\r\n"
        "onvif://www.onvif.org/Profile/S\r\n"
        "onvif://www.onvif.org/name/CViTek\r\n"
        "onvif://www.onvif.org/hardware/1835_EVB\r\n"
        "onvif://www.onvif.org/location/city/ShenZhen\r\n"
        "onvif://www.onvif.org/location/country/China\r\n";

    // response ProbeMatches
    struct wsdd__ProbeMatchesType wsdd__ProbeMatches = {0};
    struct wsdd__ProbeMatchType *pProbeMatchType = NULL;
    struct wsa__Relationship *pWsa__RelatesTo = NULL;

    ALLOC_STRUCT(pProbeMatchType, struct wsdd__ProbeMatchType);
    soap_default_wsdd__ProbeMatchType(soap, pProbeMatchType);

    sprintf(str_tmp, "http://%s:%d/onvif/device_service", gOnvifDevInfo.ip, ONVIF_TCP_PORT);
    pProbeMatchType->XAddrs = soap_strdup(soap, str_tmp);

    if (wsdd__Probe->Types && strlen(wsdd__Probe->Types))
    {
        pProbeMatchType->Types = soap_strdup(soap, wsdd__Probe->Types);
    }
    else
    {
        pProbeMatchType->Types = soap_strdup(soap, "dn:NetworkVideoTransmitter tds:Device");
    }
    pProbeMatchType->MetadataVersion = 1;

    // Build Scopes Message
    ALLOC_STRUCT(pScopes, struct wsdd__ScopesType);
    soap_default_wsdd__ScopesType(soap, pScopes);
    pScopes->MatchBy = NULL;
    pScopes->__item = soap_strdup(soap, scopes_message);
    pProbeMatchType->Scopes = pScopes;

    pProbeMatchType->wsa__EndpointReference.Address = soap_strdup(soap, gOnvifDevInfo.uuid);

    wsdd__ProbeMatches.__sizeProbeMatch = 1;
    wsdd__ProbeMatches.ProbeMatch = pProbeMatchType;
    // Build SOAP Header
    ALLOC_STRUCT(pWsa__RelatesTo, struct wsa__Relationship);
    soap_default__wsa__RelatesTo(soap, pWsa__RelatesTo);
    pWsa__RelatesTo->__item = soap->header->wsa__MessageID;
    soap->header->wsa__RelatesTo = pWsa__RelatesTo;
    soap->header->wsa__Action =
        soap_strdup(soap, "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches");
    soap->header->wsa__To =
        soap_strdup(soap, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");

    soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &wsdd__ProbeMatches);

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *soap,
                                               struct wsdd__ProbeMatchesType
                                                *wsdd__ProbeMatches)
{
    LOG_FUNC_IN();
    WSSE_SECURITY_DELETE;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *soap,
    struct wsdd__ResolveType *wsdd__Resolve)
{
    LOG_FUNC_IN();
    WSSE_SECURITY_DELETE;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *soap,
                                                 struct wsdd__ResolveMatchesType
                                                 *wsdd__ResolveMatches)
{
    LOG_FUNC_IN();
    WSSE_SECURITY_DELETE;
    LOG_FUNC_OUT();
    return 0;
}

// TODO:
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault_alex(struct soap *soap, char *faultcode,
                                               char *faultstring, char *faultactor,
                                               struct SOAP_ENV__Detail *detail,
                                               struct SOAP_ENV__Code *_SOAP_ENV__Code,
                                               struct SOAP_ENV__Reason *SOAP_ENV__Reason,
                                               char *SOAP_ENV__Node, char *SOAP_ENV__Role,
                                               struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetServiceCapabilities(struct soap *soap,
                                                         struct _ns10__GetServiceCapabilities
                                                             *ns10__GetServiceCapabilities,
                                                         struct _ns10__GetServiceCapabilitiesResponse
                                                             *ns10__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__CreateProfile(struct soap *soap,
                                                struct _ns10__CreateProfile
                                                    *ns10__CreateProfile,
                                                struct _ns10__CreateProfileResponse
                                                    *ns10__CreateProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetProfiles(struct soap *soap,
                                              struct _ns10__GetProfiles *ns10__GetProfiles,
                                              struct _ns10__GetProfilesResponse
                                                *ns10__GetProfilesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__AddConfiguration(struct soap *soap,
                                                   struct _ns10__AddConfiguration
                                                        *ns10__AddConfiguration,
                                                   struct _ns10__AddConfigurationResponse
                                                        *ns10__AddConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__RemoveConfiguration(struct soap *soap,
                                                      struct _ns10__RemoveConfiguration
                                                          *ns10__RemoveConfiguration,
                                                      struct _ns10__RemoveConfigurationResponse
                                                          *ns10__RemoveConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__DeleteProfile(struct soap *soap,
                                                struct _ns10__DeleteProfile
                                                    *ns10__DeleteProfile,
                                                struct _ns10__DeleteProfileResponse
                                                    *ns10__DeleteProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetVideoSourceConfigurations(struct soap *soap,
                                                               struct ns10__GetConfiguration
                                                                   *ns10__GetVideoSourceConfigurations,
                                                               struct _ns10__GetVideoSourceConfigurationsResponse
                                                                   *ns10__GetVideoSourceConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetVideoEncoderConfigurations(struct soap *soap,
                                                                struct ns10__GetConfiguration
                                                                    *ns10__GetVideoEncoderConfigurations,
                                                                struct _ns10__GetVideoEncoderConfigurationsResponse
                                                                    *ns10__GetVideoEncoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioSourceConfigurations(struct soap *soap,
                                                               struct ns10__GetConfiguration
                                                                   *ns10__GetAudioSourceConfigurations,
                                                               struct _ns10__GetAudioSourceConfigurationsResponse
                                                                   *ns10__GetAudioSourceConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioEncoderConfigurations(struct soap *soap,
                                                                struct ns10__GetConfiguration
                                                                    *_ns10__GetAudioEncoderConfigurations,
                                                                struct _ns10__GetAudioEncoderConfigurationsResponse
                                                                    *_ns10__GetAudioEncoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAnalyticsConfigurations(struct soap *soap,
                                                             struct ns10__GetConfiguration
                                                                 *ns10__GetAnalyticsConfigurations,
                                                             struct _ns10__GetAnalyticsConfigurationsResponse
                                                                 *ns10_GetAnalyticsConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetMetadataConfigurations(struct soap *soap,
                                                            struct ns10__GetConfiguration
                                                                *ns10__GetMetadataConfigurations,
                                                            struct _ns10__GetMetadataConfigurationsResponse
                                                                *ns10__GetMetadataConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioOutputConfigurations(struct soap *soap,
                                                               struct ns10__GetConfiguration
                                                                   *ns10__GetAudioOutputConfigurations,
                                                               struct _ns10__GetAudioOutputConfigurationsResponse
                                                                   *ns10__GetAudioOutputConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioDecoderConfigurations(struct soap *soap,
                                                                struct ns10__GetConfiguration
                                                                    *ns10__GetAudioDecoderConfigurations,
                                                                struct _ns10__GetAudioDecoderConfigurationsResponse
                                                                    *ns10__GetAudioDecoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetVideoSourceConfiguration(struct soap *soap,
                                                              struct _ns10__SetVideoSourceConfiguration
                                                                  *ns10__SetVideoSourceConfiguration,
                                                              struct ns10__SetConfigurationResponse
                                                                  *ns10__SetVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetVideoEncoderConfiguration(struct soap *soap,
                                                               struct _ns10__SetVideoEncoderConfiguration
                                                                   *ns10__SetVideoEncoderConfiguration,
                                                               struct ns10__SetConfigurationResponse
                                                                   *ns10__SetVideoEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetAudioSourceConfiguration(struct soap *soap,
                                                              struct _ns10__SetAudioSourceConfiguration
                                                                  *ns10__SetAudioSourceConfiguration,
                                                              struct ns10__SetConfigurationResponse
                                                                  *ns10__SetAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetAudioEncoderConfiguration(struct soap *soap,
                                                               struct _ns10__SetAudioEncoderConfiguration
                                                                   *ns10__SetAudioEncoderConfiguration,
                                                               struct ns10__SetConfigurationResponse
                                                                   *ns10__SetAudioEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetMetadataConfiguration(struct soap *soap,
                                                           struct _ns10__SetMetadataConfiguration
                                                               *ns10__SetMetadataConfiguration,
                                                           struct ns10__SetConfigurationResponse
                                                               *ns10__SetMetadataConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetAudioOutputConfiguration(struct soap *soap,
                                                              struct _ns10__SetAudioOutputConfiguration
                                                                  *ns10__SetAudioOutputConfiguration,
                                                              struct ns10__SetConfigurationResponse
                                                                  *ns10__SetAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetAudioDecoderConfiguration(struct soap *soap,
                                                               struct _ns10__SetAudioDecoderConfiguration
                                                                   *ns10__SetAudioDecoderConfiguration,
                                                               struct ns10__SetConfigurationResponse
                                                                   *ns10__SetAudioDecoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetVideoSourceConfigurationOptions(struct soap *soap,
                                                                     struct ns10__GetConfiguration
                                                                         *ns10__GetVideoSourceConfigurationOptions,
                                                                     struct _ns10__GetVideoSourceConfigurationOptionsResponse
                                                                         *ns10__GetVideoSourceConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetVideoEncoderConfigurationOptions(struct soap *soap,
                                                                      struct ns10__GetConfiguration
                                                                          *ns10__GetVideoEncoderConfigurationOptions,
                                                                      struct _ns10__GetVideoEncoderConfigurationOptionsResponse
                                                                          *ns10__GetVideoEncoderConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioSourceConfigurationOptions(struct soap *soap,
                                                                     struct ns10__GetConfiguration
                                                                         *ns10__GetAudioSourceConfigurationOptions,
                                                                     struct _ns10__GetAudioSourceConfigurationOptionsResponse
                                                                         *ns10__GetAudioSourceConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioEncoderConfigurationOptions(struct soap *soap,
                                                                      struct ns10__GetConfiguration
                                                                          *ns10__GetAudioEncoderConfigurationOptions,
                                                                      struct _ns10__GetAudioEncoderConfigurationOptionsResponse
                                                                          *ns10__GetAudioEncoderConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetMetadataConfigurationOptions(struct soap *soap,
                                                                  struct ns10__GetConfiguration
                                                                      *ns10__GetMetadataConfigurationOptions,
                                                                  struct _ns10__GetMetadataConfigurationOptionsResponse
                                                                      *ns10__GetMetadataConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioOutputConfigurationOptions(struct soap *soap,
                                                                     struct ns10__GetConfiguration
                                                                         *ns10__GetAudioOutputConfigurationOptions,
                                                                     struct _ns10__GetAudioOutputConfigurationOptionsResponse
                                                                         *ns10__GetAudioOutputConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetAudioDecoderConfigurationOptions(struct soap *soap,
                                                                      struct ns10__GetConfiguration
                                                                          *ns10__GetAudioDecoderConfigurationOptions,
                                                                      struct _ns10__GetAudioDecoderConfigurationOptionsResponse
                                                                          *ns10__GetAudioDecoderConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetVideoEncoderInstances(struct soap *soap,
                                                           struct _ns10__GetVideoEncoderInstances
                                                               *ns10__GetVideoEncoderInstances,
                                                           struct _ns10__GetVideoEncoderInstancesResponse
                                                               *ns10__GetVideoEncoderInstancesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetStreamUri(struct soap *soap,
                                               struct _ns10__GetStreamUri *ns10__GetStreamUri,
                                               struct _ns10__GetStreamUriResponse *ns10__GetStreamUriResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__StartMulticastStreaming(struct soap *soap,
                                                          struct ns10__StartStopMulticastStreaming
                                                              *ns10__StartMulticastStreaming,
                                                          struct ns10__SetConfigurationResponse
                                                              *ns10__StartMulticastStreamingResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__StopMulticastStreaming(struct soap *soap,
                                                         struct ns10__StartStopMulticastStreaming
                                                             *ns10__StopMulticastStreaming,
                                                         struct ns10__SetConfigurationResponse
                                                             *ns10__StopMulticastStreamingResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetSynchronizationPoint(struct soap *soap,
                                                          struct _ns10__SetSynchronizationPoint
                                                              *ns10__SetSynchronizationPoint,
                                                          struct _ns10__SetSynchronizationPointResponse
                                                              *ns10__SetSynchronizationPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetSnapshotUri(struct soap *soap,
                                                 struct _ns10__GetSnapshotUri *ns10__GetSnapshotUri,
                                                 struct _ns10__GetSnapshotUriResponse *ns10__GetSnapshotUriResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetVideoSourceModes(struct soap *soap,
                                                      struct _ns10__GetVideoSourceModes
                                                          *ns10__GetVideoSourceModes,
                                                      struct _ns10__GetVideoSourceModesResponse
                                                          *ns10__GetVideoSourceModesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetVideoSourceMode(struct soap *soap,
                                                     struct _ns10__SetVideoSourceMode
                                                         *ns10__SetVideoSourceMode,
                                                     struct _ns10__SetVideoSourceModeResponse
                                                         *ns10__SetVideoSourceModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetOSDs(struct soap *soap,
                                          struct _ns10__GetOSDs *ns10__GetOSDs,
                                          struct _ns10__GetOSDsResponse *ns10__GetOSDsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetOSDOptions(struct soap *soap,
                                                struct _ns10__GetOSDOptions *ns10__GetOSDOptions,
                                                struct _ns10__GetOSDOptionsResponse *ns10__GetOSDOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetOSD(struct soap *soap,
                                         struct _ns10__SetOSD *ns10__SetOSD,
                                         struct ns10__SetConfigurationResponse *ns10__SetOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__CreateOSD(struct soap *soap,
                                            struct _ns10__CreateOSD *ns10__CreateOSD,
                                            struct _ns10__CreateOSDResponse *ns10__CreateOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__DeleteOSD(struct soap *soap,
                                            struct _ns10__DeleteOSD *ns10__DeleteOSD,
                                            struct ns10__SetConfigurationResponse *ns10__DeleteOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetMasks(struct soap *soap,
                                           struct _ns10__GetMasks *ns10__GetMasks,
                                           struct _ns10__GetMasksResponse *ns10__GetMasksResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__GetMaskOptions(struct soap *soap,
                                                 struct _ns10__GetMaskOptions *ns10__GetMaskOptions,
                                                 struct _ns10__GetMaskOptionsResponse *ns10__GetMaskOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__SetMask(struct soap *soap,
                                          struct _ns10__SetMask *ns10__SetMask,
                                          struct ns10__SetConfigurationResponse *ns10__SetMaskResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__CreateMask(struct soap *soap,
                                             struct _ns10__CreateMask *ns10__CreateMask,
                                             struct _ns10__CreateMaskResponse *ns10__CreateMaskResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns10__DeleteMask(struct soap *soap,
                                             struct _ns10__DeleteMask *ns10__DeleteMask,
                                             struct ns10__SetConfigurationResponse *ns10__DeleteMaskResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__GetServiceCapabilities(struct soap *soap,
                                                         struct _ns11__GetServiceCapabilities
                                                             *ns11__GetServiceCapabilities,
                                                         struct _ns11__GetServiceCapabilitiesResponse
                                                             *ns11__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__PanMove(struct soap *soap,
                                          struct _ns11__PanMove *ns11__PanMove,
                                          struct _ns11__PanMoveResponse *ns11__PanMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__TiltMove(struct soap *soap,
                                           struct _ns11__TiltMove *ns11__TiltMove,
                                           struct _ns11__TiltMoveResponse *ns11__TiltMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__ZoomMove(struct soap *soap,
                                           struct _ns11__ZoomMove *ns11__ZoomMove,
                                           struct _ns11__ZoomMoveResponse *ns11__ZoomMoveResp)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__RollMove(struct soap *soap,
                                           struct _ns11__RollMove *ns11__RollMove,
                                           struct _ns11__RollMoveResponse *ns11__RollMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__FocusMove(struct soap *soap,
                                            struct _ns11__FocusMove *ns11__FocusMove,
                                            struct _ns11__FocusMoveResponse *ns11__FocusMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__Stop(struct soap *soap,
                                       struct _ns11__Stop *ns11__Stop,
                                       struct _ns11__StopResponse *ns11__StopResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns11__GetUsage(struct soap *soap,
                                           struct _ns11__GetUsage *ns11__GetUsage,
                                           struct _ns11__GetUsageResponse *ns11__GetUsageResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetServiceCapabilities(struct soap *soap,
                                                         struct _ns12__GetServiceCapabilities
                                                             *ns12__GetServiceCapabilities,
                                                         struct _ns12__GetServiceCapabilitiesResponse
                                                             *ns12__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetScheduleState(struct soap *soap,
                                                   struct _ns12__GetScheduleState
                                                       *ns12__GetScheduleState,
                                                   struct _ns12__GetScheduleStateResponse
                                                       *ns12__GetScheduleStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetScheduleInfo(struct soap *soap,
                                                  struct _ns12__GetScheduleInfo
                                                      *ns12__GetScheduleInfo,
                                                  struct _ns12__GetScheduleInfoResponse
                                                      *ns12__GetScheduleInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetScheduleInfoList(struct soap *soap,
                                                      struct _ns12__GetScheduleInfoList
                                                          *ns12__GetScheduleInfoList,
                                                      struct _ns12__GetScheduleInfoListResponse
                                                          *ns12__GetScheduleInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetSchedules(struct soap *soap,
                                               struct _ns12__GetSchedules
                                                   *ns12__GetSchedules,
                                               struct _ns12__GetSchedulesResponse
                                                   *ns12__GetSchedulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetScheduleList(struct soap *soap,
                                                  struct _ns12__GetScheduleList
                                                      *ns12__GetScheduleList,
                                                  struct _ns12__GetScheduleListResponse
                                                      *ns12__GetScheduleListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__CreateSchedule(struct soap *soap,
                                                 struct _ns12__CreateSchedule
                                                     *_ns12__CreateSchedule,
                                                 struct _ns12__CreateScheduleResponse
                                                     *ns12__CreateScheduleResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__SetSchedule(struct soap *soap,
                                              struct _ns12__SetSchedule *ns12__SetSchedule,
                                              struct _ns12__SetScheduleResponse *ns12__SetScheduleResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__ModifySchedule(struct soap *soap,
                                                 struct _ns12__ModifySchedule *ns12__ModifySchedule,
                                                 struct _ns12__ModifyScheduleResponse *ns12__ModifyScheduleResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__DeleteSchedule(struct soap *soap,
                                                 struct _ns12__DeleteSchedule
                                                     *ns12__DeleteSchedule,
                                                 struct _ns12__DeleteScheduleResponse
                                                     *ns12__DeleteScheduleResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetSpecialDayGroupInfo(struct soap *soap,
                                                         struct _ns12__GetSpecialDayGroupInfo
                                                             *ns12__GetSpecialDayGroupInfo,
                                                         struct _ns12__GetSpecialDayGroupInfoResponse
                                                             *ns12__GetSpecialDayGroupInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetSpecialDayGroupInfoList(struct soap *soap,
                                                             struct _ns12__GetSpecialDayGroupInfoList
                                                                 *ns12__GetSpecialDayGroupInfoList,
                                                             struct _ns12__GetSpecialDayGroupInfoListResponse
                                                                 *ns12__GetSpecialDayGroupInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetSpecialDayGroups(struct soap *soap,
                                                      struct _ns12__GetSpecialDayGroups
                                                          *ns12__GetSpecialDayGroups,
                                                      struct _ns12__GetSpecialDayGroupsResponse
                                                          *ns12__GetSpecialDayGroupsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__GetSpecialDayGroupList(struct soap *soap,
                                                         struct _ns12__GetSpecialDayGroupList
                                                             *ns12__GetSpecialDayGroupList,
                                                         struct _ns12__GetSpecialDayGroupListResponse
                                                             *ns12__GetSpecialDayGroupListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__CreateSpecialDayGroup(struct soap *soap,
                                                        struct _ns12__CreateSpecialDayGroup
                                                            *ns12__CreateSpecialDayGroup,
                                                        struct _ns12__CreateSpecialDayGroupResponse
                                                            *ns12__CreateSpecialDayGroupResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__SetSpecialDayGroup(struct soap *soap,
                                                     struct _ns12__SetSpecialDayGroup
                                                         *ns12__SetSpecialDayGroup,
                                                     struct _ns12__SetSpecialDayGroupResponse
                                                         *ns12__SetSpecialDayGroupResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__ModifySpecialDayGroup(struct soap *soap,
                                                        struct _ns12__ModifySpecialDayGroup
                                                            *ns12__ModifySpecialDayGroup,
                                                        struct _ns12__ModifySpecialDayGroupResponse
                                                            *ns12__ModifySpecialDayGroupResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns12__DeleteSpecialDayGroup(struct soap *soap,
                                                        struct _ns12__DeleteSpecialDayGroup
                                                            *ns12__DeleteSpecialDayGroup,
                                                        struct _ns12__DeleteSpecialDayGroupResponse
                                                            *ns12__DeleteSpecialDayGroupResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__GetServiceCapabilities(struct soap *soap,
                                                         struct _ns13__GetServiceCapabilities
                                                             *ns13__GetServiceCapabilities,
                                                         struct _ns13__GetServiceCapabilitiesResponse
                                                             *ns13__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__GetConfigurationOptions(struct soap *soap,
                                                          struct _ns13__GetConfigurationOptions
                                                              *ns13__GetConfigurationOptions,
                                                          struct _ns13__GetConfigurationOptionsResponse
                                                              *ns13__GetConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__GetConfiguration(struct soap *soap,
                                                   struct _ns13__GetConfiguration
                                                       *ns13__GetConfiguration,
                                                   struct _ns13__GetConfigurationResponse
                                                       *ns13__GetConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__GetConfigurations(struct soap *soap,
                                                    struct _ns13__GetConfigurations
                                                        *ns13__GetConfigurations,
                                                    struct _ns13__GetConfigurationsResponse
                                                        *ns13__GetConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__SetConfiguration(struct soap *soap,
                                                   struct _ns13__SetConfiguration *ns13__SetConfiguration,
                                                   struct _ns13__SetConfigurationResponse *ns13__SetConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__GetRadiometryConfigurationOptions(struct soap *soap,
                                                                    struct _ns13__GetRadiometryConfigurationOptions
                                                                        *ns13__GetRadiometryConfigurationOptions,
                                                                    struct _ns13__GetRadiometryConfigurationOptionsResponse
                                                                        *ns13__GetRadiometryConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__GetRadiometryConfiguration(struct soap *soap,
                                                             struct _ns13__GetRadiometryConfiguration
                                                                 *ns13__GetRadiometryConfiguration,
                                                             struct _ns13__GetRadiometryConfigurationResponse
                                                                 *ns13__GetRadiometryConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns13__SetRadiometryConfiguration(struct soap *soap,
                                                             struct _ns13__SetRadiometryConfiguration
                                                                 *ns13__SetRadiometryConfiguration,
                                                             struct _ns13__SetRadiometryConfigurationResponse
                                                                 *ns13__SetRadiometryConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns14__GetServiceCapabilities(struct soap *soap,
                                                         struct _ns14__GetServiceCapabilities
                                                             *ns14__GetServiceCapabilities,
                                                         struct _ns14__GetServiceCapabilitiesResponse
                                                             *ns14__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns14__GetUplinks(struct soap *soap,
                                             struct _ns14__GetUplinks *ns14__GetUplinks,
                                             struct _ns14__GetUplinksResponse *ns14__GetUplinksResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns14__SetUplink(struct soap *soap,
                                            struct _ns14__SetUplink *ns14__SetUplink,
                                            struct _ns14__SetUplinkResponse *ns14__GetUplinksResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns14__DeleteUplink(struct soap *soap,
                                               struct _ns14__DeleteUplink *ns14__DeleteUplink,
                                               struct _ns14__DeleteUplinkResponse *ns14__DeleteUplinkResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__GetSupportedActions(struct soap *soap,
                                                     struct _ns3__GetSupportedActions
                                                         *ns3__GetSupportedActions,
                                                     struct _ns3__GetSupportedActionsResponse
                                                         *ns3__GetSupportedActionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__GetActions(struct soap *soap,
                                            struct _ns3__GetActions *ns3__GetActions,
                                            struct _ns3__GetActionsResponse *ns3__GetActionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__CreateActions(struct soap *soap,
                                               struct _ns3__CreateActions *ns3__CreateActions,
                                               struct _ns3__CreateActionsResponse *ns3__CreateActionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__DeleteActions(struct soap *soap,
                                               struct _ns3__DeleteActions *ns3__DeleteActions,
                                               struct _ns3__DeleteActionsResponse *ns3__DeleteActionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__ModifyActions(struct soap *soap,
                                               struct _ns3__ModifyActions *ns3__ModifyActions,
                                               struct _ns3__ModifyActionsResponse *ns3__ModifyActionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__GetServiceCapabilities(struct soap *soap,
                                                        struct _ns3__GetServiceCapabilities
                                                            *ns3__GetServiceCapabilities,
                                                        struct _ns3__GetServiceCapabilitiesResponse
                                                            *ns3__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__GetActionTriggers(struct soap *soap,
                                                   struct _ns3__GetActionTriggers *ns3__GetActionTriggers,
                                                   struct _ns3__GetActionTriggersResponse *ns3__GetActionTriggersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__CreateActionTriggers(struct soap *soap,
                                                      struct _ns3__CreateActionTriggers
                                                          *ns3__CreateActionTriggers,
                                                      struct _ns3__CreateActionTriggersResponse
                                                          *ns3__CreateActionTriggersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__DeleteActionTriggers(struct soap *soap,
                                                      struct _ns3__DeleteActionTriggers
                                                          *ns3__DeleteActionTriggers,
                                                      struct _ns3__DeleteActionTriggersResponse
                                                          *ns3__DeleteActionTriggersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns3__ModifyActionTriggers(struct soap *soap,
                                                      struct _ns3__ModifyActionTriggers
                                                          *ns3__ModifyActionTriggers,
                                                      struct _ns3__ModifyActionTriggersResponse
                                                          *ns3__ModifyActionTriggersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetServiceCapabilities(struct soap *soap,
                                                        struct _ns4__GetServiceCapabilities
                                                            *ns4__GetServiceCapabilities,
                                                        struct _ns4__GetServiceCapabilitiesResponse
                                                            *ns4__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAccessPointInfoList(struct soap *soap,
                                                        struct _ns4__GetAccessPointInfoList
                                                            *ns4__GetAccessPointInfoList,
                                                        struct _ns4__GetAccessPointInfoListResponse
                                                            *ns4__GetAccessPointInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAccessPointInfo(struct soap *soap,
                                                    struct _ns4__GetAccessPointInfo
                                                        *ns4__GetAccessPointInfo,
                                                    struct _ns4__GetAccessPointInfoResponse
                                                        *ns4__GetAccessPointInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAccessPointList(struct soap *soap,
                                                    struct _ns4__GetAccessPointList
                                                        *ns4__GetAccessPointList,
                                                    struct _ns4__GetAccessPointListResponse
                                                        *ns4__GetAccessPointListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAccessPoints(struct soap *soap,
                                                 struct _ns4__GetAccessPoints *ns4__GetAccessPoints,
                                                 struct _ns4__GetAccessPointsResponse *ns4__GetAccessPointsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__CreateAccessPoint(struct soap *soap,
                                                   struct _ns4__CreateAccessPoint
                                                       *ns4__CreateAccessPoint,
                                                   struct _ns4__CreateAccessPointResponse
                                                       *ns4__CreateAccessPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__SetAccessPoint(struct soap *soap,
                                                struct _ns4__SetAccessPoint *ns4__SetAccessPoint,
                                                struct _ns4__SetAccessPointResponse *ns4__SetAccessPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__ModifyAccessPoint(struct soap *soap,
                                                   struct _ns4__ModifyAccessPoint
                                                       *ns4__ModifyAccessPoint,
                                                   struct _ns4__ModifyAccessPointResponse
                                                       *ns4__ModifyAccessPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__DeleteAccessPoint(struct soap *soap,
                                                   struct _ns4__DeleteAccessPoint
                                                       *ns4__DeleteAccessPoint,
                                                   struct _ns4__DeleteAccessPointResponse
                                                       *ns4__DeleteAccessPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__SetAccessPointAuthenticationProfile(struct soap *soap,
                                                                     struct _ns4__SetAccessPointAuthenticationProfile
                                                                         *ns4__SetAccessPointAuthenticationProfile,
                                                                     struct _ns4__SetAccessPointAuthenticationProfileResponse
                                                                         *ns4__SetAccessPointAuthenticationProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__DeleteAccessPointAuthenticationProfile(struct soap *soap,
                                                                        struct _ns4__DeleteAccessPointAuthenticationProfile
                                                                            *ns4__DeleteAccessPointAuthenticationProfile,
                                                                        struct _ns4__DeleteAccessPointAuthenticationProfileResponse
                                                                            *ns4__DeleteAccessPointAuthenticationProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAreaInfoList(struct soap *soap,
                                                 struct _ns4__GetAreaInfoList
                                                     *ns4__GetAreaInfoList,
                                                 struct _ns4__GetAreaInfoListResponse
                                                     *ns4__GetAreaInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAreaInfo(struct soap *soap,
                                             struct _ns4__GetAreaInfo
                                                 *ns4__GetAreaInfo,
                                             struct _ns4__GetAreaInfoResponse
                                                 *ns4_GetAreaInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAreaList(struct soap *soap,
                                             struct _ns4__GetAreaList
                                                 *ns4__GetAreaList,
                                             struct _ns4__GetAreaListResponse
                                                 *ns4__GetAreaListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAreas(struct soap *soap,
                                          struct _ns4__GetAreas
                                              *ns4__GetAreas,
                                          struct _ns4__GetAreasResponse
                                              *ns4__GetAreasResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__CreateArea(struct soap *soap,
                                            struct _ns4__CreateArea
                                                *ns4__CreateArea,
                                            struct _ns4__CreateAreaResponse
                                                *ns4__CreateAreaResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__SetArea(struct soap *soap,
                                         struct _ns4__SetArea *ns4__SetArea,
                                         struct _ns4__SetAreaResponse *ns4__SetAreaResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__ModifyArea(struct soap *soap,
                                            struct _ns4__ModifyArea *ns4__ModifyArea,
                                            struct _ns4__ModifyAreaResponse *ns4__ModifyAreaResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__DeleteArea(struct soap *soap,
                                            struct _ns4__DeleteArea *ns4__DeleteArea,
                                            struct _ns4__DeleteAreaResponse *ns4__DeleteAreaResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetAccessPointState(struct soap *soap,
                                                     struct _ns4__GetAccessPointState
                                                         *ns4__GetAccessPointState,
                                                     struct _ns4__GetAccessPointStateResponse
                                                         *ns4__GetAccessPointStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__EnableAccessPoint(struct soap *soap,
                                                   struct _ns4__EnableAccessPoint
                                                       *ns4__EnableAccessPoint,
                                                   struct _ns4__EnableAccessPointResponse
                                                       *ns4__EnableAccessPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__DisableAccessPoint(struct soap *soap,
                                                    struct _ns4__DisableAccessPoint
                                                        *ns4__DisableAccessPoint,
                                                    struct _ns4__DisableAccessPointResponse
                                                        *ns4__DisableAccessPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__ExternalAuthorization(struct soap *soap,
                                                       struct _ns4__ExternalAuthorization
                                                           *ns4__ExternalAuthorization,
                                                       struct _ns4__ExternalAuthorizationResponse
                                                           *ns4__ExternalAuthorizationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__Feedback(struct soap *soap,
                                          struct _ns4__Feedback
                                              *ns4__Feedback,
                                          struct _ns4__FeedbackResponse
                                              *ns4__FeedbackResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__Uninstall(struct soap *soap,
                                           struct _ns6__Uninstall *ns6__Uninstall,
                                           struct _ns6__UninstallResponse *ns6__UninstallResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__GetInstalledApps(struct soap *soap,
                                                  struct _ns6__GetInstalledApps *ns6__GetInstalledApps,
                                                  struct _ns6__GetInstalledAppsResponse *ns6__GetInstalledAppsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__GetAppsInfo(struct soap *soap,
                                             struct _ns6__GetAppsInfo *ns6__GetAppsInfo,
                                             struct _ns6__GetAppsInfoResponse *ns6__GetAppsInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__Activate(struct soap *soap,
                                          struct _ns6__Activate *ns6__Activate,
                                          struct _ns6__ActivateResponse *ns6__ActivateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__Deactivate(struct soap *soap,
                                            struct _ns6__Deactivate *ns6__Deactivate,
                                            struct _ns6__DeactivateResponse *ns6__DeactivateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__GetServiceCapabilities(struct soap *soap,
                                                        struct _ns6__GetServiceCapabilities
                                                            *ns6__GetServiceCapabilities,
                                                        struct _ns6__GetServiceCapabilitiesResponse
                                                            *ns6__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__InstallLicense(struct soap *soap,
                                                struct _ns6__InstallLicense *ns6__InstallLicense,
                                                struct _ns6__InstallLicenseResponse *ns6__InstallLicenseResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns6__GetDeviceId(struct soap *soap,
                                             struct _ns6__GetDeviceId *ns6__GetDeviceId,
                                             struct _ns6__GetDeviceIdReponse *ns6__GetDeviceIdResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetServiceCapabilities(struct soap *soap,
                                                        struct _ns7__GetServiceCapabilities
                                                            *ns7__GetServiceCapabilities,
                                                        struct _ns7__GetServiceCapabilitiesResponse
                                                            *ns7__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetAuthenticationProfileInfo(struct soap *soap,
                                                              struct _ns7__GetAuthenticationProfileInfo
                                                                  *ns7__GetAuthenticationProfileInfo,
                                                              struct _ns7__GetAuthenticationProfileInfoResponse
                                                                  *ns7__GetAuthenticationProfileInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetAuthenticationProfileInfoList(struct soap *soap,
                                                                  struct _ns7__GetAuthenticationProfileInfoList
                                                                      *ns7__GetAuthenticationProfileInfoList,
                                                                  struct _ns7__GetAuthenticationProfileInfoListResponse
                                                                      *ns7__GetAuthenticationProfileInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetAuthenticationProfiles(struct soap *soap,
                                                           struct _ns7__GetAuthenticationProfiles
                                                               *ns7__GetAuthenticationProfiles,
                                                           struct _ns7__GetAuthenticationProfilesResponse
                                                               *ns7__GetAuthenticationProfilesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetAuthenticationProfileList(struct soap *soap,
                                                              struct _ns7__GetAuthenticationProfileList
                                                                  *ns7__GetAuthenticationProfileList,
                                                              struct _ns7__GetAuthenticationProfileListResponse
                                                                  *ns7__GetAuthenticationProfileListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__CreateAuthenticationProfile(struct soap *soap,
                                                             struct _ns7__CreateAuthenticationProfile
                                                                 *ns7__CreateAuthenticationProfile,
                                                             struct _ns7__CreateAuthenticationProfileResponse
                                                                 *ns7__CreateAuthenticationProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__SetAuthenticationProfile(struct soap *soap,
                                                          struct _ns7__SetAuthenticationProfile
                                                              *ns7__SetAuthenticationProfile,
                                                          struct _ns7__SetAuthenticationProfileResponse
                                                              *ns7__SetAuthenticationProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__ModifyAuthenticationProfile(struct soap *soap,
                                                             struct _ns7__ModifyAuthenticationProfile
                                                                 *ns7__ModifyAuthenticationProfile,
                                                             struct _ns7__ModifyAuthenticationProfileResponse
                                                                 *ns7__ModifyAuthenticationProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__DeleteAuthenticationProfile(struct soap *soap,
                                                             struct _ns7__DeleteAuthenticationProfile
                                                                 *ns7__DeleteAuthenticationProfile,
                                                             struct _ns7__DeleteAuthenticationProfileResponse
                                                                 *ns7__DeleteAuthenticationProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetSecurityLevelInfo(struct soap *soap,
                                                      struct _ns7__GetSecurityLevelInfo
                                                          *ns7__GetSecurityLevelInfo,
                                                      struct _ns7__GetSecurityLevelInfoResponse
                                                          *ns7__GetSecurityLevelInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetSecurityLevelInfoList(struct soap *soap,
                                                          struct _ns7__GetSecurityLevelInfoList
                                                              *ns7__GetSecurityLevelInfoList,
                                                          struct _ns7__GetSecurityLevelInfoListResponse
                                                              *ns7__GetSecurityLevelInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetSecurityLevels(struct soap *soap,
                                                   struct _ns7__GetSecurityLevels
                                                       *ns7__GetSecurityLevels,
                                                   struct _ns7__GetSecurityLevelsResponse
                                                       *ns7__GetSecurityLevelsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__GetSecurityLevelList(struct soap *soap,
                                                      struct _ns7__GetSecurityLevelList
                                                          *ns7__GetSecurityLevelList,
                                                      struct _ns7__GetSecurityLevelListResponse
                                                          *ns7__GetSecurityLevelListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__CreateSecurityLevel(struct soap *soap,
                                                     struct _ns7__CreateSecurityLevel
                                                         *ns7__CreateSecurityLevel,
                                                     struct _ns7__CreateSecurityLevelResponse
                                                         *ns7__CreateSecurityLevelResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__SetSecurityLevel(struct soap *soap,
                                                  struct _ns7__SetSecurityLevel
                                                      *ns7__SetSecurityLevel,
                                                  struct _ns7__SetSecurityLevelResponse
                                                      *ns7__SetSecurityLevelResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__ModifySecurityLevel(struct soap *soap,
                                                     struct _ns7__ModifySecurityLevel
                                                         *ns7__ModifySecurityLevel,
                                                     struct _ns7__ModifySecurityLevelResponse
                                                         *ns7__ModifySecurityLevelResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns7__DeleteSecurityLevel(struct soap *soap,
                                                     struct _ns7__DeleteSecurityLevel
                                                         *ns7__DeleteSecurityLevel,
                                                     struct _ns7__DeleteSecurityLevelResponse
                                                         *ns7__DeleteSecurityLevelResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetServiceCapabilities(struct soap *soap,
                                                        struct _ns8__GetServiceCapabilities
                                                            *ns8__GetServiceCapabilities,
                                                        struct _ns8__GetServiceCapabilitiesResponse
                                                            *ns8__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetSupportedFormatTypes(struct soap *soap,
                                                         struct _ns8__GetSupportedFormatTypes
                                                             *ns8__GetSupportedFormatTypes,
                                                         struct _ns8__GetSupportedFormatTypesResponse
                                                             *ns8__GetSupportedFormatTypesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentialInfo(struct soap *soap,
                                                   struct _ns8__GetCredentialInfo
                                                       *ns8__GetCredentialInfo,
                                                   struct _ns8__GetCredentialInfoResponse
                                                       *ns8__GetCredentialInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentialInfoList(struct soap *soap,
                                                       struct _ns8__GetCredentialInfoList
                                                           *ns8__GetCredentialInfoList,
                                                       struct _ns8__GetCredentialInfoListResponse
                                                           *ns8__GetCredentialInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentials(struct soap *soap,
                                                struct _ns8__GetCredentials
                                                    *ns8__GetCredentials,
                                                struct _ns8__GetCredentialsResponse
                                                    *ns8__GetCredentialsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentialList(struct soap *soap,
                                                   struct _ns8__GetCredentialList
                                                       *ns8__GetCredentialList,
                                                   struct _ns8__GetCredentialListResponse
                                                       *ns8__GetCredentialListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__CreateCredential(struct soap *soap,
                                                  struct _ns8__CreateCredential
                                                      *ns8__CreateCredential,
                                                  struct _ns8__CreateCredentialResponse
                                                      *ns8__CreateCredentialResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__SetCredential(struct soap *soap,
                                               struct _ns8__SetCredential
                                                   *ns8__SetCredential,
                                               struct _ns8__SetCredentialResponse
                                                   *ns8__SetCredentialResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__ModifyCredential(struct soap *soap,
                                                  struct _ns8__ModifyCredential
                                                      *ns8__ModifyCredential,
                                                  struct _ns8__ModifyCredentialResponse
                                                      *ns8__ModifyCredentialResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__DeleteCredential(struct soap *soap,
                                                  struct _ns8__DeleteCredential
                                                      *ns8__DeleteCredential,
                                                  struct _ns8__DeleteCredentialResponse
                                                      *ns8__DeleteCredentialResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentialState(struct soap *soap,
                                                    struct _ns8__GetCredentialState
                                                        *ns8__GetCredentialState,
                                                    struct _ns8__GetCredentialStateResponse
                                                        *ns8__GetCredentialStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__EnableCredential(struct soap *soap,
                                                  struct _ns8__EnableCredential
                                                      *ns8__EnableCredential,
                                                  struct _ns8__EnableCredentialResponse
                                                      *ns8__EnableCredentialResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__DisableCredential(struct soap *soap,
                                                   struct _ns8__DisableCredential
                                                       *ns8__DisableCredential,
                                                   struct _ns8__DisableCredentialResponse
                                                       *ns8__DisableCredentialResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__ResetAntipassbackViolation(struct soap *soap,
                                                            struct _ns8__ResetAntipassbackViolation
                                                                *ns8__ResetAntipassbackViolation,
                                                            struct _ns8__ResetAntipassbackViolationResponse
                                                                *ns8__ResetAntipassbackViolationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentialIdentifiers(struct soap *soap,
                                                          struct _ns8__GetCredentialIdentifiers
                                                              *ns8__GetCredentialIdentifiers,
                                                          struct _ns8__GetCredentialIdentifiersResponse
                                                              *ns8__GetCredentialIdentifiersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__SetCredentialIdentifier(struct soap *soap,
                                                         struct _ns8__SetCredentialIdentifier
                                                             *ns8__SetCredentialIdentifier,
                                                         struct _ns8__SetCredentialIdentifierResponse
                                                             *ns8__SetCredentialIdentifierResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__DeleteCredentialIdentifier(struct soap *soap,
                                                            struct _ns8__DeleteCredentialIdentifier
                                                                *ns8__DeleteCredentialIdentifier,
                                                            struct _ns8__DeleteCredentialIdentifierResponse
                                                                *ns8__DeleteCredentialIdentifierResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetCredentialAccessProfiles(struct soap *soap,
                                                             struct _ns8__GetCredentialAccessProfiles
                                                                 *ns8__GetCredentialAccessProfiles,
                                                             struct _ns8__GetCredentialAccessProfilesResponse
                                                                 *ns8__GetCredentialAccessProfilesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__SetCredentialAccessProfiles(struct soap *soap,
                                                             struct _ns8__SetCredentialAccessProfiles
                                                                 *ns8__SetCredentialAccessProfiles,
                                                             struct _ns8__SetCredentialAccessProfilesResponse
                                                                 *ns8__SetCredentialAccessProfilesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__DeleteCredentialAccessProfiles(struct soap *soap,
                                                                struct _ns8__DeleteCredentialAccessProfiles
                                                                    *ns8__DeleteCredentialAccessProfiles,
                                                                struct _ns8__DeleteCredentialAccessProfilesResponse
                                                                    *ns8__DeleteCredentialAccessProfilesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetWhitelist(struct soap *soap,
                                              struct _ns8__GetWhitelist
                                                  *ns8__GetWhitelist,
                                              struct _ns8__GetWhitelistResponse
                                                  *ns8__GetWhitelistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__AddToWhitelist(struct soap *soap,
                                                struct _ns8__AddToWhitelist
                                                    *ns8__AddToWhitelist,
                                                struct _ns8__AddToWhitelistResponse
                                                    *ns8__AddToWhitelistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__RemoveFromWhitelist(struct soap *soap,
                                                     struct _ns8__RemoveFromWhitelist
                                                         *ns8__RemoveFromWhitelist,
                                                     struct _ns8__RemoveFromWhitelistResponse
                                                         *ns8__RemoveFromWhitelistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__DeleteWhitelist(struct soap *soap,
                                                 struct _ns8__DeleteWhitelist
                                                     *ns8__DeleteWhitelist,
                                                 struct _ns8__DeleteWhitelistResponse
                                                     *ns8__DeleteWhitelistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__GetBlacklist(struct soap *soap,
                                              struct _ns8__GetBlacklist
                                                  *ns8__GetBlacklist,
                                              struct _ns8__GetBlacklistResponse
                                                  *ns8__GetBlacklistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__AddToBlacklist(struct soap *soap,
                                                struct _ns8__AddToBlacklist
                                                    *ns8__AddToBlacklist,
                                                struct _ns8__AddToBlacklistResponse
                                                    *ns8__AddToBlacklistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__RemoveFromBlacklist(struct soap *soap,
                                                     struct _ns8__RemoveFromBlacklist
                                                         *ns8__RemoveFromBlacklist,
                                                     struct _ns8__RemoveFromBlacklistResponse
                                                         *ns8__RemoveFromBlacklistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns8__DeleteBlacklist(struct soap *soap,
                                                 struct _ns8__DeleteBlacklist
                                                     *ns8__DeleteBlacklist,
                                                 struct _ns8__DeleteBlacklistResponse
                                                     *ns8__DeleteBlacklistResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__GetServiceCapabilities(struct soap *soap,
                                                        struct _ns9__GetServiceCapabilities
                                                            *ns9__GetServiceCapabilities,
                                                        struct _ns9__GetServiceCapabilitiesResponse
                                                            *ns9__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__GetDoorInfoList(struct soap *soap,
                                                 struct _ns9__GetDoorInfoList
                                                     *ns9__GetDoorInfoList,
                                                 struct _ns9__GetDoorInfoListResponse
                                                     *ns9__GetDoorInfoListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__GetDoorInfo(struct soap *soap,
                                             struct _ns9__GetDoorInfo
                                                 *ns9__GetDoorInfo,
                                             struct _ns9__GetDoorInfoResponse
                                                 *ns9__GetDoorInfoResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__GetDoorList(struct soap *soap,
                                             struct _ns9__GetDoorList
                                                 *ns9__GetDoorList,
                                             struct _ns9__GetDoorListResponse
                                                 *ns9__GetDoorListResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__GetDoors(struct soap *soap,
                                          struct _ns9__GetDoors
                                              *ns9__GetDoors,
                                          struct _ns9__GetDoorsResponse
                                              *ns9__GetDoorsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__CreateDoor(struct soap *soap,
                                            struct _ns9__CreateDoor
                                                *ns9__CreateDoor,
                                            struct _ns9__CreateDoorResponse
                                                *ns9__CreateDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__SetDoor(struct soap *soap,
                                         struct _ns9__SetDoor
                                             *ns9__SetDoor,
                                         struct _ns9__SetDoorResponse
                                             *ns9__SetDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__ModifyDoor(struct soap *soap,
                                            struct _ns9__ModifyDoor
                                                *ns9__ModifyDoor,
                                            struct _ns9__ModifyDoorResponse
                                                *ns9__ModifyDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__DeleteDoor(struct soap *soap,
                                            struct _ns9__DeleteDoor
                                                *ns9__DeleteDoor,
                                            struct _ns9__DeleteDoorResponse
                                                *ns9__DeleteDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__GetDoorState(struct soap *soap,
                                              struct _ns9__GetDoorState
                                                  *ns9__GetDoorState,
                                              struct _ns9__GetDoorStateResponse
                                                  *ns9__GetDoorStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__AccessDoor(struct soap *soap,
                                            struct _ns9__AccessDoor
                                                *ns9__AccessDoor,
                                            struct _ns9__AccessDoorResponse
                                                *ns9__AccessDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__LockDoor(struct soap *soap,
                                          struct _ns9__LockDoor
                                              *ns9__LockDoor,
                                          struct _ns9__LockDoorResponse
                                              *ns9__LockDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__UnlockDoor(struct soap *soap,
                                            struct _ns9__UnlockDoor
                                                *ns9__UnlockDoor,
                                            struct _ns9__UnlockDoorResponse
                                                *ns9__UnlockDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__BlockDoor(struct soap *soap,
                                           struct _ns9__BlockDoor
                                               *ns9__BlockDoor,
                                           struct _ns9__BlockDoorResponse
                                               *ns9__BlockDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__LockDownDoor(struct soap *soap,
                                              struct _ns9__LockDownDoor
                                                  *ns9__LockDownDoor,
                                              struct _ns9__LockDownDoorResponse
                                                  *ns9__LockDownDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__LockDownReleaseDoor(struct soap *soap,
                                                     struct _ns9__LockDownReleaseDoor
                                                         *ns9__LockDownReleaseDoor,
                                                     struct _ns9__LockDownReleaseDoorResponse
                                                         *ns9__LockDownReleaseDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__LockOpenDoor(struct soap *soap,
                                              struct _ns9__LockOpenDoor
                                                  *ns9__LockOpenDoor,
                                              struct _ns9__LockOpenDoorResponse
                                                  *ns9__LockOpenDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__LockOpenReleaseDoor(struct soap *soap,
                                                     struct _ns9__LockOpenReleaseDoor
                                                         *ns9__LockOpenReleaseDoor,
                                                     struct _ns9__LockOpenReleaseDoorResponse
                                                         *ns9__LockOpenReleaseDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns9__DoubleLockDoor(struct soap *soap,
                                                struct _ns9__DoubleLockDoor
                                                    *ns9__DoubleLockDoor,
                                                struct _ns9__DoubleLockDoorResponse
                                                    *ns9__DoubleLockDoorResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetServiceCapabilities(struct soap *soap,
                                                        struct _tad__GetServiceCapabilities
                                                            *tad__GetServiceCapabilities,
                                                        struct _tad__GetServiceCapabilitiesResponse
                                                            *tad__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__DeleteAnalyticsEngineControl(struct soap *soap,
                                                              struct _tad__DeleteAnalyticsEngineControl
                                                                  *tad__DeleteAnalyticsEngineControl,
                                                              struct _tad__DeleteAnalyticsEngineControlResponse
                                                                  *tad__DeleteAnalyticsEngineControlResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__CreateAnalyticsEngineControl(struct soap *soap,
                                                              struct _tad__CreateAnalyticsEngineControl
                                                                  *tad__CreateAnalyticsEngineControl,
                                                              struct _tad__CreateAnalyticsEngineControlResponse
                                                                  *tad__CreateAnalyticsEngineControlResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetAnalyticsEngineControl(struct soap *soap,
                                                           struct _tad__SetAnalyticsEngineControl
                                                               *tad__SetAnalyticsEngineControl,
                                                           struct _tad__SetAnalyticsEngineControlResponse
                                                               *tad__SetAnalyticsEngineControlResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineControl(struct soap *soap,
                                                           struct _tad__GetAnalyticsEngineControl
                                                               *tad__GetAnalyticsEngineControl,
                                                           struct _tad__GetAnalyticsEngineControlResponse
                                                               *tad__GetAnalyticsEngineControlResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineControls(struct soap *soap,
                                                            struct _tad__GetAnalyticsEngineControls
                                                                *tad__GetAnalyticsEngineControls,
                                                            struct _tad__GetAnalyticsEngineControlsResponse
                                                                *tad__GetAnalyticsEngineControlsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngine(struct soap *soap,
                                                    struct _tad__GetAnalyticsEngine
                                                        *tad__GetAnalyticsEngine,
                                                    struct _tad__GetAnalyticsEngineResponse
                                                        *tad__GetAnalyticsEngineResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngines(struct soap *soap,
                                                     struct _tad__GetAnalyticsEngines
                                                         *tad__GetAnalyticsEngines,
                                                     struct _tad__GetAnalyticsEnginesResponse
                                                         *tad__GetAnalyticsEnginesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetVideoAnalyticsConfiguration(struct soap *soap,
                                                                struct _tad__SetVideoAnalyticsConfiguration
                                                                    *tad__SetVideoAnalyticsConfiguration,
                                                                struct _tad__SetVideoAnalyticsConfigurationResponse
                                                                    *tad__SetVideoAnalyticsConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetAnalyticsEngineInput(struct soap *soap,
                                                         struct _tad__SetAnalyticsEngineInput
                                                             *tad__SetAnalyticsEngineInput,
                                                         struct _tad__SetAnalyticsEngineInputResponse
                                                             *tad__SetAnalyticsEngineInputResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineInput(struct soap *soap,
                                                         struct _tad__GetAnalyticsEngineInput
                                                             *tad__GetAnalyticsEngineInput,
                                                         struct _tad__GetAnalyticsEngineInputResponse
                                                             *tad__GetAnalyticsEngineInputResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineInputs(struct soap *soap,
                                                          struct _tad__GetAnalyticsEngineInputs
                                                              *tad__GetAnalyticsEngineInputs,
                                                          struct _tad__GetAnalyticsEngineInputsResponse
                                                              *tad__GetAnalyticsEngineInputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsDeviceStreamUri(struct soap *soap,
                                                             struct _tad__GetAnalyticsDeviceStreamUri
                                                                 *tad__GetAnalyticsDeviceStreamUri,
                                                             struct _tad__GetAnalyticsDeviceStreamUriResponse
                                                                 *tad__GetAnalyticsDeviceStreamUriResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetVideoAnalyticsConfiguration(struct soap *soap,
                                                                struct _tad__GetVideoAnalyticsConfiguration
                                                                    *tad__GetVideoAnalyticsConfiguration,
                                                                struct _tad__GetVideoAnalyticsConfigurationResponse
                                                                    *tad__GetVideoAnalyticsConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__CreateAnalyticsEngineInputs(struct soap *soap,
                                                             struct _tad__CreateAnalyticsEngineInputs
                                                                 *tad__CreateAnalyticsEngineInputs,
                                                             struct _tad__CreateAnalyticsEngineInputsResponse
                                                                 *tad__CreateAnalyticsEngineInputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__DeleteAnalyticsEngineInputs(struct soap *soap,
                                                             struct _tad__DeleteAnalyticsEngineInputs
                                                                 *tad__DeleteAnalyticsEngineInputs,
                                                             struct _tad__DeleteAnalyticsEngineInputsResponse
                                                                 *tad__DeleteAnalyticsEngineInputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsState(struct soap *soap,
                                                   struct _tad__GetAnalyticsState
                                                       *tad__GetAnalyticsState,
                                                   struct _tad__GetAnalyticsStateResponse
                                                       *tad__GetAnalyticsStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules(struct soap *soap,
                                                   struct _tan__GetSupportedRules
                                                       *tan__GetSupportedRules,
                                                   struct _tan__GetSupportedRulesResponse
                                                       *tan__GetSupportedRulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules(struct soap *soap,
                                             struct _tan__CreateRules
                                                 *tan__CreateRules,
                                             struct _tan__CreateRulesResponse
                                                 *tan__CreateRulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules(struct soap *soap,
                                             struct _tan__DeleteRules
                                                 *tan__DeleteRules,
                                             struct _tan__DeleteRulesResponse
                                                 *tan__DeleteRulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap *soap,
                                          struct _tan__GetRules
                                              *tan__GetRules,
                                          struct _tan__GetRulesResponse
                                              *tan__GetRulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRuleOptions(struct soap *soap,
                                                struct _tan__GetRuleOptions
                                                    *tan__GetRuleOptions,
                                                struct _tan__GetRuleOptionsResponse
                                                    *tan__GetRuleOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules(struct soap *soap,
                                             struct _tan__ModifyRules
                                                 *tan__ModifyRules,
                                             struct _tan__ModifyRulesResponse
                                                 *tan__ModifyRulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetServiceCapabilities(struct soap *soap,
                                                        struct _tan__GetServiceCapabilities
                                                            *tan__GetServiceCapabilities,
                                                        struct _tan__GetServiceCapabilitiesResponse
                                                            *tan__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap *soap,
                                                              struct _tan__GetSupportedAnalyticsModules
                                                                  *tan__GetSupportedAnalyticsModules,
                                                              struct _tan__GetSupportedAnalyticsModulesResponse
                                                                  *tan__GetSupportedAnalyticsModulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateAnalyticsModules(struct soap *soap,
                                                        struct _tan__CreateAnalyticsModules
                                                            *tan__CreateAnalyticsModules,
                                                        struct _tan__CreateAnalyticsModulesResponse
                                                            *tan__CreateAnalyticsModulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteAnalyticsModules(struct soap *soap,
                                                        struct _tan__DeleteAnalyticsModules
                                                            *tan__DeleteAnalyticsModules,
                                                        struct _tan__DeleteAnalyticsModulesResponse
                                                            *tan__DeleteAnalyticsModulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap *soap,
                                                     struct _tan__GetAnalyticsModules
                                                         *tan__GetAnalyticsModules,
                                                     struct _tan__GetAnalyticsModulesResponse
                                                         *tan__GetAnalyticsModulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModuleOptions(struct soap *soap,
                                                           struct _tan__GetAnalyticsModuleOptions
                                                               *tan__GetAnalyticsModuleOptions,
                                                           struct _tan__GetAnalyticsModuleOptionsResponse
                                                               *tan__GetAnalyticsModuleOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyAnalyticsModules(struct soap *soap,
                                                        struct _tan__ModifyAnalyticsModules
                                                            *tan__ModifyAnalyticsModules,
                                                        struct _tan__ModifyAnalyticsModulesResponse
                                                            *tan__ModifyAnalyticsModulesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedMetadata(struct soap *soap,
                                                      struct _tan__GetSupportedMetadata
                                                          *tan__GetSupportedMetadata,
                                                      struct _tan__GetSupportedMetadataResponse
                                                          *tan__GetSupportedMetadataResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetServiceCapabilities(struct soap *soap,
                                                        struct _tas__GetServiceCapabilities
                                                            *tas__GetServiceCapabilities,
                                                        struct _tas__GetServiceCapabilitiesResponse
                                                            *tas__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__CreateRSAKeyPair(struct soap *soap,
                                                  struct _tas__CreateRSAKeyPair
                                                      *tas__CreateRSAKeyPair,
                                                  struct _tas__CreateRSAKeyPairResponse
                                                      *tas__CreateRSAKeyPairResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__UploadKeyPairInPKCS8(struct soap *soap,
                                                      struct _tas__UploadKeyPairInPKCS8
                                                          *tas__UploadKeyPairInPKCS8,
                                                      struct _tas__UploadKeyPairInPKCS8Response
                                                          *tas__UploadKeyPairInPKCS8Response)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__UploadCertificateWithPrivateKeyInPKCS12(struct soap *soap,
                                                                         struct _tas__UploadCertificateWithPrivateKeyInPKCS12
                                                                             *tas__UploadCertificateWithPrivateKeyInPKCS12,
                                                                         struct _tas__UploadCertificateWithPrivateKeyInPKCS12Response
                                                                             *tas__UploadCertificateWithPrivateKeyInPKCS12Response)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetKeyStatus(struct soap *soap,
                                              struct _tas__GetKeyStatus
                                                  *tas__GetKeyStatus,
                                              struct _tas__GetKeyStatusResponse
                                                  *tas__GetKeyStatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetPrivateKeyStatus(struct soap *soap,
                                                     struct _tas__GetPrivateKeyStatus
                                                         *tas__GetPrivateKeyStatus,
                                                     struct _tas__GetPrivateKeyStatusResponse
                                                         *tas__GetPrivateKeyStatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllKeys(struct soap *soap,
                                            struct _tas__GetAllKeys
                                                *tas__GetAllKeys,
                                            struct _tas__GetAllKeysResponse
                                                *tas__GetAllKeysResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteKey(struct soap *soap,
                                           struct _tas__DeleteKey
                                               *tas__DeleteKey,
                                           struct _tas__DeleteKeyResponse
                                               *tas__DeleteKeyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__CreatePKCS10CSR(struct soap *soap,
                                                 struct _tas__CreatePKCS10CSR
                                                     *tas__CreatePKCS10CSR,
                                                 struct _tas__CreatePKCS10CSRResponse
                                                     *tas__CreatePKCS10CSRResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__CreateSelfSignedCertificate(struct soap *soap,
                                                             struct _tas__CreateSelfSignedCertificate
                                                                 *tas__CreateSelfSignedCertificate,
                                                             struct _tas__CreateSelfSignedCertificateResponse
                                                                 *tas__CreateSelfSignedCertificateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__UploadCertificate(struct soap *soap,
                                                   struct _tas__UploadCertificate
                                                       *tas__UploadCertificate,
                                                   struct _tas__UploadCertificateResponse
                                                       *tas__UploadCertificateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetCertificate(struct soap *soap,
                                                struct _tas__GetCertificate
                                                    *tas__GetCertificate,
                                                struct _tas__GetCertificateResponse
                                                    *tas__GetCertificateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllCertificates(struct soap *soap,
                                                    struct _tas__GetAllCertificates
                                                        *tas__GetAllCertificates,
                                                    struct _tas__GetAllCertificatesResponse
                                                        *tas__GetAllCertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteCertificate(struct soap *soap,
                                                   struct _tas__DeleteCertificate
                                                       *tas__DeleteCertificate,
                                                   struct _tas__DeleteCertificateResponse
                                                       *tas__DeleteCertificateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__CreateCertificationPath(struct soap *soap,
                                                         struct _tas__CreateCertificationPath
                                                             *tas__CreateCertificationPath,
                                                         struct _tas__CreateCertificationPathResponse
                                                             *tas__CreateCertificationPathResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetCertificationPath(struct soap *soap,
                                                      struct _tas__GetCertificationPath
                                                          *tas__GetCertificationPath,
                                                      struct _tas__GetCertificationPathResponse
                                                          *tas__GetCertificationPathResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllCertificationPaths(struct soap *soap,
                                                          struct _tas__GetAllCertificationPaths
                                                              *tas__GetAllCertificationPaths,
                                                          struct _tas__GetAllCertificationPathsResponse
                                                              *tas__GetAllCertificationPathsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteCertificationPath(struct soap *soap,
                                                         struct _tas__DeleteCertificationPath
                                                             *tas__DeleteCertificationPath,
                                                         struct _tas__DeleteCertificationPathResponse
                                                             *tas__DeleteCertificationPathResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__UploadPassphrase(struct soap *soap,
                                                  struct _tas__UploadPassphrase
                                                      *tas__UploadPassphrase,
                                                  struct _tas__UploadPassphraseResponse
                                                      *tas__UploadPassphraseResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllPassphrases(struct soap *soap,
                                                   struct _tas__GetAllPassphrases
                                                       *tas__GetAllPassphrases,
                                                   struct _tas__GetAllPassphrasesResponse
                                                       *tas__GetAllPassphrasesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeletePassphrase(struct soap *soap,
                                                  struct _tas__DeletePassphrase
                                                      *tas__DeletePassphrase,
                                                  struct _tas__DeletePassphraseResponse
                                                      *tas__DeletePassphraseResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__UploadCRL(struct soap *soap,
                                           struct _tas__UploadCRL
                                               *tas__UploadCRL,
                                           struct _tas__UploadCRLResponse
                                               *tas__UploadCRLResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetCRL(struct soap *soap,
                                        struct _tas__GetCRL
                                            *tas__GetCRL,
                                        struct _tas__GetCRLResponse
                                            *tas__GetCRLResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllCRLs(struct soap *soap,
                                            struct _tas__GetAllCRLs
                                                *tas__GetAllCRLs,
                                            struct _tas__GetAllCRLsResponse
                                                *tas__GetAllCRLsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteCRL(struct soap *soap,
                                           struct _tas__DeleteCRL
                                               *tas__DeleteCRL,
                                           struct _tas__DeleteCRLResponse
                                               *tas__DeleteCRLResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__CreateCertPathValidationPolicy(struct soap *soap,
                                                                struct _tas__CreateCertPathValidationPolicy
                                                                    *tas__CreateCertPathValidationPolicy,
                                                                struct _tas__CreateCertPathValidationPolicyResponse
                                                                    *tas__CreateCertPathValidationPolicyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetCertPathValidationPolicy(struct soap *soap,
                                                             struct _tas__GetCertPathValidationPolicy
                                                                 *tas__GetCertPathValidationPolicy,
                                                             struct _tas__GetCertPathValidationPolicyResponse
                                                                 *tas__GetCertPathValidationPolicyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllCertPathValidationPolicies(struct soap *soap,
                                                                  struct _tas__GetAllCertPathValidationPolicies
                                                                      *tas__GetAllCertPathValidationPolicies,
                                                                  struct _tas__GetAllCertPathValidationPoliciesResponse
                                                                      *tas__GetAllCertPathValidationPoliciesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteCertPathValidationPolicy(struct soap *soap,
                                                                struct _tas__DeleteCertPathValidationPolicy
                                                                    *tas__DeleteCertPathValidationPolicy,
                                                                struct _tas__DeleteCertPathValidationPolicyResponse
                                                                    *tas__DeleteCertPathValidationPolicyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__AddServerCertificateAssignment(struct soap *soap,
                                                                struct _tas__AddServerCertificateAssignment
                                                                    *tas__AddServerCertificateAssignment,
                                                                struct _tas__AddServerCertificateAssignmentResponse
                                                                    *tas__AddServerCertificateAssignmentResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__RemoveServerCertificateAssignment(struct soap *soap,
                                                                   struct _tas__RemoveServerCertificateAssignment
                                                                       *tas__RemoveServerCertificateAssignment,
                                                                   struct _tas__RemoveServerCertificateAssignmentResponse
                                                                       *tas__RemoveServerCertificateAssignmentResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__ReplaceServerCertificateAssignment(struct soap *soap,
                                                                    struct _tas__ReplaceServerCertificateAssignment
                                                                        *tas__ReplaceServerCertificateAssignment,
                                                                    struct _tas__ReplaceServerCertificateAssignmentResponse
                                                                        *tas__ReplaceServerCertificateAssignmentResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAssignedServerCertificates(struct soap *soap,
                                                               struct _tas__GetAssignedServerCertificates
                                                                   *tas__GetAssignedServerCertificates,
                                                               struct _tas__GetAssignedServerCertificatesResponse
                                                                   *tas__GetAssignedServerCertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__SetEnabledTLSVersions(struct soap *soap,
                                                       struct _tas__SetEnabledTLSVersions
                                                           *tas__SetEnabledTLSVersions,
                                                       struct _tas__SetEnabledTLSVersionsResponse
                                                           *tas__SetEnabledTLSVersionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetEnabledTLSVersions(struct soap *soap,
                                                       struct _tas__GetEnabledTLSVersions
                                                           *tas__GetEnabledTLSVersions,
                                                       struct _tas__GetEnabledTLSVersionsResponse
                                                           *tas__GetEnabledTLSVersionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__SetClientAuthenticationRequired(struct soap *soap,
                                                                 struct _tas__SetClientAuthenticationRequired
                                                                     *tas__SetClientAuthenticationRequired,
                                                                 struct _tas__SetClientAuthenticationRequiredResponse
                                                                     *tas__SetClientAuthenticationRequiredResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetClientAuthenticationRequired(struct soap *soap,
                                                                 struct _tas__GetClientAuthenticationRequired
                                                                     *tas__GetClientAuthenticationRequired,
                                                                 struct _tas__GetClientAuthenticationRequiredResponse
                                                                     *tas__GetClientAuthenticationRequiredResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__SetCnMapsToUser(struct soap *soap,
                                                 struct _tas__SetCnMapsToUser
                                                     *tas__SetCnMapsToUser,
                                                 struct _tas__SetCnMapsToUserResponse
                                                     *tas__SetCnMapsToUserResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetCnMapsToUser(struct soap *soap,
                                                 struct _tas__GetCnMapsToUser
                                                     *tas__GetCnMapsToUser,
                                                 struct _tas__GetCnMapsToUserResponse
                                                     *tas__GetCnMapsToUserResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__AddCertPathValidationPolicyAssignment(struct soap *soap,
                                                                       struct _tas__AddCertPathValidationPolicyAssignment
                                                                           *tas__AddCertPathValidationPolicyAssignment,
                                                                       struct _tas__AddCertPathValidationPolicyAssignmentResponse
                                                                           *tas__AddCertPathValidationPolicyAssignmentResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__RemoveCertPathValidationPolicyAssignment(struct soap *soap,
                                                                          struct _tas__RemoveCertPathValidationPolicyAssignment
                                                                              *tas__RemoveCertPathValidationPolicyAssignment,
                                                                          struct _tas__RemoveCertPathValidationPolicyAssignmentResponse
                                                                              *tas__RemoveCertPathValidationPolicyAssignmentResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__ReplaceCertPathValidationPolicyAssignment(struct soap *soap,
                                                                           struct _tas__ReplaceCertPathValidationPolicyAssignment
                                                                               *tas__ReplaceCertPathValidationPolicyAssignment,
                                                                           struct _tas__ReplaceCertPathValidationPolicyAssignmentResponse
                                                                               *tas__ReplaceCertPathValidationPolicyAssignmentResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAssignedCertPathValidationPolicies(struct soap *soap,
                                                                       struct _tas__GetAssignedCertPathValidationPolicies
                                                                           *tas__GetAssignedCertPathValidationPolicies,
                                                                       struct _tas__GetAssignedCertPathValidationPoliciesResponse
                                                                           *tas__GetAssignedCertPathValidationPoliciesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__AddDot1XConfiguration(struct soap *soap,
                                                       struct _tas__AddDot1XConfiguration
                                                           *tas__AddDot1XConfiguration,
                                                       struct _tas__AddDot1XConfigurationResponse
                                                           *tas__AddDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetAllDot1XConfigurations(struct soap *soap,
                                                           struct _tas__GetAllDot1XConfigurations
                                                               *tas__GetAllDot1XConfigurations,
                                                           struct _tas__GetAllDot1XConfigurationsResponse
                                                               *tas__GetAllDot1XConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetDot1XConfiguration(struct soap *soap,
                                                       struct _tas__GetDot1XConfiguration
                                                           *tas__GetDot1XConfiguration,
                                                       struct _tas__GetDot1XConfigurationResponse
                                                           *tas__GetDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteDot1XConfiguration(struct soap *soap,
                                                          struct _tas__DeleteDot1XConfiguration
                                                              *tas__DeleteDot1XConfiguration,
                                                          struct _tas__DeleteDot1XConfigurationResponse
                                                              *tas__DeleteDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__SetNetworkInterfaceDot1XConfiguration(struct soap *soap,
                                                                       struct _tas__SetNetworkInterfaceDot1XConfiguration
                                                                           *tas__SetNetworkInterfaceDot1XConfiguration,
                                                                       struct _tas__SetNetworkInterfaceDot1XConfigurationResponse
                                                                           *tas__SetNetworkInterfaceDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__GetNetworkInterfaceDot1XConfiguration(struct soap *soap,
                                                                       struct _tas__GetNetworkInterfaceDot1XConfiguration
                                                                           *tas__GetNetworkInterfaceDot1XConfiguration,
                                                                       struct _tas__GetNetworkInterfaceDot1XConfigurationResponse
                                                                           *tas__GetNetworkInterfaceDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tas__DeleteNetworkInterfaceDot1XConfiguration(struct soap *soap,
                                                                          struct _tas__DeleteNetworkInterfaceDot1XConfiguration
                                                                              *tas__DeleteNetworkInterfaceDot1XConfiguration,
                                                                          struct _tas__DeleteNetworkInterfaceDot1XConfigurationResponse
                                                                              *tas__DeleteNetworkInterfaceDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap *soap,
                                       struct wsdd__HelloType tdn__Hello,
                                       struct wsdd__ResolveType *tdn__HelloResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap *soap,
                                     struct wsdd__ByeType tdn__Bye,
                                     struct wsdd__ResolveType *tdn__ByeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap *soap,
                                       struct wsdd__ProbeType tdn__Probe,
                                       struct wsdd__ProbeMatchesType *tdn__ProbeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap *soap,
                                             struct _tds__GetServices *tds__GetServices,
                                             struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
    LOG_FUNC_IN();
    WSSE_SECURITY_DELETE;

    int i = 0;
    struct tds__Service *Service;
    tds__GetServicesResponse->__sizeService = gOnvifServices.size;
    log_com("service size %d", gOnvifServices.size);
    ALLOC_STRUCT_NUM(Service, struct tds__Service, gOnvifServices.size);
    for (i = 0; i < tds__GetServicesResponse->__sizeService; i++)
    {
        Service[i].Namespace = soap_strdup(soap, gOnvifServices.namespaces[i]);
        char tmp_ip[DEVICE_IP_LEN] = {0};
        sprintf(tmp_ip, "http://%s:%d/onvif/services", gOnvifDevInfo.ip, ONVIF_TCP_PORT);
        Service[i].XAddr = soap_strdup(soap, tmp_ip);
        Service[i].Capabilities = NULL;
        ALLOC_STRUCT(Service[i].Version, struct tt__OnvifVersion);
        Service[i].Version->Major = 1;
        Service[i].Version->Minor = 10;
        Service[i].__size = 0;
        Service[i].__any = NULL;
        Service[i].__anyAttribute = NULL;
        log_com("service: %s \t xaddr: %s", 
            Service[i].Namespace, Service[i].XAddr);
    }
    tds__GetServicesResponse->Service = Service;

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap *soap,
                                                        struct _tds__GetServiceCapabilities
                                                            *tds__GetServiceCapabilities,
                                                        struct _tds__GetServiceCapabilitiesResponse
                                                            *tds__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    WSSE_SECURITY_DELETE;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap *soap,
                                                      struct _tds__GetDeviceInformation
                                                          *tds__GetDeviceInformation,
                                                      struct _tds__GetDeviceInformationResponse
                                                          *tds__GetDeviceInformationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    tds__GetDeviceInformationResponse->Manufacturer = 
        soap_strdup(soap, gOnvifProductInfo.manufacturer);
    tds__GetDeviceInformationResponse->Model = 
        soap_strdup(soap, gOnvifProductInfo.model);
    tds__GetDeviceInformationResponse->FirmwareVersion = 
        soap_strdup(soap, gOnvifProductInfo.firmwareVersion);
    tds__GetDeviceInformationResponse->SerialNumber = 
        soap_strdup(soap, gOnvifProductInfo.serialNumber);
    tds__GetDeviceInformationResponse->HardwareId = 
        soap_strdup(soap, gOnvifProductInfo.hardwareID);

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap *soap,
                                                      struct _tds__SetSystemDateAndTime
                                                          *tds__SetSystemDateAndTime,
                                                      struct _tds__SetSystemDateAndTimeResponse
                                                          *tds__SetSystemDateAndTimeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    log_com("dateTimeType %d DaylightSavings %d",
        tds__SetSystemDateAndTime->DateTimeType, 
        tds__SetSystemDateAndTime->DaylightSavings);

    if(tds__SetSystemDateAndTime->TimeZone == NULL)
    {
        log_war("TimeZone is NULL");
    }
    else if(tds__SetSystemDateAndTime->TimeZone->TZ == NULL)
    {
        log_war("TimeZone->TZ is NULL");
    }
    else
    {
        log_com("TimeZone %s", tds__SetSystemDateAndTime->TimeZone->TZ);
    }

    if(tds__SetSystemDateAndTime->UTCDateTime == NULL)
    {
        log_war("UTCDateTime is NULL");
    }
    else
    {
        if(tds__SetSystemDateAndTime->UTCDateTime->Time == NULL)
        {
            log_war("UTCDateTime->Time is NULL");
        }
        else
        {
            log_com("UTCDateTime time: %d:%d:%d",
            tds__SetSystemDateAndTime->UTCDateTime->Time->Hour,
            tds__SetSystemDateAndTime->UTCDateTime->Time->Minute,
            tds__SetSystemDateAndTime->UTCDateTime->Time->Second);
        }
        if(tds__SetSystemDateAndTime->UTCDateTime->Date == NULL)
        {
            log_war("UTCDateTime->Date is NULL");
        }
        else
        {
            log_com("UTCDateTime date: %d/%d/%d",
                tds__SetSystemDateAndTime->UTCDateTime->Date->Year,
                tds__SetSystemDateAndTime->UTCDateTime->Date->Month,
                tds__SetSystemDateAndTime->UTCDateTime->Date->Day);
        }
    }
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap *soap,
                                                      struct _tds__GetSystemDateAndTime
                                                          *tds__GetSystemDateAndTime,
                                                      struct _tds__GetSystemDateAndTimeResponse
                                                          *tds__GetSystemDateAndTimeResponse)
{
    LOG_FUNC_IN();

    time_t now;
    struct tm *timenow;
    time(&now);
    timenow = localtime(&now);

    struct tt__SystemDateTime *SystemDateAndTime = NULL;
    ALLOC_STRUCT(SystemDateAndTime, struct tt__SystemDateTime);

    // SystemDateTime
    SystemDateAndTime->DateTimeType = gSystemCfg.timeType;
    SystemDateAndTime->DaylightSavings = xsd__boolean__false_;
    ALLOC_STRUCT(SystemDateAndTime->TimeZone, struct tt__TimeZone);
    char tmp_tz[32];
    sprintf(tmp_tz, "GMT+%02d:00", gSystemCfg.timeZoom);
    SystemDateAndTime->TimeZone->TZ = soap_strdup(soap, tmp_tz);

    // LocalDateTime
    struct tt__DateTime *LocalDateTime = NULL;
    ALLOC_STRUCT(SystemDateAndTime->LocalDateTime, struct tt__DateTime);
    ALLOC_STRUCT(SystemDateAndTime->LocalDateTime->Time, struct tt__Time);
    ALLOC_STRUCT(SystemDateAndTime->LocalDateTime->Date, struct tt__Date);
    SystemDateAndTime->LocalDateTime->Time->Hour    = timenow->tm_hour;// + gSystemCfg.timeZoom;
    SystemDateAndTime->LocalDateTime->Time->Minute  = timenow->tm_min;
    SystemDateAndTime->LocalDateTime->Time->Second  = timenow->tm_sec;
    SystemDateAndTime->LocalDateTime->Date->Year    = timenow->tm_year + 1900;
    SystemDateAndTime->LocalDateTime->Date->Month   = timenow->tm_mon + 1;
    SystemDateAndTime->LocalDateTime->Date->Day     = timenow->tm_mday;

    log_com("local time: %d:%d:%d  %d-%d-%d timeZoom %d",
        timenow->tm_hour, timenow->tm_min, timenow->tm_sec,
        timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday,
        gSystemCfg.timeZoom);

    timenow = gmtime(&now);
    // UTCDateTime
    ALLOC_STRUCT(SystemDateAndTime->UTCDateTime, struct tt__DateTime);
    ALLOC_STRUCT(SystemDateAndTime->UTCDateTime->Time, struct tt__Time);
    ALLOC_STRUCT(SystemDateAndTime->UTCDateTime->Date, struct tt__Date);
    SystemDateAndTime->UTCDateTime->Time->Hour      = timenow->tm_hour;
    SystemDateAndTime->UTCDateTime->Time->Minute    = timenow->tm_min;
    SystemDateAndTime->UTCDateTime->Time->Second    = timenow->tm_sec;
    SystemDateAndTime->UTCDateTime->Date->Year      = timenow->tm_year + 1900;
    SystemDateAndTime->UTCDateTime->Date->Month     = timenow->tm_mon + 1;
    SystemDateAndTime->UTCDateTime->Date->Day       = timenow->tm_mday;

    log_com("UTC time: %d:%d:%d  %d-%d-%d",
        timenow->tm_hour, timenow->tm_min, timenow->tm_sec,
        timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday);

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime = SystemDateAndTime;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap *soap,
                                                         struct _tds__SetSystemFactoryDefault
                                                             *tds__SetSystemFactoryDefault,
                                                         struct _tds__SetSystemFactoryDefaultResponse
                                                             *tds__SetSystemFactoryDefaultResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    log_war("system default type: %d", tds__SetSystemFactoryDefault->FactoryDefault);
    if (tds__SetSystemFactoryDefault->FactoryDefault == tt__FactoryDefaultType__Soft)
    {
        onvif_device_init();
        onvif_event_manager_init();
    }
    else
    {
        gSystemStatus.hello_bye = 1;
        sem_post(&gSystemStatus.hello_bye_sem);
    }
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap *soap,
                                                       struct _tds__UpgradeSystemFirmware
                                                           *tds__UpgradeSystemFirmware,
                                                       struct _tds__UpgradeSystemFirmwareResponse
                                                           *tds__UpgradeSystemFirmwareResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    log_war("firmware: ptr %x size %d id %s type %s option %s contenttyoe %s",  
    // log_war("firmware contentype %s",
        tds__UpgradeSystemFirmware->Firmware->xop__Include.__ptr,
        tds__UpgradeSystemFirmware->Firmware->xop__Include.__size,
        tds__UpgradeSystemFirmware->Firmware->xop__Include.id,
        tds__UpgradeSystemFirmware->Firmware->xop__Include.type,
        tds__UpgradeSystemFirmware->Firmware->xop__Include.options,
        tds__UpgradeSystemFirmware->Firmware->xmime__contentType);
    
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap *soap,
                                              struct _tds__SystemReboot
                                                  *tds__SystemReboot,
                                              struct _tds__SystemRebootResponse
                                                  *tds__SystemRebootResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    tds__SystemRebootResponse->Message = soap_strdup(soap, "Rebooting");
    gSystemStatus.hello_bye = 2; // bye
    sem_post(&gSystemStatus.hello_bye_sem);
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap *soap,
                                               struct _tds__RestoreSystem
                                                   *tds__RestoreSystem,
                                               struct _tds__RestoreSystemResponse
                                                   *tds__RestoreSystemResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap *soap,
                                                 struct _tds__GetSystemBackup *tds__GetSystemBackup,
                                                 struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap *soap,
                                              struct _tds__GetSystemLog *tds__GetSystemLog,
                                              struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap *soap,
                                                             struct _tds__GetSystemSupportInformation
                                                                 *tds__GetSystemSupportInformation,
                                                             struct _tds__GetSystemSupportInformationResponse
                                                                 *tds__GetSystemSupportInformationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap *soap,
                                           struct _tds__GetScopes *tds__GetScopes,
                                           struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    int i = 0;
    log_com("scope size %d", gOnvifDevScopes.size);
    tds__GetScopesResponse->__sizeScopes = gOnvifDevScopes.size;
    struct tt__Scope *Scopes = NULL;
    ALLOC_STRUCT_NUM(Scopes, struct tt__Scope, gOnvifDevScopes.size);
    for(int i = 0; i < gOnvifDevScopes.size; i++)
    {
        Scopes[i].ScopeDef = tt__ScopeDefinition__Configurable;
        Scopes[i].ScopeItem = soap_strdup(soap, gOnvifDevScopes.scopes[i]);
        log_com("scopes: %s", Scopes[i].ScopeItem);
    }
    tds__GetScopesResponse->Scopes = Scopes;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap *soap,
                                           struct _tds__SetScopes *tds__SetScopes,
                                           struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    int i = 0;
    int size = tds__SetScopes->__sizeScopes;
    log_com("size %d", tds__SetScopes->__sizeScopes);
    for(int i = 0; i < size; i++) {
        log_com("index[%d]: %s", i, tds__SetScopes->Scopes[i]);
    }
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap *soap,
                                           struct _tds__AddScopes *tds__AddScopes,
                                           struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap *soap,
                                              struct _tds__RemoveScopes *tds__RemoveScopes,
                                              struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap *soap,
                                                  struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode,
                                                  struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap *soap,
                                                  struct _tds__SetDiscoveryMode
                                                      *tds__SetDiscoveryMode,
                                                  struct _tds__SetDiscoveryModeResponse
                                                      *tds__SetDiscoveryModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap *soap,
                                                        struct _tds__GetRemoteDiscoveryMode
                                                            *tds__GetRemoteDiscoveryMode,
                                                        struct _tds__GetRemoteDiscoveryModeResponse
                                                            *tds__GetRemoteDiscoveryModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap *soap,
                                                        struct _tds__SetRemoteDiscoveryMode
                                                            *tds__SetRemoteDiscoveryMode,
                                                        struct _tds__SetRemoteDiscoveryModeResponse
                                                            *tds__SetRemoteDiscoveryModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap *soap,
                                                struct _tds__GetDPAddresses
                                                    *tds__GetDPAddresses,
                                                struct _tds__GetDPAddressesResponse
                                                    *tds__GetDPAddressesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap *soap,
                                                      struct _tds__GetEndpointReference
                                                          *tds__GetEndpointReference,
                                                      struct _tds__GetEndpointReferenceResponse
                                                          *tds__GetEndpointReferenceResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap *soap,
                                               struct _tds__GetRemoteUser
                                                   *tds__GetRemoteUser,
                                               struct _tds__GetRemoteUserResponse
                                                   *tds__GetRemoteUserResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap *soap,
                                               struct _tds__SetRemoteUser *tds__SetRemoteUser,
                                               struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap *soap,
                                          struct _tds__GetUsers *tds__GetUsers,
                                          struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    tds__GetUsersResponse->__sizeUser = 1;
    ALLOC_STRUCT(tds__GetUsersResponse->User, struct tt__User);
    tds__GetUsersResponse->User->Username = soap_strdup(soap, gOnvifUsers.current->name);
    tds__GetUsersResponse->User->Password = soap_strdup(soap, gOnvifUsers.current->password);
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap *soap,
                                             struct _tds__CreateUsers
                                                 *tds__CreateUsers,
                                             struct _tds__CreateUsersResponse
                                                 *tds__CreateUsersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap *soap,
                                             struct _tds__DeleteUsers
                                                 *tds__DeleteUsers,
                                             struct _tds__DeleteUsersResponse
                                                 *tds__DeleteUsersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap *soap,
                                         struct _tds__SetUser *tds__SetUser,
                                         struct _tds__SetUserResponse *tds__SetUserResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap *soap,
                                            struct _tds__GetWsdlUrl *tds__GetWsdlUrl,
                                            struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap *soap,
                                                 struct _tds__GetCapabilities
                                                     *tds__GetCapabilities,
                                                 struct _tds__GetCapabilitiesResponse
                                                     *tds__GetCapabilitiesResponse)
{
    LOG_FUNC_IN();
    WSSE_SECURITY_DELETE;
    char _IPv4Address[ONVIF_IP_ADDR_LENGTH] = {0};
    char _IPAddr[ONVIF_IP_ADDR_LENGTH] = {0};

    struct tt__AnalyticsCapabilities* Analytics = NULL;
    struct tt__DeviceCapabilities* Device = NULL;
    struct tt__EventCapabilities* Events = NULL;
    struct tt__ImagingCapabilities* Imaging = NULL;
    struct tt__MediaCapabilities* Media = NULL;
    struct tt__PTZCapabilities* PTZ = NULL;
    struct tt__CapabilitiesExtension* Extension = NULL;
    struct tt__Capabilities* Capabilities = NULL;
    ALLOC_STRUCT(Capabilities, struct tt__Capabilities);
    soap_default_tt__Capabilities(soap, Capabilities);

    if(tds__GetCapabilities->Category == NULL)
    {
        log_err("category is NULL");
        tds__GetCapabilities->Category = 
            (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(enum tt__CapabilityCategory));
        *tds__GetCapabilities->Category = tt__CapabilityCategory__All;
    } 
    log_com("get capability category %d", *tds__GetCapabilities->Category);
    sprintf(_IPv4Address, "http://%s:%d/onvif/services", gOnvifDevInfo.ip, ONVIF_TCP_PORT);
    sprintf(_IPAddr, "http://%s:%d/onvif/device_service", gOnvifDevInfo.ip, ONVIF_TCP_PORT);
 
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All ||
        *tds__GetCapabilities->Category == tt__CapabilityCategory__Analytics)
    {
        ALLOC_STRUCT(Analytics, struct tt__AnalyticsCapabilities);
        Analytics->XAddr = soap_strdup(soap, _IPv4Address);
        Analytics->RuleSupport = xsd__boolean__true_;
        Analytics->AnalyticsModuleSupport = xsd__boolean__true_;
        Analytics->__size = 0;
        Analytics->__any = NULL;
        Analytics->__anyAttribute = NULL;
    }

    // Device
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All ||
        *tds__GetCapabilities->Category == tt__CapabilityCategory__Device)
    { 
        ALLOC_STRUCT(Device, struct tt__DeviceCapabilities);

        struct tt__NetworkCapabilities* Network = NULL;
        struct tt__SystemCapabilities* System = NULL;
        struct tt__IOCapabilities* IO = NULL;
        struct tt__SecurityCapabilities* Security = NULL;
        struct tt__DeviceCapabilitiesExtension *Extension = NULL;

        Device->XAddr = soap_strdup(soap, _IPv4Address); // _IPAddr _IPv4Address
        Device->__anyAttribute = NULL;
        
        log_com("device xaddr %s", Device->XAddr);
        log_com("test ---- , sizeof enum %d", sizeof(enum xsd__boolean));
        // Device->Network
        ALLOC_STRUCT(Network, struct tt__NetworkCapabilities);
        Network->IPFilter =             (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        Network->ZeroConfiguration =    (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        Network->IPVersion6 =           (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        Network->DynDNS =               (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        *Network->IPFilter =            xsd__boolean__false_;
        *Network->ZeroConfiguration =   xsd__boolean__false_;
        *Network->IPVersion6 =          xsd__boolean__false_;
        *Network->DynDNS =              xsd__boolean__false_;

        ALLOC_STRUCT(Network->Extension, struct tt__NetworkCapabilitiesExtension);
        Network->Extension->__size = 1;
        Network->Extension->__any = NULL;
        ALLOC_STRUCT(Network->Extension->Dot11Configuration, enum xsd__boolean);
        *Network->Extension->Dot11Configuration = xsd__boolean__false_;
        Network->Extension->Extension = NULL;
        Device->Network = Network;

        // Device->System
        ALLOC_STRUCT(System, struct tt__SystemCapabilities); 
        System->DiscoveryResolve =  xsd__boolean__true_;
        System->DiscoveryBye =      xsd__boolean__true_;
        System->RemoteDiscovery =   xsd__boolean__true_;
        System->SystemBackup =      xsd__boolean__true_;
        System->SystemLogging =     xsd__boolean__false_;
        System->FirmwareUpgrade =   xsd__boolean__true_;
        System->__sizeSupportedVersions = 1;
        ALLOC_STRUCT_NUM(System->SupportedVersions, struct tt__OnvifVersion, 1); 
        // System->SupportedVersions[0].Major = 0;
        // System->SupportedVersions[0].Minor = 3;
        System->SupportedVersions->Major = 1;
        System->SupportedVersions->Minor = 10;
        ALLOC_STRUCT(System->Extension, struct tt__SystemCapabilitiesExtension);
        System->Extension->HttpFirmwareUpgrade = (enum xsd__boolean *) soap_malloc(soap, sizeof(int));
        System->Extension->HttpSystemBackup = (enum xsd__boolean *) soap_malloc(soap, sizeof(int));
        System->Extension->HttpSystemLogging = (enum xsd__boolean *) soap_malloc(soap, sizeof(int));
        System->Extension->HttpSupportInformation = (enum xsd__boolean *) soap_malloc(soap, sizeof(int));
        *System->Extension->HttpSystemLogging =     xsd__boolean__false_;
        *System->Extension->HttpFirmwareUpgrade =   xsd__boolean__true_;
        *System->Extension->HttpSystemBackup =      xsd__boolean__true_;
        *System->Extension->HttpSupportInformation =xsd__boolean__true_;
        System->Extension->__size = 0;
        System->Extension->__any = NULL;
        System->Extension->Extension = NULL;
        Device->System = System;
       
        // Device->IO
        ALLOC_STRUCT(IO, struct tt__IOCapabilities);
        ALLOC_STRUCT(IO->InputConnectors, int);
        ALLOC_STRUCT(IO->RelayOutputs, int); 
        *IO->InputConnectors = 0;
        // *IO->RelayOutputs = 1;
        *IO->RelayOutputs = 0;
        // IO->Extension = NULL;
        ALLOC_STRUCT(IO->Extension, struct tt__IOCapabilitiesExtension);

        IO->Extension->__size =0;
        IO->Extension->__any = NULL;
        IO->Extension->Auxiliary = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *IO->Extension->Auxiliary = xsd__boolean__false_;
        IO->Extension->__anyAttribute = NULL;
        IO->Extension->__sizeAuxiliaryCommands = 0;
        IO->Extension->AuxiliaryCommands= NULL;
        IO->Extension->Extension = NULL;

        IO->__anyAttribute = NULL;
        Device->IO = IO;

        // Device->Security
        ALLOC_STRUCT(Security, struct tt__SecurityCapabilities);
        Security->RELToken =            xsd__boolean__false_;
        Security->KerberosToken =       xsd__boolean__false_;
        Security->SAMLToken =           xsd__boolean__false_;
        Security->X_x002e509Token =     xsd__boolean__false_;
        Security->AccessPolicyConfig =  xsd__boolean__false_;
        Security->OnboardKeyGeneration =xsd__boolean__false_;
        Security->TLS1_x002e2 =         xsd__boolean__false_;
        Security->TLS1_x002e1 =         xsd__boolean__false_;
        // ALLOC_STRUCT(Security->Extension, struct tt__SecurityCapabilitiesExtension);
        // Security->Extension->TLS1_x002e0 = xsd__boolean__false_;
        // Security->Extension->Extension = NULL;
        Security->Extension = NULL; 
        Security->__size = 0;
        Security->__any = NULL;
        Security->__anyAttribute = NULL;
        Device->Security = Security; 

        // Device->Extension
        Device->Extension = Extension;  // NULL
    }

    // Events
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All ||
        *tds__GetCapabilities->Category == tt__CapabilityCategory__Events)
    {
        ALLOC_STRUCT(Events, struct tt__EventCapabilities); 
        Events->XAddr = soap_strdup(soap, _IPv4Address); 
        // Events->XAddr = soap_strdup(soap, "http://192.168.1.101:8081/event_service");
        Events->WSSubscriptionPolicySupport = xsd__boolean__true_;
        Events->WSPullPointSupport = xsd__boolean__false_;
        Events->WSPausableSubscriptionManagerInterfaceSupport = xsd__boolean__false_;
        Events->__size = 0;
        Events->__any = NULL;
        Events->__anyAttribute = NULL;
        log_com("event ip: %s", Events->XAddr);
    }

    // Imaging
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All ||
        *tds__GetCapabilities->Category == tt__CapabilityCategory__Imaging)
    {
        ALLOC_STRUCT(Imaging, struct tt__ImagingCapabilities);  
        Imaging->XAddr = soap_strdup(soap, _IPv4Address); 
        Imaging->__anyAttribute = NULL;
    }
        
    // Media: must config, if use rtsp
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All ||
        *tds__GetCapabilities->Category == tt__CapabilityCategory__Media)
    {
        ALLOC_STRUCT(Media, struct tt__MediaCapabilities); 
        Media->XAddr = soap_strdup(soap, _IPv4Address); 
        ALLOC_STRUCT(Media->StreamingCapabilities, struct tt__RealTimeStreamingCapabilities);

        Media->StreamingCapabilities->RTPMulticast = 
            (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        Media->StreamingCapabilities->RTP_USCORETCP = 
            (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = 
            (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        *Media->StreamingCapabilities->RTPMulticast = xsd__boolean__false_;
        *Media->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
        *Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;
        Media->StreamingCapabilities->__anyAttribute = NULL;
        Media->StreamingCapabilities->Extension = NULL;

        // ALLOC_STRUCT(Media->Extension, struct tt__MediaCapabilitiesExtension);
        // ALLOC_STRUCT(Media->Extension->ProfileCapabilities, struct tt__ProfileCapabilities);
        // Media->Extension->ProfileCapabilities->MaximumNumberOfProfiles = 7;
        // Media->Extension->ProfileCapabilities->__size = 0;
        // Media->Extension->ProfileCapabilities->__any = NULL;
        // Media->Extension->ProfileCapabilities->__anyAttribute = NULL;
        // Media->Extension->__size = 0;
        // Media->Extension->__any = NULL;
        // Media->Extension->__anyAttribute = NULL;
        Media->Extension = NULL;
        Media->__size = 0;
        Media->__any = NULL;
        Media->__anyAttribute = NULL;
    }
        
    // PTZ  
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All ||
        *tds__GetCapabilities->Category == tt__CapabilityCategory__PTZ)
    {
        ALLOC_STRUCT(PTZ, struct tt__PTZCapabilities);
        PTZ->XAddr = soap_strdup(soap, _IPv4Address); 
        PTZ->__size = 0;
        PTZ->__any = NULL;
        PTZ->__anyAttribute = NULL;
    }

    //Extension
    if(*tds__GetCapabilities->Category == tt__CapabilityCategory__All)
    {
        ALLOC_STRUCT(Extension, struct tt__CapabilitiesExtension);
        //Device IO: must config it ,if need rtsp VideoSources
        ALLOC_STRUCT(Extension->DeviceIO, struct tt__DeviceIOCapabilities);
        Extension->DeviceIO->XAddr = soap_strdup(soap, _IPv4Address); 
        Extension->DeviceIO->VideoSources = xsd__boolean__true_;
        Extension->DeviceIO->VideoOutputs = xsd__boolean__false_;
        Extension->DeviceIO->AudioSources = xsd__boolean__false_;
        Extension->DeviceIO->AudioOutputs = xsd__boolean__false_;
        Extension->DeviceIO->RelayOutputs = xsd__boolean__false_;
        Extension->DeviceIO->__size = 0;
        Extension->DeviceIO->__any = NULL;
        Extension->DeviceIO->__anyAttribute = NULL;
        //Display //Recording //Search //Replay //Receiver //Analytics Device
        Extension->Display = NULL;
        Extension->Recording = NULL;
        Extension->Search = NULL;
        Extension->Replay = NULL;
        Extension->Receiver = NULL;
        Extension->AnalyticsDevice = NULL;
        Extension->Extensions = NULL;
        Extension->__size = 0;
        Extension->__any = NULL;
    }

    Capabilities->Analytics = Analytics;
    Capabilities->Device = Device;
    Capabilities->Events = Events;
    Capabilities->Imaging = Imaging;
    Capabilities->Media = Media;
    Capabilities->PTZ = PTZ;
    Capabilities->Extension = Extension;
    Capabilities->__anyAttribute = NULL;
    tds__GetCapabilitiesResponse->Capabilities = Capabilities;

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap *soap,
                                                struct _tds__SetDPAddresses
                                                    *tds__SetDPAddresses,
                                                struct _tds__SetDPAddressesResponse
                                                    *tds__SetDPAddressesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap *soap,
                                             struct _tds__GetHostname *tds__GetHostname,
                                             struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
    LOG_FUNC_IN();
    
    struct tt__HostnameInformation* HostnameInformation;
    ALLOC_STRUCT(HostnameInformation, struct tt__HostnameInformation);

    HostnameInformation->Extension = NULL;
    HostnameInformation->__anyAttribute = NULL;
    HostnameInformation->Name = "localhost";
    // HostnameInformation->Name = gOnvifDevInfo.hostname;
    HostnameInformation->FromDHCP = xsd__boolean__false_;
    tds__GetHostnameResponse->HostnameInformation = HostnameInformation;

    log_com("hostname: %s", HostnameInformation->Name);
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap *soap,
                                             struct _tds__SetHostname *tds__SetHostname,
                                             struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap *soap,
                                                     struct _tds__SetHostnameFromDHCP
                                                         *tds__SetHostnameFromDHCP,
                                                     struct _tds__SetHostnameFromDHCPResponse
                                                         *tds__SetHostnameFromDHCPResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap *soap,
                                        struct _tds__GetDNS *tds__GetDNS,
                                        struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap *soap,
                                        struct _tds__SetDNS *tds__SetDNS,
                                        struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap *soap,
                                        struct _tds__GetNTP *tds__GetNTP,
                                        struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    struct tt__NTPInformation* NTPInformation;
    // TODO: get ntp from cfg file 
    ALLOC_STRUCT(NTPInformation, struct tt__NTPInformation);

    NTPInformation->FromDHCP = xsd__boolean__false_;
    NTPInformation->__sizeNTPFromDHCP = 0;
    NTPInformation->NTPFromDHCP = NULL;
    NTPInformation->__sizeNTPManual = 1;

    ALLOC_STRUCT(NTPInformation->NTPManual, struct tt__NetworkHost);
    NTPInformation->NTPManual->Type = tt__NetworkHostType__IPv4;
    NTPInformation->NTPManual->IPv4Address = soap_strdup(soap, NTP_SERVER_CN);
    NTPInformation->NTPManual->IPv6Address = NULL;
    NTPInformation->NTPManual->DNSname = NULL;

    NTPInformation->Extension = NULL;
    NTPInformation->__anyAttribute = NULL;
    tds__GetNTPResponse->NTPInformation = NTPInformation;
    
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP, 
                                        struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    if (tds__SetNTP->FromDHCP == xsd__boolean__true_)
    {
        onvif_fault(soap, "ter:NotSupported", "ter:SetDHCPNotAllowed");
        return SOAP_FAULT;
    }

    if(tds__SetNTP->__sizeNTPManual > 0 && tds__SetNTP->NTPManual != NULL)
    {
        if (tds__SetNTP->NTPManual->IPv6Address != NULL ||
        tds__SetNTP->NTPManual->Type == tt__NetworkHostType__IPv6)
        {
            onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidIPv6Address");
            return SOAP_FAULT;
        }

        if(tds__SetNTP->NTPManual->DNSname != NULL ||
                tds__SetNTP->NTPManual->Type == tt__NetworkHostType__DNS)
        {
            onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidDnsName");
            return SOAP_FAULT;
        }
    }


    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap *soap,
                                               struct _tds__GetDynamicDNS
                                                   *tds__GetDynamicDNS,
                                               struct _tds__GetDynamicDNSResponse
                                                   *tds__GetDynamicDNSResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap *soap,
                                               struct _tds__SetDynamicDNS *tds__SetDynamicDNS,
                                               struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap *soap,
                                                      struct _tds__GetNetworkInterfaces
                                                          *tds__GetNetworkInterfaces,
                                                      struct _tds__GetNetworkInterfacesResponse
                                                          *tds__GetNetworkInterfacesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;
    struct tt__NetworkInterface* NetworkInterfaces = NULL;
    ALLOC_STRUCT(NetworkInterfaces, struct tt__NetworkInterface);

    // <NetworkInterfaces>
    NetworkInterfaces->token = soap_strdup(soap, "eth0");
    NetworkInterfaces->Enabled = xsd__boolean__true_;
    //<NetworkInterfaces><Info>
    ALLOC_STRUCT(NetworkInterfaces->Info, struct tt__NetworkInterfaceInfo);
    NetworkInterfaces->Info->Name = soap_strdup(soap, "eth0");

    NetworkInterfaces->Info->HwAddress = soap_strdup(soap, gOnvifDevInfo.mac);
    NetworkInterfaces->Info->MTU = (int *)soap_malloc(soap, sizeof(int));
    *(NetworkInterfaces->Info->MTU) = 1500;

    log_com("mac: %s", NetworkInterfaces->Info->HwAddress);

    //<NetworkInterfaces><Link>
    ALLOC_STRUCT(NetworkInterfaces->Link, struct tt__NetworkInterfaceLink);
    //<NetworkInterfaces><Link><AdminSettings>
    ALLOC_STRUCT(NetworkInterfaces->Link->AdminSettings, struct tt__NetworkInterfaceConnectionSetting);
    NetworkInterfaces->Link->AdminSettings->AutoNegotiation = xsd__boolean__false_;
    NetworkInterfaces->Link->AdminSettings->Speed = 100;
    NetworkInterfaces->Link->AdminSettings->Duplex = tt__Duplex__Full;

    //<NetworkInterfaces><Link><OperSettings>
    ALLOC_STRUCT(NetworkInterfaces->Link->OperSettings, struct tt__NetworkInterfaceConnectionSetting);
    NetworkInterfaces->Link->OperSettings->AutoNegotiation = xsd__boolean__false_;
    NetworkInterfaces->Link->OperSettings->Speed = 100;
    NetworkInterfaces->Link->OperSettings->Duplex = tt__Duplex__Full;
    NetworkInterfaces->Link->InterfaceType = 6;

    //<NetworkInterfaces><IPv4>
    ALLOC_STRUCT(NetworkInterfaces->IPv4, struct tt__IPv4NetworkInterface);
    ALLOC_STRUCT(NetworkInterfaces->IPv4->Config, struct tt__IPv4Configuration);
    ALLOC_STRUCT(NetworkInterfaces->IPv4->Config->Manual, struct tt__PrefixedIPv4Address);
    NetworkInterfaces->IPv4->Enabled = xsd__boolean__true_;
    NetworkInterfaces->IPv4->Config->__sizeManual = 1;
    
    NetworkInterfaces->IPv4->Config->Manual->Address = soap_strdup(soap, gOnvifDevInfo.ip);
    NetworkInterfaces->IPv4->Config->Manual->PrefixLength = 24;
    NetworkInterfaces->IPv4->Config->DHCP = xsd__boolean__false_;

    tds__GetNetworkInterfacesResponse->NetworkInterfaces = NetworkInterfaces;
    LOG_FUNC_OUT();
    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap *soap,
                                                      struct _tds__SetNetworkInterfaces
                                                          *tds__SetNetworkInterfaces,
                                                      struct _tds__SetNetworkInterfacesResponse
                                                          *tds__SetNetworkInterfacesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    gSystemStatus.hello_bye = 1;
    sem_post(&gSystemStatus.hello_bye_sem);
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap *soap,
                                                     struct _tds__GetNetworkProtocols
                                                         *tds__GetNetworkProtocols,
                                                     struct _tds__GetNetworkProtocolsResponse
                                                         *tds__GetNetworkProtocolsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap *soap,
                                                     struct _tds__SetNetworkProtocols
                                                         *tds__SetNetworkProtocols,
                                                     struct _tds__SetNetworkProtocolsResponse
                                                         *tds__SetNetworkProtocolsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap *soap,
                                                          struct _tds__GetNetworkDefaultGateway
                                                              *tds__GetNetworkDefaultGateway,
                                                          struct _tds__GetNetworkDefaultGatewayResponse
                                                              *tds__GetNetworkDefaultGatewayResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap *soap,
                                                          struct _tds__SetNetworkDefaultGateway
                                                              *tds__SetNetworkDefaultGateway,
                                                          struct _tds__SetNetworkDefaultGatewayResponse
                                                              *tds__SetNetworkDefaultGatewayResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap *soap,
                                                      struct _tds__GetZeroConfiguration
                                                          *tds__GetZeroConfiguration,
                                                      struct _tds__GetZeroConfigurationResponse
                                                          *tds__GetZeroConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap *soap,
                                                      struct _tds__SetZeroConfiguration
                                                          *tds__SetZeroConfiguration,
                                                      struct _tds__SetZeroConfigurationResponse
                                                          *tds__SetZeroConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap *soap,
                                                    struct _tds__GetIPAddressFilter
                                                        *tds__GetIPAddressFilter,
                                                    struct _tds__GetIPAddressFilterResponse
                                                        *tds__GetIPAddressFilterResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap *soap,
                                                    struct _tds__SetIPAddressFilter
                                                        *tds__SetIPAddressFilter,
                                                    struct _tds__SetIPAddressFilterResponse
                                                        *tds__SetIPAddressFilterResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap *soap,
                                                    struct _tds__AddIPAddressFilter
                                                        *tds__AddIPAddressFilter,
                                                    struct _tds__AddIPAddressFilterResponse
                                                        *tds__AddIPAddressFilterResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap *soap,
                                                       struct _tds__RemoveIPAddressFilter
                                                           *tds__RemoveIPAddressFilter,
                                                       struct _tds__RemoveIPAddressFilterResponse
                                                           *tds__RemoveIPAddressFilterResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap *soap,
                                                 struct _tds__GetAccessPolicy
                                                     *tds__GetAccessPolicy,
                                                 struct _tds__GetAccessPolicyResponse
                                                     *tds__GetAccessPolicyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap *soap,
                                                 struct _tds__SetAccessPolicy
                                                     *tds__SetAccessPolicy,
                                                 struct _tds__SetAccessPolicyResponse
                                                     *tds__SetAccessPolicyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap *soap,
                                                   struct _tds__CreateCertificate
                                                       *tds__CreateCertificate,
                                                   struct _tds__CreateCertificateResponse
                                                       *tds__CreateCertificateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap *soap,
                                                 struct _tds__GetCertificates
                                                     *tds__GetCertificates,
                                                 struct _tds__GetCertificatesResponse
                                                     *tds__GetCertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap *soap,
                                                       struct _tds__GetCertificatesStatus
                                                           *tds__GetCertificatesStatus,
                                                       struct _tds__GetCertificatesStatusResponse
                                                           *tds__GetCertificatesStatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap *soap,
                                                       struct _tds__SetCertificatesStatus
                                                           *tds__SetCertificatesStatus,
                                                       struct _tds__SetCertificatesStatusResponse
                                                           *tds__SetCertificatesStatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap *soap,
                                                    struct _tds__DeleteCertificates
                                                        *tds__DeleteCertificates,
                                                    struct _tds__DeleteCertificatesResponse
                                                        *tds__DeleteCertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap *soap,
                                                  struct _tds__GetPkcs10Request
                                                      *tds__GetPkcs10Request,
                                                  struct _tds__GetPkcs10RequestResponse
                                                      *tds__GetPkcs10RequestResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap *soap,
                                                  struct _tds__LoadCertificates
                                                      *tds__LoadCertificates,
                                                  struct _tds__LoadCertificatesResponse
                                                      *tds__LoadCertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap *soap,
                                                          struct _tds__GetClientCertificateMode
                                                              *tds__GetClientCertificateMode,
                                                          struct _tds__GetClientCertificateModeResponse
                                                              *tds__GetClientCertificateModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap *soap,
                                                          struct _tds__SetClientCertificateMode
                                                              *tds__SetClientCertificateMode,
                                                          struct _tds__SetClientCertificateModeResponse
                                                              *tds__SetClientCertificateModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap *soap,
                                                 struct _tds__GetRelayOutputs
                                                     *tds__GetRelayOutputs,
                                                 struct _tds__GetRelayOutputsResponse
                                                     *tds__GetRelayOutputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap *soap,
                                                        struct _tds__SetRelayOutputSettings
                                                            *tds__SetRelayOutputSettings,
                                                        struct _tds__SetRelayOutputSettingsResponse
                                                            *tds__SetRelayOutputSettingsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap *soap,
                                                     struct _tds__SetRelayOutputState
                                                         *tds__SetRelayOutputState,
                                                     struct _tds__SetRelayOutputStateResponse
                                                         *tds__SetRelayOutputStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap *soap,
                                                      struct _tds__SendAuxiliaryCommand
                                                          *tds__SendAuxiliaryCommand,
                                                      struct _tds__SendAuxiliaryCommandResponse
                                                          *tds__SendAuxiliaryCommandResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap *soap,
                                                   struct _tds__GetCACertificates
                                                       *tds__GetCACertificates,
                                                   struct _tds__GetCACertificatesResponse
                                                       *tds__GetCACertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap *soap,
                                                               struct _tds__LoadCertificateWithPrivateKey
                                                                   *tds__LoadCertificateWithPrivateKey,
                                                               struct _tds__LoadCertificateWithPrivateKeyResponse
                                                                   *tds__LoadCertificateWithPrivateKeyResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap *soap,
                                                           struct _tds__GetCertificateInformation
                                                               *tds__GetCertificateInformation,
                                                           struct _tds__GetCertificateInformationResponse
                                                               *tds__GetCertificateInformationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap *soap,
                                                    struct _tds__LoadCACertificates
                                                        *tds__LoadCACertificates,
                                                    struct _tds__LoadCACertificatesResponse
                                                        *tds__LoadCACertificatesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap *soap,
                                                          struct _tds__CreateDot1XConfiguration
                                                              *tds__CreateDot1XConfiguration,
                                                          struct _tds__CreateDot1XConfigurationResponse
                                                              *tds__CreateDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap *soap,
                                                       struct _tds__SetDot1XConfiguration
                                                           *tds__SetDot1XConfiguration,
                                                       struct _tds__SetDot1XConfigurationResponse
                                                           *tds__SetDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap *soap,
                                                       struct _tds__GetDot1XConfiguration
                                                           *tds__GetDot1XConfiguration,
                                                       struct _tds__GetDot1XConfigurationResponse
                                                           *tds__GetDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap *soap,
                                                        struct _tds__GetDot1XConfigurations
                                                            *tds__GetDot1XConfigurations,
                                                        struct _tds__GetDot1XConfigurationsResponse
                                                            *tds__GetDot1XConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap *soap,
                                                          struct _tds__DeleteDot1XConfiguration
                                                              *tds__DeleteDot1XConfiguration,
                                                          struct _tds__DeleteDot1XConfigurationResponse
                                                              *tds__DeleteDot1XConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap *soap,
                                                      struct _tds__GetDot11Capabilities
                                                          *tds__GetDot11Capabilities,
                                                      struct _tds__GetDot11CapabilitiesResponse
                                                          *tds__GetDot11CapabilitiesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap *soap,
                                                struct _tds__GetDot11Status
                                                    *tds__GetDot11Status,
                                                struct _tds__GetDot11StatusResponse
                                                    *tds__GetDot11StatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap *soap,
                                                            struct _tds__ScanAvailableDot11Networks
                                                                *tds__ScanAvailableDot11Networks,
                                                            struct _tds__ScanAvailableDot11NetworksResponse
                                                                *tds__ScanAvailableDot11NetworksResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap *soap,
                                               struct _tds__GetSystemUris
                                                   *tds__GetSystemUris,
                                               struct _tds__GetSystemUrisResponse
                                                   *tds__GetSystemUrisResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap *soap,
                                                      struct _tds__StartFirmwareUpgrade
                                                          *tds__StartFirmwareUpgrade,
                                                      struct _tds__StartFirmwareUpgradeResponse
                                                          *tds__StartFirmwareUpgradeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap *soap,
                                                    struct _tds__StartSystemRestore
                                                        *tds__StartSystemRestore,
                                                    struct _tds__StartSystemRestoreResponse
                                                        *tds__StartSystemRestoreResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetStorageConfigurations(struct soap *soap,
                                                          struct _tds__GetStorageConfigurations
                                                              *tds__GetStorageConfigurations,
                                                          struct _tds__GetStorageConfigurationsResponse
                                                              *tds__GetStorageConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateStorageConfiguration(struct soap *soap,
                                                            struct _tds__CreateStorageConfiguration
                                                                *tds__CreateStorageConfiguration,
                                                            struct _tds__CreateStorageConfigurationResponse
                                                                *tds__CreateStorageConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetStorageConfiguration(struct soap *soap,
                                                         struct _tds__GetStorageConfiguration
                                                             *tds__GetStorageConfiguration,
                                                         struct _tds__GetStorageConfigurationResponse
                                                             *tds__GetStorageConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetStorageConfiguration(struct soap *soap,
                                                         struct _tds__SetStorageConfiguration
                                                             *tds__SetStorageConfiguration,
                                                         struct _tds__SetStorageConfigurationResponse
                                                             *tds__SetStorageConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteStorageConfiguration(struct soap *soap,
                                                            struct _tds__DeleteStorageConfiguration
                                                                *tds__DeleteStorageConfiguration,
                                                            struct _tds__DeleteStorageConfigurationResponse
                                                                *tds__DeleteStorageConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetGeoLocation(struct soap *soap,
                                                struct _tds__GetGeoLocation
                                                    *tds__GetGeoLocation,
                                                struct _tds__GetGeoLocationResponse
                                                    *tds__GetGeoLocationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetGeoLocation(struct soap *soap,
                                                struct _tds__SetGeoLocation
                                                    *tds__SetGeoLocation,
                                                struct _tds__SetGeoLocationResponse
                                                    *tds__SetGeoLocationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteGeoLocation(struct soap *soap,
                                                   struct _tds__DeleteGeoLocation
                                                       *tds__DeleteGeoLocation,
                                                   struct _tds__DeleteGeoLocationResponse
                                                       *tds__DeleteGeoLocationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PullMessages(struct soap *soap,
                                              struct _tev__PullMessages
                                                  *tev__PullMessages,
                                              struct _tev__PullMessagesResponse
                                                  *tev__PullMessagesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    if(tev__PullMessages == NULL) {
        log_err("tev__PullMessages is NULL");
        return SOAP_FAULT;
    }
    int nTimeout = tev__PullMessages->Timeout;
    int nMsgLimit = tev__PullMessages->MessageLimit;
    int nSize = tev__PullMessages->__size;
    log_dbg("timeout: %d msglimit: %d size: %d", nTimeout, nMsgLimit, nSize);

    char* tmp = NULL;
    if(soap->header != NULL) {
        log_dbg();
        if(soap->header->wsa5__To != NULL) {
            log_dbg();
            tmp = strstr(soap->header->wsa5__To, "?subscribe=");
            log_dbg("wsa5__To: %s", tmp);
        }
    } else {
        log_err("soap header is NULL");
    }

    nTimeout = 20;
    tmp += strlen("?subscribe=");
    log_dbg("tmp: %s", tmp);
    int id = atoi(tmp);
    log_dbg("sub id: %d", id);

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Seek(struct soap *soap,
                                      struct _tev__Seek
                                          *tev__Seek,
                                      struct _tev__SeekResponse
                                          *tev__SeekResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__SetSynchronizationPoint(struct soap *soap,
                                                         struct _tev__SetSynchronizationPoint
                                                             *tev__SetSynchronizationPoint,
                                                         struct _tev__SetSynchronizationPointResponse
                                                             *tev__SetSynchronizationPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe(struct soap *soap,
                                             struct _wsnt__Unsubscribe
                                                 *tev__Unsubscribe,
                                             struct _wsnt__UnsubscribeResponse
                                                 *tev__UnsubscribeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetServiceCapabilities(struct soap *soap,
                                                        struct _tev__GetServiceCapabilities
                                                            *tev__GetServiceCapabilities,
                                                        struct _tev__GetServiceCapabilitiesResponse
                                                            *tev__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPointSubscription(struct soap *soap,
                                                             struct _tev__CreatePullPointSubscription
                                                                 *tev__CreatePullPointSubscription,
                                                             struct _tev__CreatePullPointSubscriptionResponse
                                                                 *tev__CreatePullPointSubscriptionResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
#if 1
    int nSubscribe;
    time_t nTimeOut = -1;
    int InitialTerminationTime;
    struct _tev__CreatePullPointSubscription* tev_request;
    struct _tev__CreatePullPointSubscriptionResponse* tev_response;

    tev_request = tev__CreatePullPointSubscription;
    tev_response = tev__CreatePullPointSubscriptionResponse;

    if(tev_request->InitialTerminationTime) {
        log_dbg("initial termination time: %s", tev_request->InitialTerminationTime)
        InitialTerminationTime = parse_duration_type(tev_request->InitialTerminationTime);
        log_dbg("InitialTerminationTime: %d", InitialTerminationTime);
        if(InitialTerminationTime != 0) {
            time_t nowtime = time(NULL);
            tev_response->wsnt__CurrentTime = nowtime;
            tev_response->wsnt__TerminationTime = nowtime + InitialTerminationTime;
            nTimeOut = InitialTerminationTime;
        } else {
            InitialTerminationTime = parse_datetime_type(tev_request->InitialTerminationTime);
            log_dbg("InitialTerminationTime: %d", InitialTerminationTime);
            if (InitialTerminationTime != 0) {
                time_t nowtime = time(NULL);
                if(InitialTerminationTime > nowtime) {
                    tev_response->wsnt__CurrentTime = nowtime;
                    tev_response->wsnt__TerminationTime =
                        InitialTerminationTime;
                    nTimeOut = InitialTerminationTime - nowtime;
                } else {
                    log_err();
                    return SOAP_FAULT;
                }
            }
        }
    }

    //动态创建SubscriptionManager
    nSubscribe = onvif_event_new_sub(NULL, nTimeOut);
    log_dbg("nSubscribe: %d", nSubscribe);
    if(nSubscribe > 0) {
        char tmp_ip[128] = {0};
        sprintf(tmp_ip, "http://%s:%d/onvif/event_service?subscribe=%d",
            gOnvifDevInfo.ip, ONVIF_TCP_PORT, nSubscribe);
        tev_response->SubscriptionReference.Address = soap_strdup(soap, tmp_ip);

        if (soap->header) {
            soap_default_SOAP_ENV__Header(soap, soap->header);
            soap->header->wsa5__Action = ONVIF_EVENT_PULL_ACTION;
        }
    }
    else {
        return SOAP_FAULT;
    }
#endif
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetEventProperties(struct soap *soap,
                                                    struct _tev__GetEventProperties
                                                        *tev__GetEventProperties,
                                                    struct _tev__GetEventPropertiesResponse
                                                        *tev__GetEventPropertiesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    struct _tev__GetEventPropertiesResponse *response = tev__GetEventPropertiesResponse;

    response->__sizeTopicNamespaceLocation = gOnvifEventTopicNS.size;
    response->TopicNamespaceLocation = gOnvifEventTopicNS.spaces;
    response->wsnt__FixedTopicSet = xsd__boolean__true_;
    ALLOC_STRUCT(response->wstop__TopicSet, struct wstop__TopicSetType)
    response->wstop__TopicSet->documentation = NULL;
    response->wstop__TopicSet->__size =1;// gOnvifEventTopicSet.size;
    response->wstop__TopicSet->__any = (char**)soap_malloc(soap, sizeof(char*)*1);
    response->wstop__TopicSet->__anyAttribute = NULL;
    int i = 0;

    for (i = 0; i < 1; i++) {
        response->wstop__TopicSet->__any[i] = soap_strdup(soap, gOnvifEventTopics[i]);
    }

    response->__sizeTopicExpressionDialect = gOnvifEventTED.size;
    response->wsnt__TopicExpressionDialect = gOnvifEventTED.expressions;
    response->__sizeMessageContentFilterDialect = gOnvifEventMCFD.size;
    response->MessageContentFilterDialect = gOnvifEventMCFD.filters;
    response->__sizeProducerPropertiesFilterDialect = 0;
    response->ProducerPropertiesFilterDialect = NULL;
    response->__sizeMessageContentSchemaLocation = gOnvifEventMCSL.size;
    response->MessageContentSchemaLocation = gOnvifEventMCSL.schemas;
    response->__size = 0;
    response->__any = NULL;

    if (soap->header)
    {
        soap_default_SOAP_ENV__Header(soap, soap->header);
        soap->header->wsa5__Action = (char*)ONVIF_EVENT_GET_ACTION;
    }
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__AddEventBroker(struct soap *soap,
                                                struct _tev__AddEventBroker
                                                    *tev__AddEventBroker,
                                                struct _tev__AddEventBrokerResponse
                                                    *tev__AddEventBrokerResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__DeleteEventBroker(struct soap *soap,
                                                   struct _tev__DeleteEventBroker
                                                       *tev__DeleteEventBroker,
                                                   struct _tev__DeleteEventBrokerResponse
                                                       *tev__DeleteEventBrokerResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetEventBrokers(struct soap *soap,
                                                 struct _tev__GetEventBrokers
                                                     *tev__GetEventBrokers,
                                                 struct _tev__GetEventBrokersResponse
                                                     *tev__GetEventBrokersResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew(struct soap *soap,
                                       struct _wsnt__Renew
                                           *tev__Renew,
                                       struct _wsnt__RenewResponse
                                           *tev__RenewResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe_(struct soap *soap,
                                              struct _wsnt__Unsubscribe
                                                  *wsnt__Unsubscribe,
                                              struct _wsnt__UnsubscribeResponse
                                                  *wsnt__UnsubscribeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Subscribe(struct soap *soap,
                                           struct _wsnt__Subscribe
                                               *tev__Subscribe,
                                           struct _wsnt__SubscribeResponse
                                               *tev__SubscribeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    log_com("FIXME: add event subscribe");
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetCurrentMessage(struct soap *soap,
                                                   struct _wsnt__GetCurrentMessage
                                                       *tev__GetCurrentMessage,
                                                   struct _wsnt__GetCurrentMessageResponse
                                                       *tev__GetCurrentMessageResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify(struct soap *soap,
                                        struct _wsnt__Notify *wsnt__Notify)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetMessages(struct soap *soap,
                                             struct _wsnt__GetMessages
                                                 *tev__GetMessages,
                                             struct _wsnt__GetMessagesResponse
                                                 *tev__GetMessagesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__DestroyPullPoint(struct soap *soap,
                                                  struct _wsnt__DestroyPullPoint
                                                      *tev__DestroyPullPoint,
                                                  struct _wsnt__DestroyPullPointResponse
                                                      *tev__DestroyPullPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify_(struct soap *soap,
                                         struct _wsnt__Notify *wsnt__Notify)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPoint(struct soap *soap,
                                                 struct _wsnt__CreatePullPoint
                                                     *wsnt__CreatePullPoint,
                                                 struct _wsnt__CreatePullPointResponse
                                                     *wsnt__CreatePullPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew_(struct soap *soap,
                                        struct _wsnt__Renew *wsnt__Renew,
                                        struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe__(struct soap *soap,
                                               struct _wsnt__Unsubscribe
                                                   *wsnt__Unsubscribe,
                                               struct _wsnt__UnsubscribeResponse
                                                   *wsnt__UnsubscribeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PauseSubscription(struct soap *soap,
                                                   struct _wsnt__PauseSubscription
                                                       *wsnt__PauseSubscription,
                                                   struct _wsnt__PauseSubscriptionResponse
                                                       *wsnt__PauseSubscriptionResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__ResumeSubscription(struct soap *soap,
                                                    struct _wsnt__ResumeSubscription
                                                        *wsnt__ResumeSubscription,
                                                    struct _wsnt__ResumeSubscriptionResponse
                                                        *wsnt__ResumeSubscriptionResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetServiceCapabilities(struct soap *soap,
                                                         struct _timg__GetServiceCapabilities
                                                             *timg__GetServiceCapabilities,
                                                         struct _timg__GetServiceCapabilitiesResponse
                                                             *timg__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetImagingSettings(struct soap *soap,
                                                     struct _timg__GetImagingSettings
                                                         *timg__GetImagingSettings,
                                                     struct _timg__GetImagingSettingsResponse
                                                         *timg__GetImagingSettingsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__SetImagingSettings(struct soap *soap,
                                                     struct _timg__SetImagingSettings
                                                         *timg__SetImagingSettings,
                                                     struct _timg__SetImagingSettingsResponse
                                                         *timg__SetImagingSettingsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap *soap,
                                             struct _timg__GetOptions
                                                 *timg__GetOptions,
                                             struct _timg__GetOptionsResponse
                                                 *timg__GetOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap *soap,
                                       struct _timg__Move
                                           *timg__Move,
                                       struct _timg__MoveResponse
                                           *timg__MoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap *soap,
                                       struct _timg__Stop
                                           *timg__Stop,
                                       struct _timg__StopResponse
                                           *timg__StopResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap *soap,
                                            struct _timg__GetStatus
                                                *timg__GetStatus,
                                            struct _timg__GetStatusResponse
                                                *timg__GetStatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap *soap,
                                                 struct _timg__GetMoveOptions
                                                     *timg__GetMoveOptions,
                                                 struct _timg__GetMoveOptionsResponse
                                                     *timg__GetMoveOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetPresets(struct soap *soap,
                                             struct _timg__GetPresets
                                                 *timg__GetPresets,
                                             struct _timg__GetPresetsResponse
                                                 *timg__GetPresetsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetCurrentPreset(struct soap *soap,
                                                   struct _timg__GetCurrentPreset
                                                       *timg__GetCurrentPreset,
                                                   struct _timg__GetCurrentPresetResponse
                                                       *timg__GetCurrentPresetResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__SetCurrentPreset(struct soap *soap,
                                                   struct _timg__SetCurrentPreset
                                                       *timg__SetCurrentPreset,
                                                   struct _timg__SetCurrentPresetResponse
                                                       *timg__SetCurrentPresetResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetServiceCapabilities(struct soap *soap,
                                                        struct _tls__GetServiceCapabilities
                                                            *tls__GetServiceCapabilities,
                                                        struct _tls__GetServiceCapabilitiesResponse
                                                            *tls__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetLayout(struct soap *soap,
                                           struct _tls__GetLayout
                                               *tls__GetLayout,
                                           struct _tls__GetLayoutResponse
                                               *tls__GetLayoutResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__SetLayout(struct soap *soap,
                                           struct _tls__SetLayout
                                               *tls__SetLayout,
                                           struct _tls__SetLayoutResponse
                                               *tls__SetLayoutResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetDisplayOptions(struct soap *soap,
                                                   struct _tls__GetDisplayOptions
                                                       *tls__GetDisplayOptions,
                                                   struct _tls__GetDisplayOptionsResponse
                                                       *tls__GetDisplayOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetPaneConfigurations(struct soap *soap,
                                                       struct _tls__GetPaneConfigurations
                                                           *tls__GetPaneConfigurations,
                                                       struct _tls__GetPaneConfigurationsResponse
                                                           *tls__GetPaneConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetPaneConfiguration(struct soap *soap,
                                                      struct _tls__GetPaneConfiguration
                                                          *tls__GetPaneConfiguration,
                                                      struct _tls__GetPaneConfigurationResponse
                                                          *tls__GetPaneConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__SetPaneConfigurations(struct soap *soap,
                                                       struct _tls__SetPaneConfigurations
                                                           *tls__SetPaneConfigurations,
                                                       struct _tls__SetPaneConfigurationsResponse
                                                           *tls__SetPaneConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__SetPaneConfiguration(struct soap *soap,
                                                      struct _tls__SetPaneConfiguration
                                                          *tls__SetPaneConfiguration,
                                                      struct _tls__SetPaneConfigurationResponse
                                                          *tls__SetPaneConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__CreatePaneConfiguration(struct soap *soap,
                                                         struct _tls__CreatePaneConfiguration
                                                             *tls__CreatePaneConfiguration,
                                                         struct _tls__CreatePaneConfigurationResponse
                                                             *tls__CreatePaneConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tls__DeletePaneConfiguration(struct soap *soap,
                                                         struct _tls__DeletePaneConfiguration
                                                             *tls__DeletePaneConfiguration,
                                                         struct _tls__DeletePaneConfigurationResponse
                                                             *tls__DeletePaneConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetServiceCapabilities(struct soap *soap,
                                                        struct _tmd__GetServiceCapabilities
                                                            *tmd__GetServiceCapabilities,
                                                        struct _tmd__GetServiceCapabilitiesResponse
                                                            *tmd__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputOptions(struct soap *soap,
                                                       struct _tmd__GetRelayOutputOptions
                                                           *tmd__GetRelayOutputOptions,
                                                       struct _tmd__GetRelayOutputOptionsResponse
                                                           *tmd__GetRelayOutputOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSources(struct soap *soap,
                                                 struct tmd__Get
                                                     *tmd__GetAudioSources,
                                                 struct tmd__GetResponse
                                                     *tmd__GetAudioSourcesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputs(struct soap *soap,
                                                 struct tmd__Get *tmd__GetAudioOutputs,
                                                 struct tmd__GetResponse *tmd__GetAudioOutputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSources(struct soap *soap,
                                                 struct tmd__Get
                                                     *tmd__GetVideoSources,
                                                 struct tmd__GetResponse
                                                     *tmd__GetVideoSourcesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputs(struct soap *soap,
                                                 struct _tmd__GetVideoOutputs *tmd__GetVideoOutputs,
                                                 struct _tmd__GetVideoOutputsResponse *tmd__GetVideoOutputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfiguration(struct soap *soap,
                                                             struct _tmd__GetVideoSourceConfiguration
                                                                 *tmd__GetVideoSourceConfiguration,
                                                             struct _tmd__GetVideoSourceConfigurationResponse
                                                                 *tmd__GetVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfiguration(struct soap *soap,
                                                             struct _tmd__GetVideoOutputConfiguration
                                                                 *tmd__GetVideoOutputConfiguration,
                                                             struct _tmd__GetVideoOutputConfigurationResponse
                                                                 *tmd__GetVideoOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfiguration(struct soap *soap,
                                                             struct _tmd__GetAudioSourceConfiguration
                                                                 *tmd__GetAudioSourceConfiguration,
                                                             struct _tmd__GetAudioSourceConfigurationResponse
                                                                 *tmd__GetAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfiguration(struct soap *soap,
                                                             struct _tmd__GetAudioOutputConfiguration
                                                                 *tmd__GetAudioOutputConfiguration,
                                                             struct _tmd__GetAudioOutputConfigurationResponse
                                                                 *tmd__GetAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoSourceConfiguration(struct soap *soap,
                                                             struct _tmd__SetVideoSourceConfiguration
                                                                 *tmd__SetVideoSourceConfiguration,
                                                             struct _tmd__SetVideoSourceConfigurationResponse
                                                                 *tmd__SetVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoOutputConfiguration(struct soap *soap,
                                                             struct _tmd__SetVideoOutputConfiguration
                                                                 *tmd__SetVideoOutputConfiguration,
                                                             struct _tmd__SetVideoOutputConfigurationResponse
                                                                 *tmd__SetVideoOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioSourceConfiguration(struct soap *soap,
                                                             struct _tmd__SetAudioSourceConfiguration
                                                                 *tmd__SetAudioSourceConfiguration,
                                                             struct _tmd__SetAudioSourceConfigurationResponse
                                                                 *tmd__SetAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioOutputConfiguration(struct soap *soap,
                                                             struct _tmd__SetAudioOutputConfiguration
                                                                 *tmd__SetAudioOutputConfiguration,
                                                             struct _tmd__SetAudioOutputConfigurationResponse
                                                                 *tmd__SetAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfigurationOptions(struct soap *soap,
                                                                    struct _tmd__GetVideoSourceConfigurationOptions
                                                                        *tmd__GetVideoSourceConfigurationOptions,
                                                                    struct _tmd__GetVideoSourceConfigurationOptionsResponse
                                                                        *tmd__GetVideoSourceConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfigurationOptions(struct soap *soap,
                                                                    struct _tmd__GetVideoOutputConfigurationOptions
                                                                        *tmd__GetVideoOutputConfigurationOptions,
                                                                    struct _tmd__GetVideoOutputConfigurationOptionsResponse
                                                                        *tmd__GetVideoOutputConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfigurationOptions(struct soap *soap,
                                                                    struct _tmd__GetAudioSourceConfigurationOptions
                                                                        *tmd__GetAudioSourceConfigurationOptions,
                                                                    struct _tmd__GetAudioSourceConfigurationOptionsResponse
                                                                        *tmd__GetAudioSourceConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfigurationOptions(struct soap *soap,
                                                                    struct _tmd__GetAudioOutputConfigurationOptions
                                                                        *tmd__GetAudioOutputConfigurationOptions,
                                                                    struct _tmd__GetAudioOutputConfigurationOptionsResponse
                                                                        *tmd__GetAudioOutputConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputs(struct soap *soap,
                                                 struct _tds__GetRelayOutputs
                                                     *tds__GetRelayOutputs,
                                                 struct _tds__GetRelayOutputsResponse
                                                     *tds__GetRelayOutputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputSettings(struct soap *soap,
                                                        struct _tmd__SetRelayOutputSettings
                                                            *tmd__SetRelayOutputSettings,
                                                        struct _tmd__SetRelayOutputSettingsResponse
                                                            *tmd__SetRelayOutputSettingsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputState(struct soap *soap,
                                                     struct _tds__SetRelayOutputState
                                                         *tds__SetRelayOutputState,
                                                     struct _tds__SetRelayOutputStateResponse
                                                         *tds__SetRelayOutputStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetDigitalInputs(struct soap *soap,
                                                  struct _tmd__GetDigitalInputs
                                                      *tmd__GetDigitalInputs,
                                                  struct _tmd__GetDigitalInputsResponse
                                                      *tmd__GetDigitalInputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetDigitalInputConfigurationOptions(struct soap *soap,
                                                                     struct _tmd__GetDigitalInputConfigurationOptions
                                                                         *tmd__GetDigitalInputConfigurationOptions,
                                                                     struct _tmd__GetDigitalInputConfigurationOptionsResponse
                                                                         *tmd__GetDigitalInputConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetDigitalInputConfigurations(struct soap *soap,
                                                               struct _tmd__SetDigitalInputConfigurations
                                                                   *tmd__SetDigitalInputConfigurations,
                                                               struct _tmd__SetDigitalInputConfigurationsResponse
                                                                   *tmd__SetDigitalInputConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPorts(struct soap *soap,
                                                struct _tmd__GetSerialPorts
                                                    *tmd__GetSerialPorts,
                                                struct _tmd__GetSerialPortsResponse
                                                    *tmd__GetSerialPortsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfiguration(struct soap *soap,
                                                            struct _tmd__GetSerialPortConfiguration
                                                                *tmd__GetSerialPortConfiguration,
                                                            struct _tmd__GetSerialPortConfigurationResponse
                                                                *tmd__GetSerialPortConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetSerialPortConfiguration(struct soap *soap,
                                                            struct _tmd__SetSerialPortConfiguration
                                                                *tmd__SetSerialPortConfiguration,
                                                            struct _tmd__SetSerialPortConfigurationResponse
                                                                *tmd__SetSerialPortConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfigurationOptions(struct soap *soap,
                                                                   struct _tmd__GetSerialPortConfigurationOptions
                                                                       *tmd__GetSerialPortConfigurationOptions,
                                                                   struct _tmd__GetSerialPortConfigurationOptionsResponse
                                                                       *tmd__GetSerialPortConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SendReceiveSerialCommand(struct soap *soap,
                                                          struct _tmd__SendReceiveSerialCommand
                                                              *tmd__SendReceiveSerialCommand,
                                                          struct _tmd__SendReceiveSerialCommandResponse
                                                              *tmd__SendReceiveSerialCommandResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetServiceCapabilities(struct soap *soap,
                                                         struct _tptz__GetServiceCapabilities
                                                             *tptz__GetServiceCapabilities,
                                                         struct _tptz__GetServiceCapabilitiesResponse
                                                             *tptz__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurations(struct soap *soap,
                                                    struct _tptz__GetConfigurations
                                                        *tptz___GetConfigurations,
                                                    struct _tptz__GetConfigurationsResponse
                                                        *tptz___GetConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresets(struct soap *soap,
                                             struct _tptz__GetPresets
                                                 *tptz__GetPresets,
                                             struct _tptz__GetPresetsResponse
                                                 *tptz__GetPresetsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetPreset(struct soap *soap,
                                            struct _tptz__SetPreset
                                                *tptz__SetPreset,
                                            struct _tptz__SetPresetResponse
                                                *tptz__SetPresetResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePreset(struct soap *soap,
                                               struct _tptz__RemovePreset
                                                   *tptz__RemovePreset,
                                               struct _tptz__RemovePresetResponse
                                                   *tptz__RemovePresetResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoPreset(struct soap *soap,
                                             struct _tptz__GotoPreset
                                                 *tptz__GotoPreset,
                                             struct _tptz__GotoPresetResponse
                                                 *tptz__GotoPresetResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetStatus(struct soap *soap,
                                            struct _tptz__GetStatus
                                                *tptz__GetStatus,
                                            struct _tptz__GetStatusResponse
                                                *tptz__GetStatusResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfiguration(struct soap *soap,
                                                   struct _tptz__GetConfiguration
                                                       *tptz__GetConfiguration,
                                                   struct _tptz__GetConfigurationResponse
                                                       *tptz__GetConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNodes(struct soap *soap,
                                           struct _tptz__GetNodes
                                               *tptz__GetNodes,
                                           struct _tptz__GetNodesResponse
                                               *tptz__GetNodesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNode(struct soap *soap,
                                          struct _tptz__GetNode
                                              *tptz__GetNode,
                                          struct _tptz__GetNodeResponse
                                              *tptz__GetNodeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetConfiguration(struct soap *soap,
                                                   struct _tptz__SetConfiguration *tptz__SetConfiguration,
                                                   struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurationOptions(struct soap *soap,
                                                          struct _tptz__GetConfigurationOptions
                                                              *tptz__GetConfigurationOptions,
                                                          struct _tptz__GetConfigurationOptionsResponse
                                                              *tptz__GetConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoHomePosition(struct soap *soap,
                                                   struct _tptz__GotoHomePosition
                                                       *tptz__GotoHomePosition,
                                                   struct _tptz__GotoHomePositionResponse
                                                       *tptz__GotoHomePositionResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetHomePosition(struct soap *soap,
                                                  struct _tptz__SetHomePosition
                                                      *tptz__SetHomePosition,
                                                  struct _tptz__SetHomePositionResponse
                                                      *tptz__SetHomePositionResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ContinuousMove(struct soap *soap,
                                                 struct _tptz__ContinuousMove
                                                     *tptz__ContinuousMove,
                                                 struct _tptz__ContinuousMoveResponse
                                                     *tptz__ContinuousMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RelativeMove(struct soap *soap,
                                               struct _tptz__RelativeMove
                                                   *tptz__RelativeMove,
                                               struct _tptz__RelativeMoveResponse
                                                   *tptz__RelativeMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SendAuxiliaryCommand(struct soap *soap,
                                                       struct _tptz__SendAuxiliaryCommand
                                                           *tptz__SendAuxiliaryCommand,
                                                       struct _tptz__SendAuxiliaryCommandResponse
                                                           *tptz__SendAuxiliaryCommandResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__AbsoluteMove(struct soap *soap,
                                               struct _tptz__AbsoluteMove
                                                   *tptz__AbsoluteMove,
                                               struct _tptz__AbsoluteMoveResponse
                                                   *tptz__AbsoluteMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GeoMove(struct soap *soap,
                                          struct _tptz__GeoMove
                                              *tptz__GeoMove,
                                          struct _tptz__GeoMoveResponse
                                              *tptz__GeoMoveResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__Stop(struct soap *soap,
                                       struct _tptz__Stop
                                           *tptz__Stop,
                                       struct _tptz__StopResponse
                                           *tptz__StopResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTours(struct soap *soap,
                                                 struct _tptz__GetPresetTours
                                                     *tptz__GetPresetTours,
                                                 struct _tptz__GetPresetToursResponse
                                                     *tptz__GetPresetToursResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTour(struct soap *soap,
                                                struct _tptz__GetPresetTour
                                                    *tptz__GetPresetTour,
                                                struct _tptz__GetPresetTourResponse
                                                    *tptz__GetPresetTourResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTourOptions(struct soap *soap,
                                                       struct _tptz__GetPresetTourOptions
                                                           *tptz__GetPresetTourOptions,
                                                       struct _tptz__GetPresetTourOptionsResponse
                                                           *tptz__GetPresetTourOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__CreatePresetTour(struct soap *soap,
                                                   struct _tptz__CreatePresetTour
                                                       *tptz__CreatePresetTour,
                                                   struct _tptz__CreatePresetTourResponse
                                                       *tptz__CreatePresetTourResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ModifyPresetTour(struct soap *soap,
                                                   struct _tptz__ModifyPresetTour
                                                       *tptz__ModifyPresetTour,
                                                   struct _tptz__ModifyPresetTourResponse
                                                       *tptz__ModifyPresetTourResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__OperatePresetTour(struct soap *soap,
                                                    struct _tptz__OperatePresetTour
                                                        *tptz__OperatePresetTour,
                                                    struct _tptz__OperatePresetTourResponse
                                                        *tptz__OperatePresetTourResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePresetTour(struct soap *soap,
                                                   struct _tptz__RemovePresetTour
                                                       *tptz__RemovePresetTour,
                                                   struct _tptz__RemovePresetTourResponse
                                                       *tptz__RemovePresetTourResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetCompatibleConfigurations(struct soap *soap,
                                                              struct _tptz__GetCompatibleConfigurations
                                                                  *tptz__GetCompatibleConfigurations,
                                                              struct _tptz__GetCompatibleConfigurationsResponse
                                                                  *tptz__GetCompatibleConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetServiceCapabilities(struct soap *soap,
                                                        struct _trc__GetServiceCapabilities
                                                            *trc__GetServiceCapabilities,
                                                        struct _trc__GetServiceCapabilitiesResponse
                                                            *trc__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecording(struct soap *soap,
                                                 struct _trc__CreateRecording
                                                     *trc__CreateRecording,
                                                 struct _trc__CreateRecordingResponse
                                                     *trc__CreateRecordingResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecording(struct soap *soap,
                                                 struct _trc__DeleteRecording
                                                     *trc__DeleteRecording,
                                                 struct _trc__DeleteRecordingResponse
                                                     *trc__DeleteRecordingResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordings(struct soap *soap,
                                               struct _trc__GetRecordings
                                                   *trc__GetRecordings,
                                               struct _trc__GetRecordingsResponse
                                                   *trc__GetRecordingsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingConfiguration(struct soap *soap,
                                                           struct _trc__SetRecordingConfiguration
                                                               *trc__SetRecordingConfiguration,
                                                           struct _trc__SetRecordingConfigurationResponse
                                                               *trc__SetRecordingConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingConfiguration(struct soap *soap,
                                                           struct _trc__GetRecordingConfiguration
                                                               *trc__GetRecordingConfiguration,
                                                           struct _trc__GetRecordingConfigurationResponse
                                                               *trc__GetRecordingConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingOptions(struct soap *soap,
                                                     struct _trc__GetRecordingOptions
                                                         *trc__GetRecordingOptions,
                                                     struct _trc__GetRecordingOptionsResponse
                                                         *trc__GetRecordingOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateTrack(struct soap *soap,
                                             struct _trc__CreateTrack
                                                 *trc__CreateTrack,
                                             struct _trc__CreateTrackResponse
                                                 *trc__CreateTrackResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteTrack(struct soap *soap,
                                             struct _trc__DeleteTrack
                                                 *trc__DeleteTrack,
                                             struct _trc__DeleteTrackResponse
                                                 *trc__DeleteTrackResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetTrackConfiguration(struct soap *soap,
                                                       struct _trc__GetTrackConfiguration
                                                           *trc__GetTrackConfiguration,
                                                       struct _trc__GetTrackConfigurationResponse
                                                           *trc__GetTrackConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetTrackConfiguration(struct soap *soap,
                                                       struct _trc__SetTrackConfiguration
                                                           *trc__SetTrackConfiguration,
                                                       struct _trc__SetTrackConfigurationResponse
                                                           *trc__SetTrackConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecordingJob(struct soap *soap,
                                                    struct _trc__CreateRecordingJob
                                                        *trc__CreateRecordingJob,
                                                    struct _trc__CreateRecordingJobResponse
                                                        *trc__CreateRecordingJobResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecordingJob(struct soap *soap,
                                                    struct _trc__DeleteRecordingJob
                                                        *trc__DeleteRecordingJob,
                                                    struct _trc__DeleteRecordingJobResponse
                                                        *trc__DeleteRecordingJobResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobs(struct soap *soap,
                                                  struct _trc__GetRecordingJobs
                                                      *trc__GetRecordingJobs,
                                                  struct _trc__GetRecordingJobsResponse
                                                      *trc__GetRecordingJobsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobConfiguration(struct soap *soap,
                                                              struct _trc__SetRecordingJobConfiguration
                                                                  *trc__SetRecordingJobConfiguration,
                                                              struct _trc__SetRecordingJobConfigurationResponse
                                                                  *trc__SetRecordingJobConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobConfiguration(struct soap *soap,
                                                              struct _trc__GetRecordingJobConfiguration
                                                                  *trc__GetRecordingJobConfiguration,
                                                              struct _trc__GetRecordingJobConfigurationResponse
                                                                  *trc__GetRecordingJobConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobMode(struct soap *soap,
                                                     struct _trc__SetRecordingJobMode
                                                         *trc__SetRecordingJobMode,
                                                     struct _trc__SetRecordingJobModeResponse
                                                         *trc__SetRecordingJobModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobState(struct soap *soap,
                                                      struct _trc__GetRecordingJobState
                                                          *trc__GetRecordingJobState,
                                                      struct _trc__GetRecordingJobStateResponse
                                                          *trc__GetRecordingJobStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__ExportRecordedData(struct soap *soap,
                                                    struct _trc__ExportRecordedData
                                                        *trc__ExportRecordedData,
                                                    struct _trc__ExportRecordedDataResponse
                                                        *trc__ExportRecordedDataResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__StopExportRecordedData(struct soap *soap,
                                                        struct _trc__StopExportRecordedData
                                                            *trc__StopExportRecordedData,
                                                        struct _trc__StopExportRecordedDataResponse
                                                            *trc__StopExportRecordedDataResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetExportRecordedDataState(struct soap *soap,
                                                            struct _trc__GetExportRecordedDataState
                                                                *trc__GetExportRecordedDataState,
                                                            struct _trc__GetExportRecordedDataStateResponse
                                                                *trc__GetExportRecordedDataStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetServiceCapabilities(struct soap *soap,
                                                        struct _trp__GetServiceCapabilities
                                                            *trp__GetServiceCapabilities,
                                                        struct _trp__GetServiceCapabilitiesResponse
                                                            *trp__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayUri(struct soap *soap,
                                              struct _trp__GetReplayUri
                                                  *trp__GetReplayUri,
                                              struct _trp__GetReplayUriResponse
                                                  *trp__GetReplayUriResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayConfiguration(struct soap *soap,
                                                        struct _trp__GetReplayConfiguration
                                                            *trp__GetReplayConfiguration,
                                                        struct _trp__GetReplayConfigurationResponse
                                                            *trp__GetReplayConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__SetReplayConfiguration(struct soap *soap,
                                                        struct _trp__SetReplayConfiguration
                                                            *trp__SetReplayConfiguration,
                                                        struct _trp__SetReplayConfigurationResponse
                                                            *trp__SetReplayConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap *soap,
                                                        struct _trt__GetServiceCapabilities
                                                            *trt__GetServiceCapabilities,
                                                        struct _trt__GetServiceCapabilitiesResponse
                                                            *trt__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap *soap,
                                                 struct _trt__GetVideoSources
                                                     *trt__GetVideoSources,
                                                 struct _trt__GetVideoSourcesResponse
                                                     *trt__GetVideoSourcesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN; 

    int size = 1;
    struct tt__VideoSource *VideoSources = NULL;

    trt__GetVideoSourcesResponse->__sizeVideoSources = size;
    ALLOC_STRUCT_NUM(VideoSources, struct tt__VideoSource, size);
    VideoSources->token = soap_strdup(soap, gOnvifVideoSrcCfg.token);
    VideoSources->Framerate = gOnvifVideoSrcCfg.framerate;
    ALLOC_STRUCT(VideoSources->Resolution, struct tt__VideoResolution);
    VideoSources->Resolution->Width = gOnvifVideoSrcCfg.resolution.Width;
    VideoSources->Resolution->Height = gOnvifVideoSrcCfg.resolution.Width;
    
    trt__GetVideoSourcesResponse->VideoSources = VideoSources;

    log_com("src token %s framerate %f w %d h %d", trt__GetVideoSourcesResponse->VideoSources->token, 
        trt__GetVideoSourcesResponse->VideoSources->Framerate, 
        trt__GetVideoSourcesResponse->VideoSources->Resolution->Width,
        trt__GetVideoSourcesResponse->VideoSources->Resolution->Height);

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap *soap,
                                                 struct _trt__GetAudioSources
                                                     *trc__GetAudioSources,
                                                 struct _trt__GetAudioSourcesResponse
                                                     *trc__GetAudioSourcesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap *soap,
                                                 struct _trt__GetAudioOutputs
                                                     *trt__GetAudioOutputs,
                                                 struct _trt__GetAudioOutputsResponse
                                                     *trt__GetAudioOutputsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap,
                                               struct _trt__CreateProfile *trt__CreateProfile,
                                               struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap *soap,
                                            struct _trt__GetProfile
                                                *trt__GetProfile,
                                            struct _trt__GetProfileResponse
                                                *trt__GetProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    log_war("get profile token: %s", trt__GetProfile->ProfileToken);

    // TODO: Sam. response the profile , according to profile token
    struct tt__Profile* Profile;
    ALLOC_STRUCT(Profile, struct tt__Profile);
    struct tt__VideoSourceConfiguration *VideoSourceConfiguration = NULL;
    struct tt__VideoEncoderConfiguration *VideoEncoderConfiguration = NULL;

    ALLOC_STRUCT(VideoSourceConfiguration,  struct tt__VideoSourceConfiguration);
    VideoSourceConfiguration->Name = soap_strdup(soap, gOnvifVideoSrcCfg.name);
    VideoSourceConfiguration->UseCount = gOnvifVideoSrcCfg.useCount;
    VideoSourceConfiguration->token = soap_strdup(soap, gOnvifVideoSrcCfg.token);
    VideoSourceConfiguration->SourceToken = soap_strdup(soap, gOnvifVideoSrcCfg.sourceToken);
    ALLOC_STRUCT(VideoSourceConfiguration->Bounds, struct tt__IntRectangle);
    VideoSourceConfiguration->Bounds->width = gOnvifVideoSrcCfg.bounds.width;
    VideoSourceConfiguration->Bounds->height = gOnvifVideoSrcCfg.bounds.height;
    VideoSourceConfiguration->Bounds->x = gOnvifVideoSrcCfg.bounds.x;
    VideoSourceConfiguration->Bounds->y = gOnvifVideoSrcCfg.bounds.y;

    log_com("vs name %s usecount %d token %s srctoken %s",
    VideoSourceConfiguration->Name, VideoSourceConfiguration->UseCount, 
    VideoSourceConfiguration->token, VideoSourceConfiguration->SourceToken);
    // // audio source
    // ALLOC_STRUCT(AudioSourceConfiguration,  struct tt__AudioSourceConfiguration);
    // video encode
    log_com();
    ALLOC_STRUCT(VideoEncoderConfiguration,  struct tt__VideoEncoderConfiguration);
    VideoEncoderConfiguration->Name = soap_strdup(soap, gOnvifVideoEncCfg.name);
    VideoEncoderConfiguration->UseCount = gOnvifVideoEncCfg.useCount;
    VideoEncoderConfiguration->token = soap_strdup(soap, gOnvifVideoEncCfg.token);
    VideoEncoderConfiguration->Encoding = (enum tt__VideoEncoding)gOnvifVideoEncCfg.encoding;
    ALLOC_STRUCT(VideoEncoderConfiguration->Resolution, struct tt__VideoResolution);
    VideoEncoderConfiguration->Resolution->Width = gOnvifVideoEncCfg.resolution.Width;
    VideoEncoderConfiguration->Resolution->Height = gOnvifVideoEncCfg.resolution.Height;
    VideoEncoderConfiguration->Quality = gOnvifVideoEncCfg.quality;
    ALLOC_STRUCT(VideoEncoderConfiguration->RateControl, struct tt__VideoRateControl);
    VideoEncoderConfiguration->RateControl->FrameRateLimit = gOnvifVideoEncCfg.frameRateLimit;
    VideoEncoderConfiguration->RateControl->EncodingInterval = gOnvifVideoEncCfg.gop;
    VideoEncoderConfiguration->RateControl->BitrateLimit = gOnvifVideoEncCfg.bitrate;
    ALLOC_STRUCT(VideoEncoderConfiguration->H264, struct tt__H264Configuration);
    VideoEncoderConfiguration->H264->GovLength = gOnvifVideoEncCfg.h264.govLength;
    VideoEncoderConfiguration->H264->H264Profile = (enum tt__H264Profile)gOnvifVideoEncCfg.h264.profile;
    ALLOC_STRUCT(VideoEncoderConfiguration->Multicast, struct tt__MulticastConfiguration);
    ALLOC_STRUCT(VideoEncoderConfiguration->Multicast->Address, struct tt__IPAddress);
    VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
    VideoEncoderConfiguration->Multicast->Address->IPv4Address = soap_strdup(soap, "224.1.0.0");
    VideoEncoderConfiguration->Multicast->Address->IPv6Address = NULL;
    VideoEncoderConfiguration->Multicast->Port = ONVIF_MULTICAST_PORT; // 8082
    VideoEncoderConfiguration->Multicast->TTL = 1500;
    VideoEncoderConfiguration->Multicast->AutoStart = xsd__boolean__true_;
    VideoEncoderConfiguration->SessionTimeout = 5000;

    Profile->VideoEncoderConfiguration = VideoEncoderConfiguration;
    Profile->VideoSourceConfiguration = VideoSourceConfiguration;

    trt__GetProfileResponse->Profile = Profile;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap *soap,
                                             struct _trt__GetProfiles *trt__GetProfiles,
                                             struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    // TODO: set 1 temporarily.
    int i = 0;
    trt__GetProfilesResponse->__sizeProfiles = DEVICE_PROFILE_NUM;
    struct tt__Profile* Profiles;
    ALLOC_STRUCT_NUM(Profiles, struct tt__Profile, DEVICE_PROFILE_NUM);

    struct tt__VideoSourceConfiguration *VideoSourceConfiguration = NULL;
    struct tt__AudioSourceConfiguration *AudioSourceConfiguration = NULL;
    struct tt__VideoEncoderConfiguration *VideoEncoderConfiguration = NULL;
    struct tt__AudioEncoderConfiguration *AudioEncoderConfiguration = NULL;
    struct tt__VideoAnalyticsConfiguration *VideoAnalyticsConfiguration = NULL;
    struct tt__PTZConfiguration *PTZConfiguration = NULL;
    struct tt__MetadataConfiguration *MetadataConfiguration = NULL;
    struct tt__ProfileExtension *Extension = NULL;

    for(i = 0; i < DEVICE_PROFILE_NUM; i++)
    {

        // video source
        ALLOC_STRUCT(VideoSourceConfiguration,  struct tt__VideoSourceConfiguration);
        VideoSourceConfiguration->Name = soap_strdup(soap, gOnvifVideoSrcCfg.name);
        VideoSourceConfiguration->UseCount = gOnvifVideoSrcCfg.useCount;
        VideoSourceConfiguration->token = soap_strdup(soap, gOnvifVideoSrcCfg.token);
        VideoSourceConfiguration->SourceToken = soap_strdup(soap, gOnvifVideoSrcCfg.sourceToken);
        ALLOC_STRUCT(VideoSourceConfiguration->Bounds, struct tt__IntRectangle);
        VideoSourceConfiguration->Bounds->width = gOnvifVideoSrcCfg.bounds.width;
        VideoSourceConfiguration->Bounds->height = gOnvifVideoSrcCfg.bounds.height;
        VideoSourceConfiguration->Bounds->x = gOnvifVideoSrcCfg.bounds.x;
        VideoSourceConfiguration->Bounds->y = gOnvifVideoSrcCfg.bounds.y;

        log_com("vs name %s usecount %d token %s srctoken %s",
            VideoSourceConfiguration->Name, VideoSourceConfiguration->UseCount, 
            VideoSourceConfiguration->token, VideoSourceConfiguration->SourceToken);
        // // audio source
        // ALLOC_STRUCT(AudioSourceConfiguration,  struct tt__AudioSourceConfiguration);
        
        // video encode
        ALLOC_STRUCT(VideoEncoderConfiguration,  struct tt__VideoEncoderConfiguration);
        VideoEncoderConfiguration->Name = soap_strdup(soap, gOnvifVideoEncCfg.name);
        VideoEncoderConfiguration->UseCount = gOnvifVideoEncCfg.useCount;
        VideoEncoderConfiguration->token = soap_strdup(soap, gOnvifVideoEncCfg.token);
        VideoEncoderConfiguration->Encoding = (enum tt__VideoEncoding)gOnvifVideoEncCfg.encoding;
        ALLOC_STRUCT(VideoEncoderConfiguration->Resolution, struct tt__VideoResolution);
        VideoEncoderConfiguration->Resolution->Width = gOnvifVideoEncCfg.resolution.Width;
        VideoEncoderConfiguration->Resolution->Height = gOnvifVideoEncCfg.resolution.Height;
        VideoEncoderConfiguration->Quality = gOnvifVideoEncCfg.quality;
        ALLOC_STRUCT(VideoEncoderConfiguration->RateControl, struct tt__VideoRateControl);
        VideoEncoderConfiguration->RateControl->FrameRateLimit = gOnvifVideoEncCfg.frameRateLimit;
        VideoEncoderConfiguration->RateControl->EncodingInterval = gOnvifVideoEncCfg.gop;
        VideoEncoderConfiguration->RateControl->BitrateLimit = gOnvifVideoEncCfg.bitrate;
        ALLOC_STRUCT(VideoEncoderConfiguration->H264, struct tt__H264Configuration);
        VideoEncoderConfiguration->H264->GovLength = gOnvifVideoEncCfg.h264.govLength;
        VideoEncoderConfiguration->H264->H264Profile = (enum tt__H264Profile)gOnvifVideoEncCfg.h264.profile;
        ALLOC_STRUCT(VideoEncoderConfiguration->Multicast, struct tt__MulticastConfiguration);
        ALLOC_STRUCT(VideoEncoderConfiguration->Multicast->Address, struct tt__IPAddress);
        VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
        VideoEncoderConfiguration->Multicast->Address->IPv4Address = soap_strdup(soap, "224.1.0.0");
        VideoEncoderConfiguration->Multicast->Address->IPv6Address = NULL;
        VideoEncoderConfiguration->Multicast->Port = ONVIF_MULTICAST_PORT; // 8082
        VideoEncoderConfiguration->Multicast->TTL = 1500;
        VideoEncoderConfiguration->Multicast->AutoStart = xsd__boolean__false_; //xsd__boolean__false_; //xsd__boolean__true_;
        VideoEncoderConfiguration->SessionTimeout = 5000;
        // // audio encode 
        // ALLOC_STRUCT(AudioEncoderConfiguration,  struct tt__AudioEncoderConfiguration);
        // // video analytics
        // ALLOC_STRUCT(VideoAnalyticsConfiguration,  struct tt__VideoAnalyticsConfiguration);
        // // ptz 
        // ALLOC_STRUCT(PTZConfiguration,  struct tt__PTZConfiguration);
        // // metadata
        // ALLOC_STRUCT(MetadataConfiguration,  struct tt__MetadataConfiguration);
        // // extension
        // ALLOC_STRUCT(Extension,  struct tt__ProfileExtension);

        Profiles[i].Name = soap_strdup(soap, "Name_MediaProfiles");
        Profiles[i].VideoSourceConfiguration = VideoSourceConfiguration;
        Profiles[i].AudioSourceConfiguration = AudioSourceConfiguration;
        Profiles[i].VideoEncoderConfiguration = VideoEncoderConfiguration;
        Profiles[i].AudioEncoderConfiguration = AudioEncoderConfiguration;
        Profiles[i].VideoAnalyticsConfiguration = VideoAnalyticsConfiguration;
        Profiles[i].PTZConfiguration = PTZConfiguration;
        Profiles[i].MetadataConfiguration = MetadataConfiguration;
        Profiles[i].Extension = Extension;
        Profiles[i].token = soap_strdup(soap, "Token_MediaProfiles");
        Profiles[i].fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        *Profiles[i].fixed = xsd__boolean__true_; //xsd__boolean__true_;
        Profiles[i].__anyAttribute = NULL;
    }
    trt__GetProfilesResponse->Profiles = Profiles;

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap *soap,
                                                              struct _trt__AddVideoEncoderConfiguration
                                                                  *trt__AddVideoEncoderConfiguration,
                                                              struct _trt__AddVideoEncoderConfigurationResponse
                                                                  *trt__AddVideoEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap *soap,
                                                             struct _trt__AddVideoSourceConfiguration
                                                                 *trt__AddVideoSourceConfiguration,
                                                             struct _trt__AddVideoSourceConfigurationResponse
                                                                 *trt__AddVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap *soap,
                                                              struct _trt__AddAudioEncoderConfiguration
                                                                  *trt__AddAudioEncoderConfiguration,
                                                              struct _trt__AddAudioEncoderConfigurationResponse
                                                                  *trt__AddAudioEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap *soap,
                                                             struct _trt__AddAudioSourceConfiguration
                                                                 *trt__AddAudioSourceConfiguration,
                                                             struct _trt__AddAudioSourceConfigurationResponse
                                                                 *trt__AddAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap *soap,
                                                     struct _trt__AddPTZConfiguration
                                                         *trt__AddPTZConfiguration,
                                                     struct _trt__AddPTZConfigurationResponse
                                                         *trt__AddPTZConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap *soap,
                                                                struct _trt__AddVideoAnalyticsConfiguration
                                                                    *trt__AddVideoAnalyticsConfiguration,
                                                                struct _trt__AddVideoAnalyticsConfigurationResponse
                                                                    *trt__AddVideoAnalyticsConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap *soap,
                                                          struct _trt__AddMetadataConfiguration
                                                              *trt__AddMetadataConfiguration,
                                                          struct _trt__AddMetadataConfigurationResponse
                                                              *trt__AddMetadataConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap *soap,
                                                             struct _trt__AddAudioOutputConfiguration
                                                                 *trt__AddAudioOutputConfiguration,
                                                             struct _trt__AddAudioOutputConfigurationResponse
                                                                 *trt__AddAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap *soap,
                                                              struct _trt__AddAudioDecoderConfiguration
                                                                  *trt__AddAudioDecoderConfiguration,
                                                              struct _trt__AddAudioDecoderConfigurationResponse
                                                                  *trt__AddAudioDecoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap *soap,
                                                                 struct _trt__RemoveVideoEncoderConfiguration
                                                                     *trt__RemoveVideoEncoderConfiguration,
                                                                 struct _trt__RemoveVideoEncoderConfigurationResponse
                                                                     *trt__RemoveVideoEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap *soap,
                                                                struct _trt__RemoveVideoSourceConfiguration
                                                                    *trt__RemoveVideoSourceConfiguration,
                                                                struct _trt__RemoveVideoSourceConfigurationResponse
                                                                    *trt__RemoveVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap *soap,
                                                                 struct _trt__RemoveAudioEncoderConfiguration
                                                                     *trt__RemoveAudioEncoderConfiguration,
                                                                 struct _trt__RemoveAudioEncoderConfigurationResponse
                                                                     *trt__RemoveAudioEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap *soap,
                                                                struct _trt__RemoveAudioSourceConfiguration
                                                                    *trt__RemoveAudioSourceConfiguration,
                                                                struct _trt__RemoveAudioSourceConfigurationResponse
                                                                    *trt__RemoveAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap *soap,
                                                        struct _trt__RemovePTZConfiguration
                                                            *trt__RemovePTZConfiguration,
                                                        struct _trt__RemovePTZConfigurationResponse
                                                            *trt__RemovePTZConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap,
                                                                   struct _trt__RemoveVideoAnalyticsConfiguration
                                                                       *trt__RemoveVideoAnalyticsConfiguration,
                                                                   struct _trt__RemoveVideoAnalyticsConfigurationResponse
                                                                       *trt__RemoveVideoAnalyticsConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap *soap,
                                                             struct _trt__RemoveMetadataConfiguration
                                                                 *trt__RemoveMetadataConfiguration,
                                                             struct _trt__RemoveMetadataConfigurationResponse
                                                                 *trt__RemoveMetadataConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap *soap,
                                                                struct _trt__RemoveAudioOutputConfiguration
                                                                    *trt__RemoveAudioOutputConfiguration,
                                                                struct _trt__RemoveAudioOutputConfigurationResponse
                                                                    *trt__RemoveAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap *soap,
                                                                 struct _trt__RemoveAudioDecoderConfiguration
                                                                     *trt__RemoveAudioDecoderConfiguration,
                                                                 struct _trt__RemoveAudioDecoderConfigurationResponse
                                                                     *trt__RemoveAudioDecoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap,
                                               struct _trt__DeleteProfile
                                                   *trt__DeleteProfile,
                                               struct _trt__DeleteProfileResponse
                                                   *trt__DeleteProfileResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap *soap,
                                                              struct _trt__GetVideoSourceConfigurations
                                                                  *trt__GetVideoSourceConfigurations,
                                                              struct _trt__GetVideoSourceConfigurationsResponse
                                                                  *trt__GetVideoSourceConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = 1;
    struct tt__VideoSourceConfiguration *Configurations;
    ALLOC_STRUCT_NUM(Configurations, struct tt__VideoSourceConfiguration, 1);

    Configurations[0].UseCount = 1;
    Configurations[0].Name = soap_strdup(soap, gOnvifVideoSrcCfg.name);
    Configurations[0].token = soap_strdup(soap, gOnvifVideoSrcCfg.token);
    Configurations[0].SourceToken = soap_strdup(soap, gOnvifVideoSrcCfg.sourceToken);

    ALLOC_STRUCT(Configurations[0].Bounds, struct tt__IntRectangle);
    Configurations[0].Bounds->x = gOnvifVideoSrcCfg.bounds.x;
    Configurations[0].Bounds->y = gOnvifVideoSrcCfg.bounds.y;
    Configurations[0].Bounds->width = gOnvifVideoSrcCfg.bounds.width;
    Configurations[0].Bounds->height = gOnvifVideoSrcCfg.bounds.height;

    trt__GetVideoSourceConfigurationsResponse->Configurations = Configurations;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap *soap,
                                                               struct _trt__GetVideoEncoderConfigurations
                                                                   *trt__GetVideoEncoderConfigurations,
                                                               struct _trt__GetVideoEncoderConfigurationsResponse
                                                                   *trt__GetVideoEncoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    int size = 1;
    trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = size;
    struct tt__VideoEncoderConfiguration *Configurations = NULL;
    ALLOC_STRUCT_NUM(Configurations, struct tt__VideoEncoderConfiguration, size);

    //<VideoEncoderConfigurations>
    int i = 0;
    Configurations[i].Name = soap_strdup(soap, gOnvifVideoEncCfg.name);
    Configurations[i].token = soap_strdup(soap, gOnvifVideoEncCfg.token);
    Configurations[i].UseCount = gOnvifVideoEncCfg.useCount;
    Configurations[i].Quality = gOnvifVideoEncCfg.quality;
        // JPEG = 0 , MPEG4 = 1, H264 = 2; H265 = 3;
    Configurations[i].Encoding = (enum tt__VideoEncoding)gOnvifVideoEncCfg.encoding; 
    //<Configurations><Resolution>
    ALLOC_STRUCT(Configurations[i].Resolution, struct tt__VideoResolution);
    Configurations[i].Resolution->Width = gOnvifVideoEncCfg.resolution.Width;
    Configurations[i].Resolution->Height = gOnvifVideoEncCfg.resolution.Height;
    //<Configurations><RateControl>
    ALLOC_STRUCT(Configurations[i].RateControl, struct tt__VideoRateControl);
    Configurations[i].RateControl->FrameRateLimit = gOnvifVideoEncCfg.frameRateLimit;
    Configurations[i].RateControl->EncodingInterval = gOnvifVideoEncCfg.gop;
    Configurations[i].RateControl->BitrateLimit = gOnvifVideoEncCfg.bitrate;
    //<Configurations><H264>
    ALLOC_STRUCT(Configurations[i].H264, struct tt__H264Configuration);
    Configurations[i].H264->GovLength = gOnvifVideoEncCfg.h264.govLength;
    Configurations[i].H264->H264Profile = (enum tt__H264Profile)gOnvifVideoEncCfg.h264.profile;

    //<Configuration><Multicast>
    ALLOC_STRUCT(Configurations[i].Multicast, struct tt__MulticastConfiguration);
    ALLOC_STRUCT(Configurations[i].Multicast->Address, struct tt__IPAddress)
    Configurations[i].Multicast->Address->Type = tt__IPType__IPv4;
    Configurations[i].Multicast->Address->IPv4Address = soap_strdup(soap, "224.1.0.0");
    Configurations[i].Multicast->Port = 5000;
    Configurations[i].Multicast->TTL = 1500;
    Configurations[i].Multicast->AutoStart = xsd__boolean__false_; //xsd__boolean__true_; xsd__boolean__false_;
    //<Configuration><SessionTimeout>
    Configurations[i].SessionTimeout = ONVIF_SESSION_TIME_OUT;

    trt__GetVideoEncoderConfigurationsResponse->Configurations = Configurations;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap *soap,
                                                              struct _trt__GetAudioSourceConfigurations
                                                                  *trt__GetAudioSourceConfigurations,
                                                              struct _trt__GetAudioSourceConfigurationsResponse
                                                                  *trt__GetAudioSourceConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap *soap,
                                                               struct _trt__GetAudioEncoderConfigurations
                                                                   *trt__GetAudioEncoderConfigurations,
                                                               struct _trt__GetAudioEncoderConfigurationsResponse
                                                                   *trt__GetAudioEncoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap *soap,
                                                                 struct _trt__GetVideoAnalyticsConfigurations
                                                                     *trt__GetVideoAnalyticsConfigurations,
                                                                 struct _trt__GetVideoAnalyticsConfigurationsResponse
                                                                     *trt__GetVideoAnalyticsConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap *soap,
                                                           struct _trt__GetMetadataConfigurations
                                                               *trt__GetMetadataConfigurations,
                                                           struct _trt__GetMetadataConfigurationsResponse
                                                               *trt__GetMetadataConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap *soap,
                                                              struct _trt__GetAudioOutputConfigurations
                                                                  *trt__GetAudioOutputConfigurations,
                                                              struct _trt__GetAudioOutputConfigurationsResponse
                                                                  *trt__GetAudioOutputConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap *soap,
                                                               struct _trt__GetAudioDecoderConfigurations
                                                                   *trt__GetAudioDecoderConfigurations,
                                                               struct _trt__GetAudioDecoderConfigurationsResponse
                                                                   *trt__GetAudioDecoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap *soap,
                                                             struct _trt__GetVideoSourceConfiguration
                                                                 *trt__GetVideoSourceConfiguration,
                                                             struct _trt__GetVideoSourceConfigurationResponse
                                                                 *trt__GetVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    struct tt__VideoSourceConfiguration *Configuration = NULL;

    ALLOC_STRUCT(Configuration, struct tt__VideoSourceConfiguration);
    Configuration->Name = soap_strdup(soap, gOnvifVideoSrcCfg.name);
    Configuration->token = soap_strdup(soap, gOnvifVideoSrcCfg.token);
    Configuration->SourceToken = soap_strdup(soap, gOnvifVideoSrcCfg.sourceToken); 

    ALLOC_STRUCT(Configuration->Bounds, struct tt__IntRectangle);
    Configuration->Bounds->x = gOnvifVideoSrcCfg.bounds.x;
    Configuration->Bounds->y = gOnvifVideoSrcCfg.bounds.y;
    Configuration->Bounds->width = gOnvifVideoSrcCfg.bounds.width;
    Configuration->Bounds->height = gOnvifVideoSrcCfg.bounds.height;

    trt__GetVideoSourceConfigurationResponse->Configuration = Configuration;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap *soap,
                                                              struct _trt__GetVideoEncoderConfiguration
                                                                  *trt__GetVideoEncoderConfiguration,
                                                              struct _trt__GetVideoEncoderConfigurationResponse
                                                                  *trt__GetVideoEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    struct tt__VideoEncoderConfiguration *Configuration = NULL;
    ALLOC_STRUCT(Configuration, struct tt__VideoEncoderConfiguration);

    //<VideoEncoderConfiguration>
    Configuration->Name = soap_strdup(soap, gOnvifVideoEncCfg.name);
    Configuration->token = soap_strdup(soap, gOnvifVideoEncCfg.token);
    Configuration->UseCount = 1;
    Configuration->Quality = 1;
    // JPEG = 0 , MPEG = 1, H264 = 2;
    Configuration->Encoding = (enum tt__VideoEncoding)gOnvifVideoEncCfg.encoding; 
    //<Configuration><Resolution>
    ALLOC_STRUCT(Configuration->Resolution, struct tt__VideoResolution);
    Configuration->Resolution->Width = gOnvifVideoEncCfg.resolution.Width;
    Configuration->Resolution->Height = gOnvifVideoEncCfg.resolution.Height;
    //<Configuration><RateControl>
    ALLOC_STRUCT(Configuration->RateControl, struct tt__VideoRateControl);
    Configuration->RateControl->FrameRateLimit = gOnvifVideoEncCfg.frameRateLimit;
    Configuration->RateControl->EncodingInterval = gOnvifVideoEncCfg.gop;
    Configuration->RateControl->BitrateLimit = gOnvifVideoEncCfg.bitrate;
    //<Configuration><H264>
    ALLOC_STRUCT(Configuration->H264, struct tt__H264Configuration);
    Configuration->H264->GovLength = gOnvifVideoEncCfg.h264.govLength;
    Configuration->H264->H264Profile = (enum tt__H264Profile)gOnvifVideoEncCfg.h264.profile;

    //<Configuration><Multicast>
    ALLOC_STRUCT(Configuration->Multicast, struct tt__MulticastConfiguration);
    ALLOC_STRUCT(Configuration->Multicast->Address, struct tt__IPAddress);
    Configuration->Multicast->Address->Type = tt__IPType__IPv4;
    Configuration->Multicast->Address->IPv4Address = soap_strdup(soap, "224.1.0.0");
    Configuration->Multicast->Port = 5000;
    Configuration->Multicast->TTL = 1500;
    Configuration->Multicast->AutoStart = xsd__boolean__true_;
    //<Configuration><SessionTimeout>
    Configuration->SessionTimeout = ONVIF_SESSION_TIME_OUT;

    trt__GetVideoEncoderConfigurationResponse->Configuration = Configuration;
    LOG_FUNC_OUT();
    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap *soap,
                                                             struct _trt__GetAudioSourceConfiguration
                                                                 *trt__GetAudioSourceConfiguration,
                                                             struct _trt__GetAudioSourceConfigurationResponse
                                                                 *trt__GetAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap *soap,
                                                              struct _trt__GetAudioEncoderConfiguration
                                                                  *trt__GetAudioEncoderConfiguration,
                                                              struct _trt__GetAudioEncoderConfigurationResponse
                                                                  *trt__GetAudioEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap *soap,
                                                                struct _trt__GetVideoAnalyticsConfiguration
                                                                    *trt__GetVideoAnalyticsConfiguration,
                                                                struct _trt__GetVideoAnalyticsConfigurationResponse
                                                                    *trt__GetVideoAnalyticsConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap *soap,
                                                          struct _trt__GetMetadataConfiguration
                                                              *trt__GetMetadataConfiguration,
                                                          struct _trt__GetMetadataConfigurationResponse
                                                              *trt__GetMetadataConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap *soap,
                                                             struct _trt__GetAudioOutputConfiguration
                                                                 *trt__GetAudioOutputConfiguration,
                                                             struct _trt__GetAudioOutputConfigurationResponse
                                                                 *trt__GetAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap *soap,
                                                              struct _trt__GetAudioDecoderConfiguration
                                                                  *trt__GetAudioDecoderConfiguration,
                                                              struct _trt__GetAudioDecoderConfigurationResponse
                                                                  *trt__GetAudioDecoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap,
                                                                         struct _trt__GetCompatibleVideoEncoderConfigurations
                                                                             *trt__GetCompatibleVideoEncoderConfigurations,
                                                                         struct _trt__GetCompatibleVideoEncoderConfigurationsResponse
                                                                             *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap,
                                                                        struct _trt__GetCompatibleVideoSourceConfigurations
                                                                            *trt__GetCompatibleVideoSourceConfigurations,
                                                                        struct _trt__GetCompatibleVideoSourceConfigurationsResponse
                                                                            *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap,
                                                                         struct _trt__GetCompatibleAudioEncoderConfigurations
                                                                             *trt__GetCompatibleAudioEncoderConfigurations,
                                                                         struct _trt__GetCompatibleAudioEncoderConfigurationsResponse
                                                                             *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap,
                                                                        struct _trt__GetCompatibleAudioSourceConfigurations
                                                                            *trt__GetCompatibleAudioSourceConfigurations,
                                                                        struct _trt__GetCompatibleAudioSourceConfigurationsResponse
                                                                            *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap,
                                                                           struct _trt__GetCompatibleVideoAnalyticsConfigurations
                                                                               *trt__GetCompatibleVideoAnalyticsConfigurations,
                                                                           struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse
                                                                               *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap *soap,
                                                                     struct _trt__GetCompatibleMetadataConfigurations
                                                                         *trt__GetCompatibleMetadataConfigurations,
                                                                     struct _trt__GetCompatibleMetadataConfigurationsResponse
                                                                         *trt__GetCompatibleMetadataConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap,
                                                                        struct _trt__GetCompatibleAudioOutputConfigurations
                                                                            *trt__GetCompatibleAudioOutputConfigurations,
                                                                        struct _trt__GetCompatibleAudioOutputConfigurationsResponse
                                                                            *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap,
                                                                         struct _trt__GetCompatibleAudioDecoderConfigurations
                                                                             *trt__GetCompatibleAudioDecoderConfigurations,
                                                                         struct _trt__GetCompatibleAudioDecoderConfigurationsResponse
                                                                             *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap *soap,
                                                             struct _trt__SetVideoSourceConfiguration
                                                                 *trt__SetVideoSourceConfiguration,
                                                             struct _trt__SetVideoSourceConfigurationResponse
                                                                 *trt__SetVideoSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap *soap,
                                                              struct _trt__SetVideoEncoderConfiguration
                                                                  *trt__SetVideoEncoderConfiguration,
                                                              struct _trt__SetVideoEncoderConfigurationResponse
                                                                  *trt__SetVideoEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    // get config

    // set config

    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap *soap,
                                                             struct _trt__SetAudioSourceConfiguration
                                                                 *trt__SetAudioSourceConfiguration,
                                                             struct _trt__SetAudioSourceConfigurationResponse
                                                                 *trt__SetAudioSourceConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap *soap,
                                                              struct _trt__SetAudioEncoderConfiguration
                                                                  *trt__SetAudioEncoderConfiguration,
                                                              struct _trt__SetAudioEncoderConfigurationResponse
                                                                  *trt__SetAudioEncoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap *soap,
                                                                struct _trt__SetVideoAnalyticsConfiguration
                                                                    *trt__SetVideoAnalyticsConfiguration,
                                                                struct _trt__SetVideoAnalyticsConfigurationResponse
                                                                    *trt__SetVideoAnalyticsConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap *soap,
                                                          struct _trt__SetMetadataConfiguration
                                                              *trt__SetMetadataConfiguration,
                                                          struct _trt__SetMetadataConfigurationResponse
                                                              *trt__SetMetadataConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap *soap,
                                                             struct _trt__SetAudioOutputConfiguration
                                                                 *trt__SetAudioOutputConfiguration,
                                                             struct _trt__SetAudioOutputConfigurationResponse
                                                                 *trt__SetAudioOutputConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap *soap,
                                                              struct _trt__SetAudioDecoderConfiguration
                                                                  *trt__SetAudioDecoderConfiguration,
                                                              struct _trt__SetAudioDecoderConfigurationResponse
                                                                  *trt__SetAudioDecoderConfigurationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap,
                                                                    struct _trt__GetVideoSourceConfigurationOptions
                                                                        *trt__GetVideoSourceConfigurationOptions,
                                                                    struct _trt__GetVideoSourceConfigurationOptionsResponse
                                                                        *trt__GetVideoSourceConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap *soap,
                                                                     struct _trt__GetVideoEncoderConfigurationOptions
                                                                         *trt__GetVideoEncoderConfigurationOptions,
                                                                     struct _trt__GetVideoEncoderConfigurationOptionsResponse
                                                                         *trt__GetVideoEncoderConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap *soap,
                                                                    struct _trt__GetAudioSourceConfigurationOptions
                                                                        *trt__GetAudioSourceConfigurationOptions,
                                                                    struct _trt__GetAudioSourceConfigurationOptionsResponse
                                                                        *trt__GetAudioSourceConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap *soap,
                                                                     struct _trt__GetAudioEncoderConfigurationOptions
                                                                         *trt__GetAudioEncoderConfigurationOptions,
                                                                     struct _trt__GetAudioEncoderConfigurationOptionsResponse
                                                                         *trt__GetAudioEncoderConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap *soap,
                                                                 struct _trt__GetMetadataConfigurationOptions
                                                                     *trt__GetMetadataConfigurationOptions,
                                                                 struct _trt__GetMetadataConfigurationOptionsResponse
                                                                     *trt__GetMetadataConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap *soap,
                                                                    struct _trt__GetAudioOutputConfigurationOptions
                                                                        *trt__GetAudioOutputConfigurationOptions,
                                                                    struct _trt__GetAudioOutputConfigurationOptionsResponse
                                                                        *trt__GetAudioOutputConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap *soap,
                                                                     struct _trt__GetAudioDecoderConfigurationOptions
                                                                         *trt__GetAudioDecoderConfigurationOptions,
                                                                     struct _trt__GetAudioDecoderConfigurationOptionsResponse
                                                                         *trt__GetAudioDecoderConfigurationOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap,
                                                                            struct _trt__GetGuaranteedNumberOfVideoEncoderInstances
                                                                                *trt__GetGuaranteedNumberOfVideoEncoderInstances,
                                                                            struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse
                                                                                *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap *soap,
                                              struct _trt__GetStreamUri
                                                  *trt__GetStreamUri,
                                              struct _trt__GetStreamUriResponse
                                                  *trt__GetStreamUriResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    log_war("get stream uri token: %s", trt__GetStreamUri->ProfileToken);

    // get the stream setting
    log_com("stream type %d, protocol %d tunnel %s",
        trt__GetStreamUri->StreamSetup->Stream,
        trt__GetStreamUri->StreamSetup->Transport->Protocol,
        trt__GetStreamUri->StreamSetup->Transport->Tunnel);
    struct tt__MediaUri *MediaUri = NULL;
    ALLOC_STRUCT(MediaUri, struct tt__MediaUri);
    char tmp_ip[128] = {0};
    sprintf(tmp_ip, "rtsp://%s:8554/live0", gOnvifDevInfo.ip);
    MediaUri->Uri = soap_strdup(soap, tmp_ip);
    MediaUri->InvalidAfterConnect = xsd__boolean__true_;
    MediaUri->InvalidAfterReboot = xsd__boolean__true_;
    MediaUri->Timeout = 3;

    trt__GetStreamUriResponse->MediaUri = MediaUri;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap *soap,
                                                         struct _trt__StartMulticastStreaming
                                                             *trt__StartMulticastStreaming,
                                                         struct _trt__StartMulticastStreamingResponse
                                                             *trt__StartMulticastStreamingResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap *soap,
                                                        struct _trt__StopMulticastStreaming
                                                            *trt__StopMulticastStreaming,
                                                        struct _trt__StopMulticastStreamingResponse
                                                            *trt__StopMulticastStreamingResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap *soap,
                                                         struct _trt__SetSynchronizationPoint
                                                             *trt__SetSynchronizationPoint,
                                                         struct _trt__SetSynchronizationPointResponse
                                                             *trt__SetSynchronizationPointResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap,
                                                struct _trt__GetSnapshotUri
                                                    *trt__GetSnapshotUri,
                                                struct _trt__GetSnapshotUriResponse
                                                    *trt__GetSnapshotUriResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;

    log_war("token: %s", trt__GetSnapshotUri->ProfileToken);
#if 1
    char tmp_ip[DEVICE_IP_LEN] = {0};
    struct tt__MediaUri* MediaUri = NULL;
    ALLOC_STRUCT(MediaUri, struct tt__MediaUri);
    sprintf(tmp_ip, "http://%s:%d/snap0.jpeg", gOnvifDevInfo.ip, ONVIF_SNAP_PORT); //ONVIF_TCP_PORT
    MediaUri->Uri = soap_strdup(soap, tmp_ip);
    MediaUri->InvalidAfterConnect = xsd__boolean__false_;
    MediaUri->InvalidAfterReboot = xsd__boolean__false_;
    MediaUri->Timeout = ONVIF_SESSION_TIME_OUT;
    trt__GetSnapshotUriResponse->MediaUri = MediaUri;
    log_com("uri: %s", trt__GetSnapshotUriResponse->MediaUri->Uri);
#else
    trt__GetSnapshotUriResponse->MediaUri = (struct tt__MediaUri *)soap_malloc(soap, sizeof(struct tt__MediaUri));
    memset(trt__GetSnapshotUriResponse->MediaUri, 0, sizeof(struct tt__MediaUri));
 
    trt__GetSnapshotUriResponse->MediaUri->Uri = (char *)soap_malloc(soap, sizeof(char) * 100);
    memset(trt__GetSnapshotUriResponse->MediaUri->Uri, 0, sizeof(char) * 100);
    sprintf(trt__GetSnapshotUriResponse->MediaUri->Uri, "http://%s:%d/snap0.jpeg", "192.168.1.101", 80);
    trt__GetSnapshotUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__false_;
    trt__GetSnapshotUriResponse->MediaUri->InvalidAfterReboot = xsd__boolean__false_;
    trt__GetSnapshotUriResponse->MediaUri->Timeout = ONVIF_SESSION_TIME_OUT;
#endif
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceModes(struct soap *soap,
                                                     struct _trt__GetVideoSourceModes
                                                         *trt__GetVideoSourceModes,
                                                     struct _trt__GetVideoSourceModesResponse
                                                         *trt__GetVideoSourceModesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceMode(struct soap *soap,
                                                    struct _trt__SetVideoSourceMode
                                                        *trt__SetVideoSourceMode,
                                                    struct _trt__SetVideoSourceModeResponse
                                                        *trt__SetVideoSourceModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs(struct soap *soap,
                                         struct _trt__GetOSDs
                                             *trt__GetOSDs,
                                         struct _trt__GetOSDsResponse
                                             *trt__GetOSDsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD(struct soap *soap,
                                        struct _trt__GetOSD
                                            *trt__GetOSD,
                                        struct _trt__GetOSDResponse
                                            *trt__GetOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions(struct soap *soap,
                                               struct _trt__GetOSDOptions
                                                   *trt__GetOSDOptions,
                                               struct _trt__GetOSDOptionsResponse
                                                   *trt__GetOSDOptionsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD(struct soap *soap,
                                        struct _trt__SetOSD
                                            *trt__SetOSD,
                                        struct _trt__SetOSDResponse
                                            *trt__SetOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD(struct soap *soap,
                                           struct _trt__CreateOSD
                                               *trt__CreateOSD,
                                           struct _trt__CreateOSDResponse
                                               *trt__CreateOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD(struct soap *soap,
                                           struct _trt__DeleteOSD
                                               *trt__DeleteOSD,
                                           struct _trt__DeleteOSDResponse
                                               *trt__DeleteOSDResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetServiceCapabilities(struct soap *soap,
                                                        struct _trv__GetServiceCapabilities
                                                            *trv__GetServiceCapabilities,
                                                        struct _trv__GetServiceCapabilitiesResponse
                                                            *trv__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetReceivers(struct soap *soap,
                                              struct _trv__GetReceivers *trv__GetReceivers,
                                              struct _trv__GetReceiversResponse *trv__GetReceiversResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetReceiver(struct soap *soap,
                                             struct _trv__GetReceiver *trv__GetReceiver,
                                             struct _trv__GetReceiverResponse *trv__GetReceiverResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__CreateReceiver(struct soap *soap,
                                                struct _trv__CreateReceiver *trv__CreateReceiver,
                                                struct _trv__CreateReceiverResponse *trv__CreateReceiverResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__DeleteReceiver(struct soap *soap,
                                                struct _trv__DeleteReceiver *trv__DeleteReceiver,
                                                struct _trv__DeleteReceiverResponse *trv__DeleteReceiverResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__ConfigureReceiver(struct soap *soap,
                                                   struct _trv__ConfigureReceiver *trv__ConfigureReceiver,
                                                   struct _trv__ConfigureReceiverResponse *trv__ConfigureReceiverResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__SetReceiverMode(struct soap *soap,
                                                 struct _trv__SetReceiverMode *trv__SetReceiverMode,
                                                 struct _trv__SetReceiverModeResponse *trv__SetReceiverModeResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetReceiverState(struct soap *soap,
                                                  struct _trv__GetReceiverState *trv__GetReceiverState,
                                                  struct _trv__GetReceiverStateResponse *trv__GetReceiverStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetServiceCapabilities(struct soap *soap,
                                                        struct _tse__GetServiceCapabilities
                                                            *tse__GetServiceCapabilities,
                                                        struct _tse__GetServiceCapabilitiesResponse
                                                            *tse__GetServiceCapabilitiesResponse)
{
    LOG_FUNC_IN();
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSummary(struct soap *soap,
                                                     struct _tse__GetRecordingSummary
                                                         *tse__GetRecordingSummary,
                                                     struct _tse__GetRecordingSummaryResponse
                                                         *tse__GetRecordingSummaryResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingInformation(struct soap *soap,
                                                         struct _tse__GetRecordingInformation
                                                             *tse__GetRecordingInformation,
                                                         struct _tse__GetRecordingInformationResponse
                                                             *tse__GetRecordingInformationResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMediaAttributes(struct soap *soap,
                                                    struct _tse__GetMediaAttributes
                                                        *tse__GetMediaAttributes,
                                                    struct _tse__GetMediaAttributesResponse
                                                        *tse__GetMediaAttributesResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindRecordings(struct soap *soap,
                                                struct _tse__FindRecordings
                                                    *tse__FindRecordings,
                                                struct _tse__FindRecordingsResponse
                                                    *tse__FindRecordingsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSearchResults(struct soap *soap,
                                                           struct _tse__GetRecordingSearchResults
                                                               *tse__GetRecordingSearchResults,
                                                           struct _tse__GetRecordingSearchResultsResponse
                                                               *tse__GetRecordingSearchResultsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindEvents(struct soap *soap,
                                            struct _tse__FindEvents
                                                *tse__FindEvents,
                                            struct _tse__FindEventsResponse
                                                *tse__FindEventsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetEventSearchResults(struct soap *soap,
                                                       struct _tse__GetEventSearchResults
                                                           *tse__GetEventSearchResults,
                                                       struct _tse__GetEventSearchResultsResponse
                                                           *tse__GetEventSearchResultsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindPTZPosition(struct soap *soap,
                                                 struct _tse__FindPTZPosition
                                                     *tse__FindPTZPosition,
                                                 struct _tse__FindPTZPositionResponse
                                                     *tse__FindPTZPositionResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetPTZPositionSearchResults(struct soap *soap,
                                                             struct _tse__GetPTZPositionSearchResults
                                                                 *tse__GetPTZPositionSearchResults,
                                                             struct _tse__GetPTZPositionSearchResultsResponse
                                                                 *tse__GetPTZPositionSearchResultsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetSearchState(struct soap *soap,
                                                struct _tse__GetSearchState
                                                    *tse__GetSearchState,
                                                struct _tse__GetSearchStateResponse
                                                    *tse__GetSearchStateResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__EndSearch(struct soap *soap,
                                           struct _tse__EndSearch
                                               *tse__EndSearch,
                                           struct _tse__EndSearchResponse
                                               *tse__EndSearchResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindMetadata(struct soap *soap,
                                              struct _tse__FindMetadata
                                                  *tse__FindMetadata,
                                              struct _tse__FindMetadataResponse
                                                  *tse__FindMetadataResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMetadataSearchResults(struct soap *soap,
                                                          struct _tse__GetMetadataSearchResults
                                                              *tse__GetMetadataSearchResults,
                                                          struct _tse__GetMetadataSearchResultsResponse
                                                              *tse__GetMetadataSearchResultsResponse)
{
    LOG_FUNC_IN();
    CHECK_USERNAME_TOKEN;
    LOG_FUNC_OUT();
    return 0;
}
