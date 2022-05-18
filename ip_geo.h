#ifndef IPIFY_ORG_CA_ROOT_CERTIFICATE
#define IPIFY_ORG_CA_ROOT_CERTIFICATE

/**
 * This certificate is downloaded from Chrome.
 *
 * 1. Go to web site ipify.org.
 * 2. Right click on the symbol for web site information and choose
 *    information about web site certificate.
 * 3. Open tab security chain and select certificate next to the end of chain.
 *    This will be the certificate of a Certificate Authority that has issued
 *    the certificate for ipify.org, in this case Sectigo RSA Domain Validation
 *    Secure Server CA.
 * 4. Copy the certificate (X.509) to file in PEM (Base-64-encodet ASCII)
 *    format.
 * 5. Open file in GitBash and issue commands
 *    :%norm I"        to add " at start of each line, then
 *    :%norm A\n"      to add \n" at end of each line, then
 *    :wq              to save file and quit.
 * 6. Copy content to some source file
 *    const char root_cert[] = <the copied pem cert>;
 */

// Certificate of issuer Sectigo RSA Domain Validation Secure Server CA
const char ip_geo_cert[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDvjCCAqagAwIBAgIJQgAAB5dihR8xMA0GCSqGSIb3DQEBCwUAMFQxGTAXBgNV\n"
"BAoMEEFPIEthc3BlcnNreSBMYWIxNzA1BgNVBAMMLkthc3BlcnNreSBBbnRpLVZp\n"
"cnVzIFBlcnNvbmFsIFJvb3QgQ2VydGlmaWNhdGUwHhcNMjExMTE3MTYzMDQxWhcN\n"
"MjIxMTE2MTYzMDQxWjB1MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5p\n"
"YTEWMBQGA1UEBxMNU2FuIEZyYW5jaXNjbzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwg\n"
"SW5jLjEeMBwGA1UEAxMVc25pLmNsb3VkZmxhcmVzc2wuY29tMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAs6WXTJeqym3GIB3QAryGCchIboASKY8wg0Tq\n"
"y/gtgK+jRHQnY0B0CEAjHuWijNsKRimXoXCu//EdAITSNHoNyGWMX1vme/4W2h3/\n"
"It298ZCr4MOxCeeILPj/cCAD10doekoMJx2gLuGZzMHP6I081+vq1JFLfbE36lp8\n"
"B9kBzj9uVVvxzqDzcBA/kyO3Br10X4GCG3YDdssqzUei6VY9PBu8MTEhM1ZI9XMk\n"
"VB82YpbQ/TSZwya94P/bmqPHR9OoSPgGFevi8+n/M5iSQkq8N6ap2QjrJ/lyQWUI\n"
"vpdEGs6K14HYkd2vwl+nbqctJgYXguV5vvaw3HIwiZ7aEKAwbQIDAQABo3IwcDAT\n"
"BgNVHSUEDDAKBggrBgEFBQcDATALBgNVHQ8EBAMCBaAwTAYDVR0RBEUwQ4cErEMR\n"
"R4IQaXBnZW9sb2NhdGlvbi5pb4IVc25pLmNsb3VkZmxhcmVzc2wuY29tghIqLmlw\n"
"Z2VvbG9jYXRpb24uaW8wDQYJKoZIhvcNAQELBQADggEBAIbOptctC/XZx2S8Ikzq\n"
"t9QMx86wpJv2W8YBWlet5rrdzT/SwU/+ADWqIT40B5R8xPbMbr38o1Rpwoq1KlLr\n"
"wOGE4NGsp5DchuL77EICTCVI4qsaRhYU+9ZgTQEKIWgXQqrRJRnwdCJAox6+cu7F\n"
"fvyQBZ94UdUj5FhumkqDI82ZMGLS/h4dQmi3w8dSwntW556ZdKyz+TLm45VPdbQf\n"
"xhWieZGKi5pihfxx4jmvN2nI42lU4qimRTDxGFeGFM4HA1LmeTQl0I83uNEWoPDp\n"
"VWdRmRNjPZGlZBiMcFt0NzX/ieXOHQbByDN7fCgqiyrxhb7ukQEOYgXYj2IYPaAP\n"
"y58=\n"
"-----END CERTIFICATE-----\n";

#endif // IPIFY_ORG_CA_ROOT_CERTIFICATE