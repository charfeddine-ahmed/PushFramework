#include "Symbols.h"

typedef struct ssl_ctx_st SSL_CTX; //Forward declaration.

class SSLPROTOCOL_API SSLProtocol : public Protocol
{
public:
	SSLProtocol(void);
	~SSLProtocol();

	bool initializeAsServer(const char* serverCertificatePubKey, const char* serverCertificatePrivKey, const char* serverCertificatePrivPassword = NULL);
	bool initializeAsClient(const char* caCertificatePubKey);

protected:
	virtual ProtocolContext* createNewProtocolContext();
	virtual void startSession(ProtocolContext& context, Buffer& outgoingBytes);
	virtual bool readData(ProtocolContext& context, Buffer& incomingBytes);
	virtual DecodeResult::Type tryDecode(ProtocolContext& context, Buffer& outputBuffer);
	virtual EncodeResult::Type encodeContent(ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer);
	virtual unsigned int getRequiredOutputSize(unsigned int maxInputSize);
private:
	bool isServer;
	bool isInitialized;
	SSL_CTX *ctx;

private:
	static void initializeLocks();
	bool initializeCommon();
};


