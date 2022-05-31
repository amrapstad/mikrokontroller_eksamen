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
const char ipify_org_ca_root_certificate[] =
    "-----BEGIN CERTIFICATE-----\n"
    "a9097ef26d36404f886703871ba008bd\n"
    "-----END CERTIFICATE-----\n";

#endif // IPIFY_ORG_CA_ROOT_CERTIFICATE
