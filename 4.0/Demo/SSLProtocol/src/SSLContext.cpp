#include "stdafx.h"
#include "SSLContext.h"


SSLContext::SSLContext(BufferPool& pool)
	:ProtocolContext(pool)
{
	
}

bool SSLContext::initializeSSLSession(SSL_CTX *ctx, bool isServer, Buffer& outgoingBytes)
{
	ssl = SSL_new(ctx);
	bioIn = BIO_new(BIO_s_mem());
	bioOut = BIO_new(BIO_s_mem());

	SSL_set_bio(ssl, bioIn, bioOut);

	if (isServer)
	{
		SSL_set_accept_state(ssl);
		return true;
	}
	else
	{
		SSL_set_connect_state(ssl);
		ssl->handshake_func(ssl);

		return getPendingWrite(outgoingBytes);
	}
}


SSLContext::~SSLContext(void)
{
}

bool SSLContext::getPendingWrite( Buffer& outputBuffer )
{
	int nPending = BIO_ctrl_pending(bioOut);
	if (nPending > 0)
	{
		int nSize = BIO_read(bioOut, outputBuffer.getBuffer(), outputBuffer.getCapacity());
		outputBuffer.growSize(nSize);

		return true;
	}

	return false;
}

bool SSLContext::readData( Buffer& incomingBytes )
{
	int ret = BIO_write(bioIn, incomingBytes.getBuffer(), incomingBytes.getDataSize());
	return ret == incomingBytes.getDataSize();
}

DecodeResult::Type SSLContext::tryDecode( Buffer& outputBuffer )
{
	int totalContentDecrypted = 0;
	while (true)
	{
		if (outputBuffer.isFull())
		{
			break;
		}

		int ret = SSL_read(ssl, outputBuffer.getPosition(), outputBuffer.getRemainingSize());
		
		if (ret == 0)
			return DecodeResult::Close; //SSL close notify frame
		else if(ret > 0)
		{
			totalContentDecrypted += ret;
			outputBuffer.growSize(ret);
		}
		else
		{
			int error = SSL_get_error(ssl, ret);
			if (error == SSL_ERROR_WANT_WRITE)
				return DecodeResult::Failure; //TODO. Can this happen ?
			else if(error == SSL_ERROR_WANT_READ)
			{
				break;
			}
			else
			{
				cout << "SSL_read failure: " << outputBuffer.getRemainingSize() << endl;
				return DecodeResult::Failure;
			}
		}
	}

	if (totalContentDecrypted > 0)
	{
		checkForHandshakeCompletion();
		return DecodeResult::Content;
	}

	if (getPendingWrite(outputBuffer))
	{
		checkForHandshakeCompletion();
		return DecodeResult::ProtocolBytes;
	}

	return DecodeResult::WantMoreData; //
}

EncodeResult::Type SSLContext::encodeContent( Buffer& inputBuffer, Buffer& outputBuffer )
{
	unsigned int remainingBufferSize = outputBuffer.getRemainingSize();
	unsigned int nMaxExpectedEncryptedSize = inputBuffer.getDataSize() * 2;

	if (remainingBufferSize < nMaxExpectedEncryptedSize)
	{
		return EncodeResult::InsufficientBufferSpace;
	}

	int nWritten = SSL_write(ssl, inputBuffer.getBuffer(), inputBuffer.getDataSize());

	if (nWritten != inputBuffer.getDataSize())
	{
		cout << "SSL_write returned: " << nWritten << endl;
		if (nWritten <= 0)
		{
			int error = SSL_get_error(ssl, nWritten);
			cout << "SSL_write failed. error:= " << error << endl;
			//Note that error = SSL_ERROR_WANT_READ should be considered as non fatal error.
			//But due to design of ProtocolFramework, this should not occur. If it occurs then there is something wrong.
		}
		return EncodeResult::FatalFailure;
	}

	//Now read all encrypted data:
	int ret = BIO_ctrl_pending(bioOut);
	if(ret <= 0)
		return EncodeResult::FatalFailure; //How come there is encryption result ?!

	double dFactor = (double) ret / (double) nWritten;
	cout << "Factor = " << dFactor << endl;
	
	if (ret > remainingBufferSize)
	{
		cout << "Encryption size is more than expected" << endl;
		return EncodeResult::FatalFailure; //Normally the condition set in beginning of the method will prevent this.
	}

	int nEncrypted = BIO_read(bioOut, outputBuffer.getPosition(), remainingBufferSize);

	if (nEncrypted != ret)
	{
		cout << "BIO_read returned different result than BIO_ctrl_pending" << endl; //This is here to detect sync issues. Todo. remove.
		return EncodeResult::FatalFailure;
	}

	outputBuffer.growSize(nEncrypted);

	return EncodeResult::Success;
}

void SSLContext::checkForHandshakeCompletion()
{
	if (!isInitialized())
	{
		if (SSL_in_init(ssl) == 0)
			setInitialized();
	}
}

void SSLContext::recycle()
{
	SSL_free(ssl); // This will automatically free bioIn and bioOut
}


