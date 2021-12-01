#pragma once
class SSLContext : public ProtocolContext
{
public:
	SSLContext(BufferPool& pool);
	~SSLContext(void);

	bool initializeSSLSession(SSL_CTX *ctx, bool isServer, Buffer& outgoingBytes);

	bool readData(Buffer& incomingBytes);
	DecodeResult::Type tryDecode(Buffer& outputBuffer);
	EncodeResult::Type encodeContent(Buffer& inputBuffer, Buffer& outputBuffer);
private:
	SSL* ssl;
	BIO* bioIn;
	BIO* bioOut;

	void checkForHandshakeCompletion();
	bool getPendingWrite(Buffer& outputBuffer);

protected:
	virtual void recycle();
};

